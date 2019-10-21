#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>

pid_t pid1, pid2;
const int sleep_time = 5;

int main() {
    int fd[2];
    char out_pipe[100], in_pipe[100];

    pipe(fd);
    while ((pid1 = fork()) == -1);

    if (pid1 > 0) {
        while ((pid2 = fork()) == -1);

        if (pid2 > 0) {
            wait(0);
            read(fd[0], in_pipe, 50);
            printf("Received: %s\n", in_pipe);

            wait(0);
			read(fd[0], in_pipe, 50);
			printf("Received: %s\n", in_pipe);

			printf("Parent process exit!\n");
			exit(0);
        } else {
            lockf(fd[1], F_LOCK, 0);
			sprintf(out_pipe, "Child process 2 is sending message!\n");
			write(fd[1], out_pipe, 50);
			sleep(sleep_time);
			lockf(fd[1], F_ULOCK, 0);

			printf("Child process 2 exit!\n");
			exit(0);
        }
    } else {
        lockf(fd[1], F_LOCK, 0);
		sprintf(out_pipe, "Child process 1 is sending message!\n");
		write(fd[1], out_pipe, 50);
		sleep(sleep_time);
		lockf(fd[1], F_ULOCK, 0);
        
        printf("Child process 1 exit!\n");
		exit(0);
    }
}