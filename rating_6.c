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
    char data[buffer_size];
    if (argc != 3) {
        printf("Incorrect input command line arguments\n");
        return 0;
    }

    int fd_12[2], fd_23[2];
    int process_1, size;

    // создаём первый канал (1 -> 2)
    if (pipe(fd_12) < 0) {
        printf("Can't open pipe\n");
        return -1;
    }
    // создаём второй канал (2 -> 3)
    if (pipe(fd_23) < 0) {
        printf("Can't open pipe\n");
        return -1;
    }

    // форкаем первый процесс (создаем первые два процесса)
    process_1 = fork();
    if (process_1 < 0) {
        printf("Can't fork child\n");
        return -1;
    } else if (process_1 > 0) { // Первый процесс
        if (close(fd_12[0]) < 0) {
            printf("parent: Can't close reading side of pipe\n");
            return -1;
        }

        int input = open(argv[1], O_RDONLY, 0666);

        if (input < 0) {
            printf("Can't open file\n");
            return -1;
        }

        read(input, data, buffer_size); // считываем из файла
        if (close(input) < 0) {
            printf("Can't close file\n");
        }

        // записываем данные в первый канал
        size = write(fd_12[1], data, buffer_size);

        if (close(fd_12[1]) < 0) {
            printf("parent: Can't close writing side of pipe\n");
            return -1;
        }
        // если записалось не полностью
        if (size != buffer_size) {
            printf("Can't write all data to pipe\n");
            return -1;
        }
    } else {  // второй процесс
        if (close(fd_12[1]) < 0) {
            printf("child: Can't close writing side of pipe\n");
            return -1;
        }
        // считываем из первого канала
        size = read(fd_12[0], data, buffer_size);
        if (size < 0) {
            printf("Can't read data from pipe\n");
            return -1;
        }

        printf("I call countingNumbers...\n");
        // вызываем функцию подсчета количества чисел в data
        countingNumbers(data);
        printf("countingNumbers end.\n");

        if (close(fd_12[0]) < 0) {
            printf("child: Can't close reading side of pipe\n");
            return -1;
        }

        // записываем во второй канал
        size = write(fd_23[1], data, buffer_size);

        if(size != buffer_size){
            printf("Can't write all data to pipe\n (size = %d)",size);
            return -1;
        }
        if(close(fd_23[1]) < 0) {
            printf("parent: Can't close writing side of pipe\n");
            return -1;
        }
        exit(0);
    } // первый процесс

    // ждем
    while(wait(NULL) > 0) {

    }

    if(close(fd_23[1]) < 0){
        printf("child: I Can't close writing side of pipe\n");
        return -1;
    }

    // считываем из второго канала
    size = read(fd_23[0], data, buffer_size);
    if(size < 0){
        printf("I Can't read data from pipe\n");
        return -1;
    }
    if(close(fd_23[0]) < 0){
        printf("child: Can\'t close reading side of pipe\n");
        return -1;
    }
    int output = open(argv[2], O_WRONLY | O_CREAT, 0666);

    if(output < 0) {
        printf("Can't open file\n");
        return -1;
    }

    // записываем ответ
    size = write(output, data, strlen(data));

    // проверка на закрытие файла
    if(close(output) < 0) {
        printf("Can't close file\n");
    }
    // проверяем, данные записались полностью
    if(size != strlen(data)) {
        printf("Can't write all data\n");
        return -1;
    }
    return 0;
}