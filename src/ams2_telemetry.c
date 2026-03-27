#define _GNU_SOURCE
#include "ams2_telemetry.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>

// Get AMS2 PID
int get_ams2_pid() {
  DIR *proc_dir = opendir("/proc");
  if (!proc_dir) {
    perror("opendir /proc");
    return 0;
  }

  int result = 0;
  struct dirent *entry;
  while (!result && (entry = readdir(proc_dir)) != NULL) {
    if (entry->d_type != DT_DIR) {
      continue;
    }

    int pid = atoi(entry->d_name);
    if (!pid) {
      continue;
    }

    char comm_path[256];
    snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", pid);

    FILE *comm_file = fopen(comm_path, "r");
    if (!comm_file) {
      continue;
    }

    char comm[256];
    if (fgets(comm, sizeof(comm), comm_file)) {
      if (strcmp(comm, "AMS2AVX.exe\n") == 0) {
        result = pid;
      }
    }
    fclose(comm_file);
  }

  closedir(proc_dir);
  return result;
}

// Size of shared mem region created by AMS2
const off_t REGION_SIZE = 0x6000;

// Scan /proc/.../maps, and find virtual address in AMS2 space that hold telemetry shared mem
void *get_ams2_telemetry_address(int pid) {
  char maps_path[256];
  snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

  FILE *fp = fopen(maps_path, "r");
  if (!fp) {
    perror("Failed to open maps");
    return NULL;
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

    // printf("Found AMS2 telemetry shared mem at %lx\n", start);
    fclose(fp);
    return (void *)start;
  }

  // fprintf(stderr, "Can't find AMS2 telemetry shared mem. Make sure it's enabled in AMS2 settings\n");
  fclose(fp);
  return NULL;
}

// Read memory from another process
void read_ams2_telemetry(int pid, ams2_telemetry *local_addr, void *remote_addr) {
  struct iovec local_iov = {
      .iov_base = (void *)local_addr,
      .iov_len = sizeof(ams2_telemetry),
  };

  struct iovec remote_iov = {
      .iov_base = (void *)remote_addr,
      .iov_len = sizeof(ams2_telemetry),
  };

  ssize_t nread = process_vm_readv(pid, &local_iov, 1, &remote_iov, 1, 0);
  if (nread < 0) {
    perror("process_vm_readv"); // AMS2 exit, or not enough permissions
    exit(1);
  }
}
