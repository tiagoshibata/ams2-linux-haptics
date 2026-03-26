#define _GNU_SOURCE
#include <fcntl.h>
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
const int BAR_WIDTH = 50;

static const char *CSI = "\033[";
static const char *RED = "\033[31m";
static const char *GREEN = "\033[32m";
static const char *BLUE = "\033[34m";
static const char *RESET = "\033[0m";

static void clear_screen(void) { printf("%s2J%sH", CSI, CSI); }

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

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
    return 1;
  }

  pid_t pid = (pid_t)atoi(argv[1]);
  off_t region_start = get_telemetry_sharedmem_address(pid);

  struct timespec ts = {
      .tv_sec = 0,
      .tv_nsec = 20 * 1000 * 1000, // 20ms = 50Hz
  };

  while (1) {
    ams2_shmem data;
    read_proc_memory(pid, region_start, &data, sizeof(data));

    clear_screen();
    printf("AMS2 Telemetry (50Hz)\n");
    printf("========================================\n");

    print_bar("Throttle", GREEN, data.mUnfilteredThrottle);
    print_bar("Brake", RED, data.mUnfilteredBrake);
    print_bar("Steering", BLUE, (data.mUnfilteredSteering + 1.0f) / 2.0f);

    printf("\n");
    fflush(stdout);
    nanosleep(&ts, NULL);
  }

  return 0;
}
