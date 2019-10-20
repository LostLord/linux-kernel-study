#include <stdio.h>
#include <unistd.h>

int main() {
    char to1[100] = {"Hello World!"};
    char to2[100] = {'\0'};

    int ret = syscall(333, to1, to2);

    printf("%s\n", to1);
    return 0;
}