#include <stdlib.h>
#include <unistd.h>

#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>


#include <stdio.h>
#include <string.h>

int main() {

    int n1,n2,n3;
    n1=n2=n3=10;
    int fd = open("sample.txt", O_CREAT | O_RDWR);
    int pd1, pd2;
    
    pd1 = fork();
    if (pd1 == 0) {
        // Child 1 process
        write(fd, "COMP 8567\n", strlen("COMP 8567\n"));
        for (int i = 0; i < 15; i++) {
            printf("Process ID for child 1 is: %d Parent Process ID: %d\n", getpid(), getppid());
            sleep(1);
        }
        n1 += 10;
        n2 += 10;
        n3 += 10;
        printf("Numbers obtained for child 1 is: n1=%d n2=%d n3=%d\n", n1, n2, n3);
        exit(0);
    } else if (pd1 > 0) {
        pd2 = fork();
        if (pd2 == 0) {
            // Child 2 process
            write(fd, "COMP 8567\n", strlen("COMP 8567\n"));
            for (int i = 0; i < 15; i++) {
                printf("Process ID for child 2 is %d PPID: %d\n", getpid(), getppid());
                sleep(1);
            }
            n1 += 10;
            n2 += 10;
            n3 += 10;
            printf("Process ID for child 1 is: n1=%d n2=%d n3=%d\n", n1, n2, n3);
            
            exit(0);
        } else if (pd2 > 0) {
            // Parent process
            int status;
            wait(&status);
            if (WIFEXITED(status)) {
                printf("Child has %d exited normally with the status %d\n", pd1 == 0 ? 1 : 2, WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("Child has %d exited due to the following signal %d\n", pd1 == 0 ? 1 : 2, WTERMSIG(status));
            }
            write(fd, "HELLO! FROM PARENT\n", strlen("HELLO! FROM PARENT\n"));
            close(fd);
            n1 += 25;
            n2 += 25;
            n3 += 25;
            printf("Parent has the following numbers: n1=%d n2=%d n3=%d\n", n1, n2, n3);
        } else {
            exit(1);
        }
    } else {
        exit(1);
    }
    return 0;
}