#define _GNU_SOURCE
#include <ctype.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

// Size of shared mem region created by AMS2
const off_t REGION_SIZE = 0x6000;

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

        if (perms[0] != 'r' || perms[1] != 'w' || perms[2] != '-' || perms[3] != 's' || offset || (end - start) != REGION_SIZE || !strstr(line, "tmpmap-")) {
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

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
        return 1;
    }

    pid_t pid = (pid_t)atoi(argv[1]);
    off_t region_start = get_telemetry_sharedmem_address(pid);

    uint8_t data[REGION_SIZE];
    read_proc_memory(pid, region_start, data, REGION_SIZE);

    printf("Read %zu bytes from remote process\n", REGION_SIZE);
    printf("First 128 bytes: ");
    for (int i = 0; i < 128; i++) {
        uint8_t v = data[i];
        printf("%d:\t%02x %c\n", i, v, isalnum(v) ? v : '.');
    }
    printf("\n");

    return 0;
}
