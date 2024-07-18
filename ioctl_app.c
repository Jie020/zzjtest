#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/ioctl_example" // 设备文件路径
#define IOCTL_ALLOC_MEMORY _IO('k', 1)   // ioctl命令：申请内存
#define IOCTL_STORE_DATA _IOW('k', 2, char *) // ioctl命令：存储数据到内核空间
#define IOCTL_EXPORT_DATA _IOR('k', 3, char *) // ioctl命令：导出数据到用户空间

int main() {
    int fd;
    char *buffer;
    int file_fd;
    int file_size;

    // 打开设备文件
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return -1;
    }

    // 第一步：将一个文件写入内核申请的内存中
    file_fd = open("test.txt", O_RDONLY);
    if (file_fd < 0) {
        perror("Failed to open file");
        close(fd);
        return -1;
    }

    // 获取文件大小
    file_size = lseek(file_fd, 0, SEEK_END);
    lseek(file_fd, 0, SEEK_SET);

    // 使用ioctl命令申请内存空间
    if (ioctl(fd, IOCTL_ALLOC_MEMORY, file_size) < 0) {
        perror("IOCTL_ALLOC_MEMORY failed");
        close(file_fd);
        close(fd);
        return -1;
    }

    // 分配用户空间的缓冲区并将文件内容读取到缓冲区
    buffer = malloc(file_size);
    if (!buffer) {
        perror("Memory allocation failed");
        close(file_fd);
        close(fd);
        return -1;
    }

    read(file_fd, buffer, file_size);

    // 使用ioctl命令将数据从用户空间写入内核空间
    if (ioctl(fd, IOCTL_STORE_DATA, buffer) < 0) {
        perror("IOCTL_STORE_DATA failed");
        free(buffer);
        close(file_fd);
        close(fd);
        return -1;
    }

    free(buffer);
    close(file_fd);

    // 第二步：将内核空间的数据读出并写入一个文件中
    buffer = malloc(file_size);
    if (!buffer) {
        perror("Memory allocation failed");
        close(fd);
        return -1;
    }

    // 使用ioctl命令将数据从内核空间读取到用户空间
    if (ioctl(fd, IOCTL_EXPORT_DATA, buffer) < 0) {
        perror("IOCTL_EXPORT_DATA failed");
        free(buffer);
        close(fd);
        return -1;
    }

    file_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd < 0) {
        perror("Failed to open output file");
        free(buffer);
        close(fd);
        return -1;
    }

    write(file_fd, buffer, file_size); // 将数据写入输出文件

    free(buffer);
    close(file_fd);
    close(fd);

    return 0;
}

