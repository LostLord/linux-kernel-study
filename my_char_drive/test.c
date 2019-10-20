#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

char line[100] = "aaaaaaaaaabbbbbbbbbbccccccccccdddddddddd";
char out[100] = {"\0"};

int main() {
	int fd, num;
	int wret, rret;
	fd = open("/dev/chardev0", O_RDWR, S_IRUSR | S_IWUSR);

	printf("%d\n", sizeof(int));

	if (fd != -1) {

		// sprintf(line, "%d-%d-%d", 2019, 10, 15);
		wret = write(fd, line, 20);
		printf("write ret = %d\n", wret);

		rret = read(fd, out, 5);
  		printf("read ret = %d\n", rret);
		printf("out ret = %s\n", out);

		rret = read(fd, out, 5);
		printf("read ret = %d\n", rret);
		printf("out ret = %s\n", out);

		rret = read(fd, out, 5);
  		printf("read ret = %d\n", rret);
		printf("out ret = %s\n", out);

		rret = read(fd, out, 5);
		printf("read ret = %d\n", rret);
		printf("out ret = %s\n", out);

		wret = write(fd, line + 10, 20);
		printf("write ret = %d\n", wret);
		wret = write(fd, line + 10, 20);
		printf("write ret = %d\n", wret);

		rret = read(fd, out, 30);
		printf("read ret = %d\n", rret);
		printf("out ret = %s\n", out);

		close(fd);
	} else {
		printf("Open error\n");
	}
	return 0;
}
