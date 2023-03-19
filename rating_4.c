#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

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
        return -1;
    }
    char *input_file_title = argv[1];
    char *output_file_title = argv[2];
    printf("Okey, input file: %s\nOutput file: %s\n", input_file_title, output_file_title);
    int canal[2], canal_23[2], process_1, size, process_2;

    // создаём первый канал (для передачи из первого во второй процесс)
    if (pipe(canal) < 0) {
        printf("Can't open pipe\n");
        return -1;
    }
    // создаём второй канал (для передачи из второго в третий процесс)
    if (pipe(canal_23) < 0) {
        printf("Can\'t open pipe\n");
        return -1;
    }

    // создаём два первых процесса
    process_1 = fork();
    if (process_1 < 0) {
        printf("I can't fork child...(\n");
        return -1;
    } else if (process_1 > 0) { // первый процесс
        if (close(canal[0]) < 0) {
            printf("parent: I can't close reading pipe\n");
            return -1;
        }
        int input_file = open(input_file_title, O_RDONLY, 0666);
        if (input_file < 0) {
            printf("Can't open file\n");
            return -1;
        }
        read(input_file, data, buffer_size);

        if (close(input_file) < 0) {
            printf("Can't close file\n");
        }

        // записываем данные в первый канал
        size = write(canal[1], data, buffer_size);

        if (size != buffer_size) {
            printf("Can't write all data to pipe");
            return -1;
        }

        if (close(canal[1]) < 0) {
            printf("parent: Can't close writing side of pipe\n");
            return -1;
        }
    } else {  // второй процесс
        process_2 = fork(); // создаём третий процесс из второго

        if (process_2 < 0) {
            printf("Can't fork child\n");
            return -1;
        } else if (process_2 > 0) { // второй процесс
            if (close(canal[1]) < 0) {
                printf("child: Can't close writing side of pipe\n");
                return -1;
            }

            // считываем из первого канала data
            size = read(canal[0], data, buffer_size);

            if (size < 0) {
                printf("Can't read data from pipe\n");
                return -1;
            }

            printf("I call func countingNumbers...\nwork work work work\n");
            // вызываем функцию, которая решает поставленную задачу
            countingNumbers(data);
            printf("func countingNumbers end\n");

            if (close(canal[0]) < 0) {
                printf("child: Can't close reading side of pipe\n");
                return -1;
            }

            // записываем во второй канал
            size = write(canal_23[1], data, buffer_size);

            if (size != buffer_size) {
                printf("Can't write all data to pipe\n (size = %d)", size);
                return -1;
            }
            if (close(canal_23[1]) < 0) {
                printf("parent: I Can't close writing side of pipe\n");
                return -1;
            }
        } else { // третий процесс
            if (close(canal_23[1]) < 0) {
                printf("child: I can't close writing side of pipe\n");
                return -1;
            }

            // получаем данные из второго канала
            size = read(canal_23[0], data, buffer_size);

            if (size < 0) {
                printf("Can't read data from pipe\n");
                return -1;
            }
            if (close(canal_23[0]) < 0) {
                printf("child: I can't close reading side of pipe\n");
                return -1;
            }
            int output = open(output_file_title, O_WRONLY | O_CREAT, 0666);

            if (output < 0) {
                printf("Can't %s open file\n", argv[2]);
                return -1;
            }

            // записываем data
            size = write(output, data, strlen(data));
            if (size != strlen(data)) {
                printf("Error! I can't write all data in %s\n", argv[2]);
                return -1;
            }
            if (close(output) < 0) {
                printf("I can't close %s", argv[2]);
            }
        }
    }
    return 0;
}