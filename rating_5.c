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
    const char *pipe_1 = "pipe1.fifo";
    const char *pipe_2 = "pipe2.fifo";
    if (argc != 3) {
        printf("Incorrect input command line arguments\n");
        return -1;
    }

    int canal_12, canal_23;
    int process_1, size, process_2;
    char data[buffer_size] = {0};

    mknod(pipe_1, S_IFIFO | 0666, 0);
    mknod(pipe_2, S_IFIFO | 0666, 0);

    // форкаем процесс (создаём первые два процесса)
    process_1 = fork();
    if (process_1 < 0) {
        printf("Can\'t fork child\n");
        return -1;
    } else if (process_1 > 0) { // первый процесс
        int input = open(argv[1], O_RDONLY, 0666);
        if (input < 0) {
            printf("Can't open file\n");
            return -1;
        }

        read(input, data, buffer_size);
        if (close(input) < 0) {
            printf("Can't close file\n");
        }

        canal_12 = open(pipe_1, O_WRONLY);
        if (canal_12 < 0) {
            printf("Can't open FIFO for writing\n");
            return -1;
        }

        // записываем в первый канал data
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
        // форкаем второй процесс (создаём третий процесс из второго)
        process_2 = fork();
        if (process_2 < 0) {
            printf("Can't fork child\n");
            return -1;
        } else if (process_2 > 0) { // второй процесс
            canal_12 = open(pipe_1, O_RDONLY);
            if (canal_12 < 0) {
                printf("Can't open FIFO for reading\n");
                return -1;
            }
            // считываем данные из первого канала
            size = read(canal_12, data, buffer_size);

            if (size < 0) {
                printf("Can't read data from FIFO\n");
                return -1;
            }

            printf("I call countingNumbers...\n");
            // вызываем функцию подсчета количества чисел в data
            countingNumbers(data);
            printf("countingNumbers end.\n");

            if (close(canal_12) < 0) {
                printf("child: Can't close FIFO\n");
                return -1;
            }
            if ((canal_23 = open(pipe_2, O_WRONLY)) < 0) {
                printf("Can't open FIFO for writing\n");
                return -1;
            }

            // записываем во второй канал
            size = write(canal_23, data, buffer_size);

            if (size != buffer_size) {
                printf("Can't write all data to FIFO\n (size = %d)", size);
                return -1;
            }
            if (close(canal_23) < 0) {
                printf("parent: Can't close FIFO\n");
                return -1;
            }
        } else { // третий процесс
            if ((canal_23 = open(pipe_2, O_RDONLY)) < 0) {
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

            // записываем ответ на задачу в файл
            size = write(output, data, strlen(data));
            // проверка на корректное закрытие
            if (close(output) < 0) {
                printf("Can't close file\n");
            }

            // проверка на полную запись
            if (size != strlen(data)) {
                printf("Can't write all data\n");
                return -1;
            }
        }
    }
    return 0;
}