#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include<string.h>

int main() {

    // Create new.txt with permission 0777
    umask(0);  // Set umask to 0
    int fd = open("new.txt", 0777);
    
    if (fd == -1) {
        printf("Error while opening the file.\n");
        return 1;
    }
    // Writing the message into the file
    const char *msg1 = "Welcome to COMP 8567\n";
    write(fd, msg1, strlen(msg1));


    // Closing the file
    close(fd);

    // Opening new.txt in append only mode
    fd = open("new.txt", O_APPEND);
    if (fd == -1) {
        printf("Error\n");
        return 1;
    }

    // Write into the file in append mode
    const char *msg2 = "Advanced Systems Programming\n";
    write(fd, msg2, strlen(msg2));

    // Close the file
    close(fd);


    // Open new.txt in read write mode
    fd = open("new.txt", O_RDWR);
    if (fd == -1) {
        printf("Error\n");
        return 1;
    }

    // Write "*****" at  into new.txt
    lseek(fd, 12, SEEK_SET);
    write(fd, "*****", 5);
    lseek(fd, 8, SEEK_CUR);
    write(fd, "*****", 5);
    lseek(fd, 31, SEEK_END);
    write(fd, "*****", 5);


    // Replacing the  NULL chars with '#' character.
    char buf[1];
    while (read(fd, buf, 1) == 1) {
        if (buf[0] == '\0') {
            lseek(fd, -1, SEEK_CUR);
            write(fd, "#", 1);
        }
    }
    // Closing the file after performing operations.
    close(fd);
    // Open new.txt in read-only mode
    fd = open("new.txt", O_RDONLY);
    if (fd == -1) {
        printf("Error\n");
        return 1;
    }

    // Printing  contents of  file  with the limit of 13 bytes at a time (line by line)
    char ln[14];
    ssize_t br;
    while ((br = read(fd, ln, 13)) > 0) {
        ln[br] = '\0';
        printf("%s\n", ln);
    }

    // Printing the size of given file
    struct stat status;
    fstat(fd, &status);
    printf("Size of the file in bytes: %ld bytes\n", status.st_size);

    // Close the file
    close(fd);

    return 0;
}
