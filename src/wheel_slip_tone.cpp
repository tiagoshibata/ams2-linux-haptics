#include <cmath>
#include <getopt.h>
#include <stdint.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pipewire/pipewire.h>
#include <spa/param/audio/raw-utils.h>
#include <spa/param/audio/raw.h>
#include <spa/param/param.h>
#include <thread>

#include "ams2_telemetry.h"

#define SAMPLE_RATE 48000u
#define CHANNELS 2u
#define DEFAULT_FREQUENCY 40
#define VOLUME_LOW 0.4f
#define VOLUME_HIGH 1.f
#define TWO_PI (float)(2 * M_PI)

#define SLIP_LOW 0.07f
#define SLIP_HIGH 0.15f

#define TELEMETRY_POLL_MS 20

constexpr float TYRE_DIAMETER = 0.584f;

static struct {
  const char *sink_name = nullptr;
  int frequency = DEFAULT_FREQUENCY;
} cli_args;

static std::atomic<float> g_max_slip{0.0f};
static std::atomic<bool> g_running{true};
static struct pw_main_loop *main_loop = nullptr;
static struct pw_stream *stream = nullptr;

static float compute_slip(float tyre_rps, float x_vel) {
  if (std::abs(x_vel) < 0.01f) {
    return 0.0f;
  }
  float tyre_surface_speed = TYRE_DIAMETER / 2.0f * tyre_rps;
  return std::abs((tyre_surface_speed - x_vel) / x_vel);
}

static void on_process([[maybe_unused]] void *userdata) {
  struct pw_buffer *b = pw_stream_dequeue_buffer(stream);
  if (!b) {
    return;
  }

  struct spa_buffer *buf = b->buffer;
  struct spa_data *d = &buf->datas[0];
  auto *dst = static_cast<int16_t *>(d->data);
  if (!dst) {
    pw_stream_queue_buffer(stream, b);
    return;
  }

  int32_t stride = sizeof(int16_t) * CHANNELS;
  uint32_t n_frames = d->maxsize / stride;
  if (b->requested) {
    n_frames = SPA_MIN(b->requested, n_frames);
  }

  d->chunk->offset = 0;
  d->chunk->stride = stride;
  d->chunk->size = n_frames * stride;

  float slip = g_max_slip.load(std::memory_order_relaxed);

  static float phase = 0.0f;

  if (slip < SLIP_LOW) {
    memset(dst, 0, n_frames * 2 * sizeof(uint16_t));
    phase = 0.f;
  } else {
    for (uint32_t i = 0; i < n_frames; ++i) {
      int16_t val = (int16_t)(sinf(phase) * 32767.0f * (slip >= SLIP_HIGH ? VOLUME_HIGH : VOLUME_LOW));
      phase += TWO_PI * float(cli_args.frequency) / SAMPLE_RATE;
      if (phase >= TWO_PI) {
        phase -= TWO_PI;
      }
      dst[i * CHANNELS] = val;
      dst[i * CHANNELS + 1] = val;
    }
  }

  pw_stream_queue_buffer(stream, b);
}

static void on_stream_state_changed([[maybe_unused]] void *userdata, [[maybe_unused]] enum pw_stream_state old,
                                    enum pw_stream_state state, const char *error) {
  switch (state) {
  case PW_STREAM_STATE_ERROR:
    fprintf(stderr, "Stream error: %s\n", error);
    [[fallthrough]];
  case PW_STREAM_STATE_UNCONNECTED:
    if (main_loop) {
      pw_main_loop_quit(main_loop);
    }
    break;
  case PW_STREAM_STATE_STREAMING:
    printf("Audio streaming...\n");
    break;
  default:
    break;
  }
}

static void on_stream_destroy([[maybe_unused]] void *userdata) {
  if (main_loop) {
    pw_main_loop_quit(main_loop);
  }
}

static void telemetry_loop() {
  int pid = wait_for_ams2_pid();
  const void *remote_addr = wait_for_ams2_telemetry_address(pid);

  while (g_running.load(std::memory_order_relaxed)) {
    ams2_telemetry tele;
    if (read_ams2_telemetry(pid, &tele, remote_addr)) {
      float x_vel = tele.localVelocity[2];
      float fl = compute_slip(tele.tyreRPS[0], x_vel);
      float fr = compute_slip(tele.tyreRPS[1], x_vel);
      float rl = compute_slip(tele.tyreRPS[2], x_vel);
      float rr = compute_slip(tele.tyreRPS[3], x_vel);
      float max_slip = std::max({fl, fr, rl, rr});
      g_max_slip.store(max_slip, std::memory_order_relaxed);
    }
    usleep(TELEMETRY_POLL_MS * 1000);
  }
}

static void print_usage(const char *prog) {
  printf("Usage: %s [options]\n"
         "  -f, --frequency HZ    Tone frequency in Hz (default: %d)\n"
         "  -s, --sink NAME       Target sink node name or serial\n"
         "  -h, --help            Show this help\n"
         "\n"
         "Plays a tone on wheel slip for haptic feedback.\n"
         "\n"
         "pw-cli can be used to discover sink names and serials. Example:\n"
         "pw-cli list-objects Node | grep -e object.serial -e node.name -e Audio/Sink | grep -B2 'Audio/Sink'",
         prog, DEFAULT_FREQUENCY);
}

int main(int argc, char *argv[]) {
  static const struct option long_opts[] = {
      {"frequency", required_argument, nullptr, 'f'},
      {"sink", required_argument, nullptr, 's'},
      {"help", no_argument, nullptr, 'h'},
      {nullptr, 0, nullptr, 0},
  };

  int opt;
  while ((opt = getopt_long(argc, argv, "f:s:h", long_opts, nullptr)) != -1) {
    switch (opt) {
    case 'f':
      cli_args.frequency = atoi(optarg);
      break;
    case 's':
      cli_args.sink_name = optarg;
      break;
    case 'h':
      print_usage(argv[0]);
      return 0;
    default:
      print_usage(argv[0]);
      return 1;
    }
  }

  pw_init(&argc, &argv);

  struct pw_properties *props =
      pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio", PW_KEY_MEDIA_CATEGORY, "Playback", PW_KEY_MEDIA_ROLE, "Music",
                        PW_KEY_NODE_NAME, "ams2-haptics", PW_KEY_NODE_DESCRIPTION, "Sim racing haptics", nullptr);
  if (!props) {
    fprintf(stderr, "pw_properties_new failed\n");
    return 1;
  }

  if (cli_args.sink_name) {
    pw_properties_set(props, PW_KEY_TARGET_OBJECT, cli_args.sink_name);
  }

  main_loop = pw_main_loop_new(nullptr);
  if (!main_loop) {
    fprintf(stderr, "pw_main_loop_new failed\n");
    return 1;
  }

  static struct pw_stream_events stream_events;
  memset(&stream_events, 0, sizeof(stream_events));
  stream_events.version = PW_VERSION_STREAM_EVENTS;
  stream_events.destroy = on_stream_destroy;
  stream_events.state_changed = on_stream_state_changed;
  stream_events.process = on_process;

  stream = pw_stream_new_simple(pw_main_loop_get_loop(main_loop), "ams2-haptics", props, &stream_events, nullptr);
  if (!stream) {
    fprintf(stderr, "pw_stream_new_simple failed\n");
    return 1;
  }

  char pod_buffer[1024];
  struct spa_pod_builder b = SPA_POD_BUILDER_INIT(pod_buffer, sizeof(pod_buffer));

  struct spa_audio_info_raw info;
  memset(&info, 0, sizeof(info));
  info.format = SPA_AUDIO_FORMAT_S16;
  info.rate = SAMPLE_RATE;
  info.channels = CHANNELS;

  const struct spa_pod *params;
  params = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &info);
  if (!params) {
    fprintf(stderr, "spa_format_audio_raw_build failed\n");
    return 1;
  }

  if (pw_stream_connect(stream, PW_DIRECTION_OUTPUT, PW_ID_ANY,
                        static_cast<enum pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS |
                                                          PW_STREAM_FLAG_RT_PROCESS),
                        &params, 1) < 0) {
    fprintf(stderr, "pw_stream_connect failed\n");
    return 1;
  }

  std::jthread telemetry_thread(telemetry_loop);

  pw_main_loop_run(main_loop);

  g_running.store(false, std::memory_order_relaxed);
  telemetry_thread.request_stop();
  telemetry_thread.join();

  pw_stream_destroy(stream);
  pw_main_loop_destroy(main_loop);
  pw_deinit();

  return 0;
}
