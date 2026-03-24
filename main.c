#include <ctype.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

// Size of shared mem region created by AMS2
const size_t REGION_SIZE = 0x6000;

// Scan /proc/.../maps, and find virtual address in AMS2 space that hold telemetry shared mem
off_t get_telemetry_sharedmem_address(char* pid) {
    char maps_path[256];
    snprintf(maps_path, sizeof(maps_path), "/proc/%s/maps", pid);

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

// Map address from another process
void* map_proc_address(char* pid, off_t offset) {
    char mem_path[256];
    snprintf(mem_path, sizeof(mem_path), "/proc/%s/mem", pid);

    int mem_fd = open(mem_path, O_RDONLY);
    if (mem_fd < 0) {
        perror("Failed to open AMS2's mem. Make sure ams2-linux-haptics is run with elevated permissions");
        exit(1);
    }

    void *mapped = mmap(NULL, REGION_SIZE, PROT_READ, MAP_PRIVATE, mem_fd, offset);
    if (mapped == MAP_FAILED) {
        perror("mmap");
        close(mem_fd);
        exit(1);
    }

    close(mem_fd);
    return mapped;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
        return 1;
    }

    off_t region_start = get_telemetry_sharedmem_address(argv[1]);
    void* data = map_proc_address(argv[1], region_start);

    printf("Mapped region at %p\n", data);
    printf("First 128 bytes: ");
    for (int i = 0; i < 128; i++) {
        uint8_t v = ((uint8_t *)data)[i];
        printf("%02x %c", v, isalnum(v) ? v : ' ');
    }
    printf("\n");

    munmap(data, REGION_SIZE);

    return 0;
}
