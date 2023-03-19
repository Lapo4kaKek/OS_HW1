#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

void countingNumbers(char *str) {
    int count = 0;
    char *start = str;
    while (*str != '\0') {
        if (*str >= '0' && *str <= '9') {
            while (*str != '\0' && *str >= '0' && *str <= '9') {
                str++;
            }
            count++;
        } else {
            str++;
        }
    }

    char result[32];
    sprintf(result, "%d", count);
    strcpy(start, result);
}

const int buffer_size = 5000;
int main(int argc, char *argv[]) {
    const char *first_channel = "first_channel.fifo";
    const char *second_channel = "second_channel.fifo";

    int canal_12, canal_23;
    int size;
    char data[buffer_size];

    mknod(first_channel, S_IFIFO | 0666, 0);
    mknod(second_channel, S_IFIFO | 0666, 0);

    canal_12 = open(first_channel, O_RDONLY);
    if (canal_12 < 0) {
        printf("Can't open FIFO for reading\n");
        return -1;
    }

    // Считывание из первого канала
    size = read(canal_12, data, buffer_size);
    printf("second: I wake up!!!\n");
    if (size < 0) {
        printf("Can't read data from FIFO\n");
        return -1;
    }

    printf("I call countingNumbers...\n");
    // вызываем функцию подсчета количества чисел в data
    countingNumbers(data);
    printf("countingNumbers end.\n");

    if (close(canal_12) < 0) {
        printf("child: I can't close FIFO\n");
        return -1;
    }

    canal_23 = open(second_channel, O_WRONLY);

    if (canal_23 < 0) {
        printf("Can't open FIFO for writing\n");
        return -1;
    }

    size = write(canal_23, data, buffer_size); // записываем во второй канал

    if (size != buffer_size) {
        printf("Can't write all data to FIFO\n (size = %d)", size);
        return -1;
    }

    if (close(canal_23) < 0) {
        printf("parent: Can't close FIFO\n");
        return -1;
    }

    return 0;
}