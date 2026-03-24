#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
        return 1;
    }

    char maps_path[256];
    snprintf(maps_path, sizeof(maps_path), "/proc/%s/maps", argv[1]);

    FILE *fp = fopen(maps_path, "r");
    if (!fp) {
        perror("Failed to open maps");
        return 1;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        unsigned long start, end, offset;
        char perms[5];

        if (sscanf(line, "%lx-%lx %4s %lx", &start, &end, perms, &offset) != 4) {
            continue;
        }

        if (perms[0] != 'r' || perms[1] != 'w' || perms[2] != '-' || perms[3] != 's' || offset || (end - start) != 0x6000 || !strstr(line, "tmpmap-")) {
            continue;
        }

        printf("%s", line);
    }

    fclose(fp);
    return 0;
}
