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

int main(int argc, char *argv[]) {
    const int buffer_size = 5000;
    const char *pipe_1_fifo = "pipe1.fifo";
    const char *pipe_2_fifo = "pipe2.fifo";
    if (argc != 3) {
        printf("Incorrect input command line arguments\n");
        return 0;
    }

    int canal_12, canal_23;
    int result, size;
    char data[buffer_size];

    mknod(pipe_1_fifo, S_IFIFO | 0666, 0);
    mknod(pipe_2_fifo, S_IFIFO | 0666, 0);

    // создаём первые два процесса
    result = fork();

    if (result < 0) {
        printf("Can't fork child\n");
        exit(-1);
    } else if (result > 0) { // первый процесс
        int input = open(argv[1], O_RDONLY, 0666);
        if (input < 0) {
            printf("Can't open file\n");
            return -1;
        }

        //  читаем в data
        read(input, data, buffer_size);
        if (close(input) < 0) {
            printf("Can't close file\n");
        }

        canal_12 = open(pipe_1_fifo, O_WRONLY);
        if (canal_12 < 0) {
            printf("Can't open FIFO for writing\n");
            return -1;
        }

        // записываем в первый канал
        size = write(canal_12, data, buffer_size);
        if (size != buffer_size) {
            printf("Can't write all data to FIFO\n");
            return -1;
        }
        if (close(canal_12) < 0) {
            printf("parent: Can't close writing side of FIFO\n");
            return -1;
        }
    } else {  // второй процесс
        canal_12 = open(pipe_1_fifo, O_RDONLY);
        if (canal_12 < 0) {
            printf("Can't open FIFO for reading\n");
            return -1;
        }

        // получаем из первого канала data
        size = read(canal_12, data, buffer_size);

        if (size < 0) {
            printf("Can't read data from FIFO\n");
            return -1;
        }

        printf("I call countingNumbers...\n");
        // вызываем функцию подсчета количества чисел в data
        countingNumbers(data);
        printf("countingNumbers end.\n");
        // если не удалось закрыть канал
        if (close(canal_12) < 0) {
            printf("child: Can't close FIFO\n");
            return -1;
        }

        // открываем канал для записи
        canal_23 = open(pipe_2_fifo, O_WRONLY);
        // если не удалось открыть канал для записи, начинаем плакать в консоль и завершаем работу
        if (canal_23 < 0) {
            printf("Can't open FIFO for writing\n");
            return -1;
        }

        // канал открыт, можно записывать данные во второй канал
        size = write(canal_23, data, buffer_size);

        // проверки если не удалось закрыть fifo
        if (close(canal_23) < 0) {
            printf("parent: Can't close FIFO\n");
            return -1;
        }
        // если записались не все данные
        if (size != buffer_size) {
            printf("Can't write all data to FIFO\n");
            return -1;
        }
        return -1;
    }

    // открываем канал
    canal_23 = open(pipe_2_fifo, O_RDONLY);
    // если не удалось открыть канал
    if (canal_23 < 0) {
        printf("Can't open FIFO for reading\n");
        return -1;
    }

    // считываем из второго канала
    size = read(canal_23, data, buffer_size);
    if (size < 0) {
        printf("Can't read data from FIFO\n");
        return -1;
    }
    if (close(canal_23) < 0) {
        printf("Can't close FIFO\n");
        return -1;
    }

    int output = open(argv[2], O_WRONLY | O_CREAT, 0666);

    if (output < 0) {
        printf("Can't open file\n");
        return -1;
    }

    size = write(output, data, strlen(data));
    if (size != strlen(data)) {
        printf("Can't write all data\n");
        return -1;
    }
    if (close(output) < 0) {
        printf("Can't close file\n");
    }

    unlink(pipe_1_fifo);
    unlink(pipe_2_fifo);

    return 0;
}