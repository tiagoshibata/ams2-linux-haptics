#define _GNU_SOURCE
#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#include "ams2_shmem.h"

// Size of shared mem region created by AMS2
const off_t REGION_SIZE = 0x6000;
const int PLOT_WIDTH = 80;
const int PLOT_HISTORY = 100;

static const char *CSI = "\033[";
static const char *RED = "\033[31m";
static const char *GREEN = "\033[32m";
static const char *BLUE = "\033[34m";
static const char *RESET = "\033[0m";

static void clear_screen(void) { printf("%s2J%sH", CSI, CSI); }

static void hide_cursor(void) { printf("%s?25l", CSI); }

static void show_cursor(void) { printf("%s?25h", CSI); }

// Scan /proc/.../maps, and find virtual address in AMS2 space that hold telemetry shared mem
off_t get_telemetry_sharedmem_address(pid_t pid) {
  char maps_path[256];
  snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

  FILE *fp = fopen(maps_path, "r");
  if (!fp) {
    perror("Failed to open maps");
    exit(1);
  }

  char line[1024];
  while (fgets(line, sizeof(line), fp)) {
    off_t start, end, offset;
    char perms[5];

    if (sscanf(line, "%lx-%lx %4s %lx", &start, &end, perms, &offset) != 4) {
      continue;
    }

    if (perms[0] != 'r' || perms[1] != 'w' || perms[2] != '-' || perms[3] != 's' || offset ||
        (end - start) != REGION_SIZE || !strstr(line, "tmpmap-")) {
      continue;
    }

    printf("Found AMS2 telemetry shared mem at %lx\n", start);
    fclose(fp);
    return start;
  }

  fprintf(stderr, "Can't find AMS2 telemetry shared mem. Make sure it's enabled in AMS2 settings\n");
  fclose(fp);
  exit(1);
}

// Read memory from another process
void read_proc_memory(pid_t pid, off_t remote_addr, void *local_buf, size_t size) {
  struct iovec local_iov = {
      .iov_base = local_buf,
      .iov_len = size,
  };

  struct iovec remote_iov = {
      .iov_base = (void *)remote_addr,
      .iov_len = size,
  };

  ssize_t nread = process_vm_readv(pid, &local_iov, 1, &remote_iov, 1, 0);
  if (nread < 0) {
    perror("process_vm_readv");
    exit(1);
  }
}

static void plot_line(const char *label, const char *color, float value, const char *plot_char) {
  int bar_width = (int)(value * (PLOT_WIDTH - strlen(label) - 10));
  bar_width = bar_width < 0 ? 0 : bar_width;
  printf("%s%s:%s ", color, label, RESET);
  for (int i = 0; i < bar_width; i++) {
    printf("%s%s%s", color, plot_char, RESET);
  }
  printf(" %.2f\n", value);
}

static void plot_overlay(float *history, int len, const char *color) {
  for (int row = 10; row >= 0; row--) {
    printf("%s│%s", color, RESET);
    for (int i = 0; i < len; i++) {
      int plot_row = (int)(history[i] * 10.0f);
      if (plot_row == row) {
        printf("%s●%s", color, RESET);
      } else if (plot_row > row && plot_row < 10) {
        printf("%s│%s", color, RESET);
      } else {
        printf(" ");
      }
    }
    printf("%s│%s %d%%\n", color, RESET, row * 10);
  }
}

static void plot_separator(const char *color) {
  printf("%s├──%s", color, RESET);
  for (int i = 0; i < PLOT_WIDTH - 6; i++) {
    printf("%s─%s", color, RESET);
  }
  printf("%s──┤%s\n", color, RESET);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
    return 1;
  }

  pid_t pid = (pid_t)atoi(argv[1]);
  off_t region_start = get_telemetry_sharedmem_address(pid);

  float throttle_hist[PLOT_HISTORY] = {};
  float brake_hist[PLOT_HISTORY] = {};
  float steer_hist[PLOT_HISTORY] = {};
  int hist_idx = 0;
  int hist_count = 0;

  hide_cursor();
  atexit(show_cursor);

  struct timespec ts = {
      .tv_sec = 0,
      .tv_nsec = 20 * 1000 * 1000, // 20ms = 50Hz
  };

  while (1) {
    ams2_shmem data;
    read_proc_memory(pid, region_start, &data, sizeof(data));

    throttle_hist[hist_idx] = data.mUnfilteredThrottle;
    brake_hist[hist_idx] = data.mUnfilteredBrake;
    steer_hist[hist_idx] = (data.mUnfilteredSteering + 1.0f) / 2.0f; // remap -1..1 to 0..1
    hist_idx = (hist_idx + 1) % PLOT_HISTORY;
    if (hist_count < PLOT_HISTORY) {
      hist_count++;
    }

    clear_screen();
    printf("AMS2 Telemetry (50Hz)\n");
    printf("═══════════════════════════════════════════════════════════════════════════════════════════\n\n");

    plot_line("Throttle", GREEN, data.mUnfilteredThrottle, "█");
    plot_line("Brake", RED, data.mUnfilteredBrake, "█");
    plot_line("Steering", BLUE, data.mUnfilteredSteering, "█");

    printf("\n%sSteering:%s\n", BLUE, RESET);
    plot_separator(BLUE);
    plot_overlay(steer_hist, hist_count, BLUE);
    plot_separator(BLUE);

    printf("\n%sThrottle:%s\n", GREEN, RESET);
    plot_separator(GREEN);
    plot_overlay(throttle_hist, hist_count, GREEN);
    plot_separator(GREEN);

    printf("\n%sBrake:%s\n", RED, RESET);
    plot_separator(RED);
    plot_overlay(brake_hist, hist_count, RED);
    plot_separator(RED);

    fflush(stdout);
    nanosleep(&ts, NULL);
  }

  return 0;
}
