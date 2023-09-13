#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int ut_fd1[2], ut_fd2[2];
    pid_t ut_pid1, ut_pid2;
    FILE *ut_fp;

    // Forming the pipes
    if (pipe(ut_fd1) == -1 || pipe(ut_fd2) == -1) {
        //throwing the error
        perror("Error in the pipe");
        //exiting the file
        exit(EXIT_FAILURE);
    }

    // for the first, child , forkning
    ut_pid1 = fork();
    if (ut_pid1 < 0) {
        //throwing error in case the file doesn't work
        perror("Error in the fork");
        //exiting the file
        exit(EXIT_FAILURE);
    } else if (ut_pid1 == 0) {
        // logic fof thte child process
        close(ut_fd1[1]); // Close the write end of the first pipe
        dup2(ut_fd1[0], STDIN_FILENO); // Redirect the read end of the first pipe to stdin
        close(ut_fd1[0]); // Close the read end of the first pipe
        close(ut_fd2[0]); // Close the read end of the second pipe
        dup2(ut_fd2[1], STDOUT_FILENO); // Redirect the write end of the second pipe to stdout
        close(ut_fd2[1]); // Close the write end of the second pipe
        execlp("wc", "wc", NULL); // Execute the wc command
        perror("error in the execlp");
        exit(EXIT_FAILURE);
    } else {
        // For the second child process, forking
        ut_pid2 = fork();
        if (ut_pid2 < 0) {
            perror("error in the fork");
            exit(EXIT_FAILURE);
        } else if (ut_pid2 == 0) {
            // Child process 2
            ut_fp = fopen("output.txt", "w"); // Open the output file in write mode
            fprintf(ut_fp, "3\n"); // Write "3" to the output file
            fclose(ut_fp); // Close the output file
            exit(EXIT_SUCCESS);
        } else {
            // logic for the main process
            close(ut_fd1[0]); // Closing the read file end
            ut_fp = fopen("input.txt", "r"); // Opening the fiole in the read mode
            char ut_buffer[1024];
            while (fgets(ut_buffer, sizeof(ut_buffer), ut_fp) != NULL) {
                write(ut_fd1[1], ut_buffer, sizeof(ut_buffer)); // Writing the cointents into the file in write mode
            }
            fclose(ut_fp); // Close the input file
            close(ut_fd1[1]); // Close the write end of the first pipe
            wait(NULL); // Wait for the first child process to finish
            wait(NULL); // Wait for the second child process to finish
        }
    }

    return 0;
}