#include "ams2_telemetry.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>

const int BAR_WIDTH = 50;

static const char *CSI = "\033[";
static const char *RED = "\033[31m";
static const char *GREEN = "\033[32m";
static const char *BLUE = "\033[34m";
static const char *RESET = "\033[0m";

static void clear_screen(void) { printf("%s2J%sH", CSI, CSI); }

static void print_bar(const char *label, const char *color, float value) {
  int len = (int)(value * BAR_WIDTH);
  printf("%s%-10s%s [%s", color, label, RESET, CSI);
  for (int i = 0; i < BAR_WIDTH; i++) {
    if (i < len) {
      printf("%s█", color);
    } else {
      printf("░");
    }
  }
  printf("%s] %5.1f%%\n", RESET, value * 100);
}

int main() {
  int pid;
  do {
    pid = get_ams2_pid();
    if (pid) {
      printf("Found AMS2 running with PID %d\n", pid);
    } else {
      printf("No AMS2 process found. Retrying...\n");
      sleep(1);
    }
  } while (!pid);

  void *remote_addr;
  do {
    remote_addr = get_ams2_telemetry_address(pid);
    if (!remote_addr) {
      printf("AMS2 telemetry shared memory not found. Make sure AMS2 is fully initialized, and has its shared memory "
             "enabled in settings. Retrying...\n");
      sleep(1);
    }
  } while (!remote_addr);

  struct timespec ts = {
      .tv_sec = 0,
      .tv_nsec = 20 * 1000 * 1000, // 20ms = 50Hz
  };

  while (1) {
    ams2_telemetry tele;
    read_ams2_telemetry(pid, &tele, remote_addr);

    clear_screen();
    printf("AMS2 Telemetry\n");
    printf("========================================\n");

    print_bar("Throttle", GREEN, tele.mUnfilteredThrottle);
    print_bar("Brake", RED, tele.mUnfilteredBrake);
    print_bar("Steering", BLUE, (tele.mUnfilteredSteering + 1.0f) / 2.0f);

    printf("\n");
    fflush(stdout);
    nanosleep(&ts, NULL);
  }

  return 0;
}
