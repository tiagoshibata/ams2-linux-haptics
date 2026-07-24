#include <getopt.h>
#include <inttypes.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <carquet/carquet.h>
#include <cstring>
#include <vector>

#include "ams2_telemetry_reflect.h"

static constexpr size_t BATCH_SIZE = 1024 * 1024;

// --- Column set (filtered columns + buffer layout) ---

struct column_set {
  std::vector<const column_def *> cols;
  std::vector<size_t> buf_offsets;
  size_t total_buf_size = 0;

  void compute_layout() {
    buf_offsets.clear();
    size_t off = 0;
    for (const auto *c : cols) {
      buf_offsets.push_back(off);
      off += BATCH_SIZE * c->elem_size;
    }
    total_buf_size = off;
  }
};

// --- Row buffer ---

struct row_buffer {
  std::vector<std::byte> data;
  size_t count = 0;
};

static void buffer_add_row(row_buffer &buf, const column_set &cs, const ams2_telemetry &tele) {
  size_t row = buf.count++;
  for (size_t i = 0; i < cs.cols.size(); i++) {
    size_t dst_off = cs.buf_offsets[i] + row * cs.cols[i]->elem_size;
    const void *src = reinterpret_cast<const uint8_t *>(&tele) + cs.cols[i]->tele_offset;
    std::memcpy(buf.data.data() + dst_off, src, cs.cols[i]->elem_size);
  }
}

static void buffer_flush(row_buffer &buf, carquet_writer_t *writer, const column_set &cs) {
  if (buf.count == 0) {
    return;
  }

  for (size_t i = 0; i < cs.cols.size(); i++) {
    const void *data = buf.data.data() + cs.buf_offsets[i];
    carquet_status_t st = carquet_writer_write_batch(writer, (int32_t)i, data, (int32_t)buf.count, NULL, NULL);
    if (st != CARQUET_OK) {
      fprintf(stderr, "Failed to write column '%s': %s\n", cs.cols[i]->name, carquet_status_string(st));
    }
  }
  buf.count = 0;
}

// --- Signal handling ---

static volatile sig_atomic_t signal_received = 0;

static void handle_signal(int) { signal_received = 1; }

// --- CLI ---

static void usage(const char *prog) {
  fprintf(stderr, "Usage: %s [OPTIONS]\n", prog);
  fprintf(stderr, "  -o, --output PATH     Output file (default: telemetry.parquet)\n");
  fprintf(stderr, "  -c, --columns LIST    Comma-separated column names to include\n");
  fprintf(stderr, "  -l, --list            Print available columns and exit\n");
  fprintf(stderr, "  -h, --help            Show this help\n");
}

static void list_columns() {
  for (size_t i = 0; i < all_columns.count; i++) {
    fprintf(stdout, "%s\n", all_columns.cols[i].name);
  }
}

static column_set parse_columns(const char *filter) {
  column_set cs;
  std::string input(filter);
  size_t pos = 0;

  while (pos <= input.size()) {
    size_t next = input.find(',', pos);
    std::string name = input.substr(pos, next - pos);

    if (!name.empty()) {
      bool found = false;
      for (size_t i = 0; i < all_columns.count; i++) {
        if (name == all_columns.cols[i].name) {
          cs.cols.push_back(&all_columns.cols[i]);
          found = true;
          break;
        }
      }
      if (!found) {
        fprintf(stderr, "Unknown column: %s\n", name.c_str());
        fprintf(stderr, "Available columns:\n");
        list_columns();
        cs.cols.clear();
        return cs;
      }
    }

    if (next == std::string::npos) {
      break;
    }
    pos = next + 1;
  }

  cs.compute_layout();
  return cs;
}

// --- Schema ---

static carquet_schema_t *create_schema(const column_set &cs, carquet_error_t *err) {
  carquet_schema_t *schema = carquet_schema_create(err);
  if (!schema) {
    return NULL;
  }

  for (size_t i = 0; i < cs.cols.size(); i++) {
    carquet_status_t st =
        carquet_schema_add_column(schema, cs.cols[i]->name, cs.cols[i]->type, NULL, CARQUET_REPETITION_REQUIRED, 0, 0);
    if (st != CARQUET_OK) {
      carquet_schema_free(schema);
      carquet_error_set(err, st, __FILE__, __LINE__, __func__, "failed to add column '%s'", cs.cols[i]->name);
      return NULL;
    }
  }
  return schema;
}

// --- Main ---

int main(int argc, char *argv[]) {
  const char *output_path = "telemetry.parquet";
  const char *columns_filter = nullptr;
  bool do_list = false;

  static struct option long_options[] = {{"output", required_argument, nullptr, 'o'},
                                         {"columns", required_argument, nullptr, 'c'},
                                         {"list", no_argument, nullptr, 'l'},
                                         {"help", no_argument, nullptr, 'h'},
                                         {nullptr, 0, nullptr, 0}};

  int opt;
  while ((opt = getopt_long(argc, argv, "o:c:lh", long_options, nullptr)) != -1) {
    switch (opt) {
    case 'o':
      output_path = optarg;
      break;
    case 'c':
      columns_filter = optarg;
      break;
    case 'l':
      do_list = true;
      break;
    case 'h':
      usage(argv[0]);
      return 0;
    default:
      usage(argv[0]);
      return 1;
    }
  }

  if (do_list) {
    list_columns();
    return 0;
  }

  // Build column set
  column_set cs;
  if (columns_filter) {
    cs = parse_columns(columns_filter);
    if (cs.cols.empty()) {
      return 1;
    }
  } else {
    for (size_t i = 0; i < all_columns.count; i++) {
      cs.cols.push_back(&all_columns.cols[i]);
    }
    cs.compute_layout();
  }

  fprintf(stderr, "Logging %zu columns\n", cs.cols.size());

  // Set up signal handlers
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  // Wait for AMS2
  fprintf(stderr, "Waiting for AMS2...\n");
  int pid = wait_for_ams2_pid();
  const void *remote_addr = wait_for_ams2_telemetry_address(pid);
  fprintf(stderr, "Connected. Logging telemetry to %s\n", output_path);

  // Create parquet writer
  carquet_error_t err = CARQUET_ERROR_INIT;
  carquet_schema_t *schema = create_schema(cs, &err);
  if (!schema) {
    fprintf(stderr, "Failed to create schema: %s\n", carquet_error_message(&err));
    return 1;
  }

  carquet_writer_options_t opts;
  carquet_writer_options_init(&opts);
  opts.compression = CARQUET_COMPRESSION_ZSTD;
  opts.write_statistics = true;

  carquet_writer_t *writer = carquet_writer_create(output_path, schema, &opts, &err);
  if (!writer) {
    fprintf(stderr, "Failed to create writer: %s\n", carquet_error_message(&err));
    carquet_schema_free(schema);
    return 1;
  }

  if (carquet_writer_add_metadata(writer, "app", "ams2_parquet_logger_cpp") != CARQUET_OK) {
    fprintf(stderr, "Warning: failed to add file metadata\n");
  }

  // Allocate buffer
  row_buffer buf;
  buf.data.resize(cs.total_buf_size);

  struct timespec short_sleep = {.tv_sec = 0, .tv_nsec = 1 * 1000 * 1000};
  struct timespec long_sleep = {.tv_sec = 0, .tv_nsec = 18 * 1000 * 1000};

  unsigned seq_num = 0;
  int64_t total_rows = 0;

  while (!signal_received) {
    ams2_telemetry tele;
    while (!signal_received && !read_ams2_telemetry(pid, &tele, remote_addr)) {
      nanosleep(&short_sleep, NULL);
    }
    if (signal_received) {
      break;
    }

    if (seq_num && tele.sequenceNumber != seq_num + 2) {
      fprintf(stderr, "Skipped updates: %u -> %u\n", seq_num, tele.sequenceNumber);
    }
    seq_num = tele.sequenceNumber;

    buffer_add_row(buf, cs, tele);
    total_rows++;

    if (buf.count >= BATCH_SIZE) {
      buffer_flush(buf, writer, cs);
      fprintf(stderr, "Flushed row group (%" PRId64 " total rows)\n", total_rows);
    }

    nanosleep(&long_sleep, NULL);
  }

  buffer_flush(buf, writer, cs);

  carquet_status_t st = carquet_writer_close(writer);
  if (st != CARQUET_OK) {
    fprintf(stderr, "Failed to close writer: %s\n", carquet_status_string(st));
  }

  carquet_schema_free(schema);
  fprintf(stderr, "Wrote %" PRId64 " rows to %s\n", total_rows, output_path);
  return 0;
}
