#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>

int wait_flag = 0;

void stop() {
    wait_flag = 0;
}

int main() {
    pid_t pid1, pid2;

    // 监听键盘输入"quit"键
    signal(3, stop);

    while ((pid1 = fork()) == -1);

    if (pid1 > 0) {
        while((pid2 = fork()) == -1);

        if (pid2 > 0) {
            wait_flag = 1;
            sleep(5);
            kill(pid1, 16);
            kill(pid2, 17);
            wait(0);
            wait(0);
            printf("\nParent process is killed!!\n");
            exit(0);
        } else {
            wait_flag = 1;
            signal(17, stop);
            while (wait_flag);
            printf("\nChild process 2 is killed by parent!!\n");
            exit(0);
        }
    } else {
        wait_flag = 1;
        signal(16, stop);
        while (wait_flag);
        printf("\nChild process 1 is killed by parent!!\n");
        exit(0);
    }

    return 0;
}