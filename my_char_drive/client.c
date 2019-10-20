#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

int fd;
char buffer[1024];
char name[100];
char line[1024];
char input[1000];

void send(const char* str) {
	sprintf(line, "%s: %s", name, str);
	write(fd, line, strlen(line) + 1);
}

void chat() {
	fgets(input, 900, stdin);
	send("One man enters");
	while (1) {
		fgets(input, 900, stdin);
		input[strcspn(input, "\n")] = '\0';

		if (strcmp(input, "/quit") == 0) {
			send("One man exits");
			close(fd);
			exit(0);
		}
		send(input);
	}
}

void receive() {
	read(fd, buffer, sizeof(buffer));
	while (1) {
		read(fd, line, sizeof(line));
		printf("%s\n", line);
	}
}


int main() {
	fd = open("/dev/chardev0", O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Open error\n");
		return -1;
	}

	pthread_t receive_thread;
	
	printf("Please input your name:");
	scanf("%s", name);
	pthread_create(&receive_thread, NULL, (void*)(&receive), NULL);

	chat();
	return 0;
}