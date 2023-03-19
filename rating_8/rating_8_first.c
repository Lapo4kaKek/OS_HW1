#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

const int buffer_size = 5000;
int main(int argc, char *argv[]) {
    const char *first_channel = "first_channel.fifo";
    const char *second_channel = "second_channel.fifo";

    (void) umask(0);

    if (argc != 3) {
        printf("Incorrect input command line arguments\n");
        return 0;
    }

    int canal_12, canal_23, size;
    char data[buffer_size];

    mknod(first_channel, S_IFIFO | 0666, 0);
    mknod(second_channel, S_IFIFO | 0666, 0);

    // Первый процесс - считать из файла и направить во второй процесс
    int input = open(argv[1], O_RDONLY, 0666);

    if (input < 0) {
        printf("Can't open file\n");
        return -1;
    }

    read(input, data, buffer_size);

    if (close(input) < 0) {
        printf("Can't close file\n");
    }

    canal_12 = open(first_channel, O_WRONLY);
    if (canal_12 < 0) {
        printf("Can't open FIFO for writing\n");
        return -1;
    }

    // записываем в первый канал
    size = write(canal_12, data, buffer_size);

    printf("first: I wait second...\n");
    if (size != buffer_size) {
        printf("Can't write all data to FIFO\n");
        return -1;
    }
    if (close(canal_12) < 0) {
        printf("parent: I can't close writing side of FIFO\n");
        return -1;
    }
    canal_23 = open(second_channel, O_RDONLY);
    if (canal_23 < 0) {
        printf("Can't open FIFO for reading\n");
        return -1;
    }

    // Считываем результат выполнения функции из второго канала и записываем в файл
    size = read(canal_23, data, buffer_size);
    // проверка, если считалось не всё
    if (size < 0) {
        printf("Can't read data from FIFO\n");
        return -1;
    }

    if (close(canal_23) < 0) {
        printf("Can't close FIFO\n");
        return -1;
    }

    int output = open(argv[2], O_WRONLY | O_CREAT, 0666);

    // проверка на открытие
    if (output < 0) {
        printf("Can't open file\n");
        return -1;
    }

    size = write(output, data, strlen(data));
    // проверка на закрытие файла
    if (close(output) < 0) {
        printf("Can't close file\n");
    }

    // проверка на всё ли записалось
    if (size != strlen(data)) {
        printf("Can't write all data\n");
        return -1;
    }
    unlink(first_channel);
    unlink(second_channel);

    return 0;
}