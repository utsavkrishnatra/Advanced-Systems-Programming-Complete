#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



int main() {
    //declaring child1 and its granchild 
    int child1, grandChild;

    // Forking the  child1
    child1 = fork();

    if (child1 < 0) {
        perror("failed to fork the process");
        exit(1);
    } else if (child1 == 0) {
        //  Child 1
        // Forking Grandchild (grandChild)
        grandChild = fork();

        if (grandChild < 0) {
            perror("failed to fork the process");
            exit(1);
        } else if (grandChild == 0) {
            //  Grandchild 
            chdir("/home/krishnau/lab4");  // Changing working directory
            umask(0);

            FILE* file = fopen("sample.txt", "w");
            if (file == NULL) {
                perror("Error opening file");
                exit(1);
            }

            // Set file permission to 0777
            if (chmod("sample.txt", S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
                perror("Error encountered while setting the permissions");
                exit(1);
            }

            // Writing the content to the file
            const char* content = "This is a sample file.\n";
            if (fputs(content, file) == EOF) {
                perror("Error while writing the file");
                exit(1);
            }

            fclose(file);

            exit(0);  // Grandchild process exits normally
        } else {
            // This is Child 1
            int status;
            waitpid(grandChild, &status, 0);  // Wait for Grandchild to complete

            if (WIFEXITED(status)) {
                printf("Child 1 (child1) - Contents of /home/krishnau/lab4:\n");
                char* args[] = {"ls", "-1", "/home/krishnau/lab4", NULL};
                execv("/bin/ls", args);  // Execute ls command
            } else {
                printf("Grandchild process (grandChild) exited abnormally.\n");
            }

            exit(0);
        }
    } else {
        // main process
        // Fork Child 2
        int child2 = fork();

        if (child2 < 0) {
            perror("failed to fork the process");
            exit(1);
        } else if (child2 == 0) {
            // Child 2 
            execl("/bin/ls", "ls", "-1", "/home/krishnau/", NULL);  // Execute ls command
            exit(0);
        } else {
            // Back in main process
            int status;
            waitpid(child1, &status, 0);  // Waiting for Child 1  to complete
            waitpid(child2, &status, 0);  // Waiting for Child 2  to complete
        }
    }

    return 0;
}
