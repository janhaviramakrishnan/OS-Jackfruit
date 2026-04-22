#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "monitor_ioctl.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <PID>\n", argv[0]);
        return 1;
    }

    int fd = open("/dev/container_monitor", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct monitor_request req;

    memset(&req, 0, sizeof(req));

    strcpy(req.container_id, "alpha");
    req.pid = atoi(argv[1]);

    // 🔥 IMPORTANT: small limits so monitor triggers BEFORE OOM
    req.soft_limit_bytes = 20 * 1024 * 1024;   // 20 MB
    req.hard_limit_bytes = 40 * 1024 * 1024;   // 40 MB

    if (ioctl(fd, MONITOR_REGISTER, &req) < 0) {
        perror("ioctl");
        close(fd);
        return 1;
    }

    printf("Registered PID %d\n", req.pid);

    close(fd);
    return 0;
}
