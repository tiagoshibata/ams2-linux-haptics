#define _GNU_SOURCE
#include <getopt.h>
#include <math.h>
#include <pipewire/pipewire.h>
#include <stdint.h>
#include <stdio.h>

#include <spa/param/audio/raw-utils.h>
#include <spa/param/audio/raw.h>
#include <spa/param/param.h>

#define SAMPLE_RATE 48000u
#define CHANNELS 2u
#define FREQUENCY 400 // TODO 40
#define VOLUME 0.7f
#define TWO_PI (float)(2 * M_PI)

static struct {
  const char *sink_name;
} cli_args = {};

static struct pw_main_loop *main_loop = NULL;
static struct pw_stream *stream = NULL;

static void on_process([[maybe_unused]] void *userdata) {
  struct pw_buffer *b = pw_stream_dequeue_buffer(stream);
  if (!b) {
    pw_log_warn("out of buffers: %m");
    return;
  }

  struct spa_buffer *buf = b->buffer;
  struct spa_data *d = &buf->datas[0];
  int16_t *dst = d->data;
  if (!dst)
    return;

  int32_t stride = sizeof(int16_t) * CHANNELS;
  uint32_t n_frames = d->maxsize / stride;
  if (b->requested) {
    n_frames = SPA_MIN(b->requested, n_frames);
  }

  d->chunk->offset = 0;
  d->chunk->stride = stride;
  d->chunk->size = n_frames * stride;

  for (uint32_t i = 0; i < n_frames; ++i) {
    static float x = 0;
    x += TWO_PI * FREQUENCY / SAMPLE_RATE;
    if (x > TWO_PI) {
      x -= TWO_PI;
    }

    float val = sinf(x);
    val *= 32767.0f * VOLUME;
    dst[i * CHANNELS] = (int16_t)val;
    dst[i * CHANNELS + 1] = (int16_t)val;
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
    if (main_loop)
      pw_main_loop_quit(main_loop);
    break;

  case PW_STREAM_STATE_STREAMING:
    printf("Audio streaming...\n");
    break;

  default:
    break;
  }
}

static void on_stream_destroy(void *userdata) {
  (void)userdata;
  if (main_loop)
    pw_main_loop_quit(main_loop);
}

static void print_usage(const char *prog) {
  const char *help =
      "Usage: %s [options]\n"
      "  -f, --frequency HZ    Sine wave frequency in Hz (default: 40)\n"
      "  -s, --sink NAME       Target sink node name or serial\n"
      "  -h, --help            Show this help\n"
      "\n"
      "pw-cli can be used to discover sink names and serials. Example:\n"
      "pw-cli list-objects Node | grep -e object.serial -e node.name -e Audio/Sink | grep -B2 'Audio/Sink'";

  printf(help, prog);
}

int main(int argc, char *argv[]) {
  static const struct option long_opts[] = {
      {"frequency", required_argument, NULL, 'f'},
      {"sink", required_argument, NULL, 's'},
      {"help", no_argument, NULL, 'h'},
      {NULL, 0, NULL, 0},
  };

  int opt;
  while ((opt = getopt_long(argc, argv, "f:s:h", long_opts, NULL)) != -1) {
    switch (opt) {
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
                        PW_KEY_NODE_NAME, "ams2-haptics", PW_KEY_NODE_DESCRIPTION, "Sim racing haptics", NULL);
  if (!props) {
    fprintf(stderr, "pw_properties_new failed\n");
    return 1;
  }

  if (cli_args.sink_name) {
    pw_properties_set(props, PW_KEY_TARGET_OBJECT, cli_args.sink_name);
  }

  main_loop = pw_main_loop_new(NULL);
  if (!main_loop) {
    fprintf(stderr, "pw_main_loop_new failed\n");
    return 1;
  }

  static const struct pw_stream_events stream_events = {
      PW_VERSION_STREAM_EVENTS,
      .destroy = on_stream_destroy,
      .state_changed = on_stream_state_changed,
      .process = on_process,
  };

  stream = pw_stream_new_simple(pw_main_loop_get_loop(main_loop), "ams2-haptics", props, &stream_events, NULL);
  if (!stream) {
    fprintf(stderr, "pw_stream_new_simple failed\n");
    return 1;
  }

  char pod_buffer[1024];
  struct spa_pod_builder b = SPA_POD_BUILDER_INIT(pod_buffer, sizeof(pod_buffer));

  struct spa_audio_info_raw info = {
      .format = SPA_AUDIO_FORMAT_S16,
      .rate = SAMPLE_RATE,
      .channels = CHANNELS,
  };

  const struct spa_pod *params;
  params = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &info);
  if (!params) {
    fprintf(stderr, "spa_format_audio_raw_build failed\n");
    return 1;
  }

  if (pw_stream_connect(stream, PW_DIRECTION_OUTPUT, PW_ID_ANY,
                        PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_RT_PROCESS, &params,
                        1) < 0) {
    fprintf(stderr, "pw_stream_connect failed\n");
    return 1;
  }

  pw_main_loop_run(main_loop);

  pw_stream_destroy(stream);
  pw_main_loop_destroy(main_loop);
  pw_deinit();

  return 0;
}
