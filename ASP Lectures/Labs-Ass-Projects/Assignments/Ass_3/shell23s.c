// Utsav Krishnatra 110095341 Section 2

//I have inlcuded the required libraries
#include <stdlib.h>         // Standard library functions
#include <string.h>         // String handling functions
#include <unistd.h>         // POSIX operating system API
#include <sys/wait.h>       // Process management functions
#include <stdbool.h>        // Boolean data type and true/false macros
#include <wordexp.h>        // Word expansion (used for utcmd line expansion)
#include <fcntl.h>          // File control options
#include <glob.h>           // Pattern matching with wildcards
#include <sys/types.h>      // Basic system data types
#include <stdio.h>          // Standard feed/Output functions



//defining the maximum cammands to 6 as per the question
#define COMMAND_LIMIT 6
//defining the maximum arguments to 6 as per the requirement of the question
#define ARG_LIMIT 6
//defining the max buffer size to 1MB as per the requirement of the question
#define MAX_BUF_SZ 1024


//this function parses the given utcmd and stores it in the argument variable on the basis of the delimited provided
void parser(char *utcmd, char *args[ARG_LIMIT], const char *utdelimeter) {
    //it tokenizes the given utcmd on the basis of utdelimeter and stores in a pointer to character array
    char *ut_token = strtok(utcmd, utdelimeter);
    //it keeps count of number of arguments in the code
    int utarg_index = 0;
    //this function keeps generating the utcmd until the end of the string as per the utdelimeter and also checks that it doesn't
    //exceeds the ARG_LIMIT limit
    while (ut_token != NULL && utarg_index < ARG_LIMIT - 1) {
        //it stores the in the argument the utcmd produced as ut_token
        args[utarg_index++] = ut_token;
        //it keeps tokenizing the  utcmd string until it no more tokens can be generated
        ut_token = strtok(NULL, utdelimeter);
    }
    // it marks the end of the character array using NULL keyword
    args[utarg_index] = NULL;
}

//this function is used to execute the utcmd  with proper piping as well as check if the given process is a background process or not
int executor(char *args[ARG_LIMIT], int utinpt_fd, int utout_pd, int ut_isbackgrnd) {
    //forking the process 
    pid_t utpid = fork();

    // if the process is not forked successfully, it will write the error on STANDARD OUTPUT and exit
    if (utpid < 0) {
        perror("failed fork");
        exit(1);
    } else if (utpid == 0) {
        // checking for the Child process
       
        //if the given output file descriptor is not a the last file descriptor, it will keep re-directing it for piping operations
        if (utout_pd != STDOUT_FILENO) {
            //redirect feed of the child to utout_pd
            dup2(utout_pd, STDOUT_FILENO);
            //clos the ut_outfile descriptor
            close(utout_pd);
        }
         //if the given feed file descriptor is not a the last file descriptor, it will keep re-directing it for piping operations
        if (utinpt_fd != STDIN_FILENO) {
            //redirect the feed of the child to utinpt_fd
            dup2(utinpt_fd, STDIN_FILENO);
            //close the rrinpfile descriptor
            close(utinpt_fd);
        }
        
       //this will execute the utcmd using exec which accepts vector and path arguments
        execvp(args[0], args);
        //in case execvp doesn't execute, print the error and exit abnormally
        perror("failed execvp ");
        exit(1);
    } else {
        // coming to the parent process
        // checking if the given process is a background process or not
        if (!ut_isbackgrnd) {
            //checking the status of the process
            int status;
            //waiting for the child process for the given utpid and retrieving status of the child process after termination
            waitpid(utpid, &status, 0);
            //checking if the child process terminated normally or abnormally
            if (WIFEXITED(status)) {
                // if it exited normally, it will return the status of the child process
                return WEXITSTATUS(status);  // Return the exit status of the child process
            }
        } else {
            // in case process in a background process, print the utcmd and the child utpid
            printf("exec of backgrounf '%s' with utpid %d.\n", args[0], utpid);
        }    
    }
    // normal course of execution
    return 0;  
}

// this will execute the commands as pipes
void execute_pipeline(char *commands[COMMAND_LIMIT], int utnum_cmds) {

    int i;
    //pipes for communication between two commands
    //previous pipe stores the file_desciptors of the last utcmd executed
    int ut_pre_pipe[2];
    //current pipe stores the file_descriptor of the current utcmd executed
    int curr_pipe[2];
    //stores the feed file descriptor as standard output file number 0
    int utinpt_fd = STDIN_FILENO;

    //this will run all commands execpt the last coammand as the last utcmd in the utcmd sequence 
    //doesn't require to be written into a pipe and has to be displayed on the stdout
    for (i = 0; i < utnum_cmds - 1; i++) {
     // creates a pipe for to be shared with the child, in case of failure pipe the code will register an abnormal exit   
        if (pipe(curr_pipe) < 0) {
            perror("pipe failed");
            exit(1);
        }
     //args utcmd will write the take the parsed utcmd at the ith position dilimited by space in the utcmd array
        char *args[ARG_LIMIT];
        //parses the utcmd and stores in the argument array
        parser(commands[i], args, " ");
        //executes the utcmd using  for loop creating parent and child processes and piping them using current and previous
        //file descriptors
        executor(args, utinpt_fd, curr_pipe[1], 0);
        //closes the write file descriptor of the current file after the utcmd execution
        close(curr_pipe[1]);
        //sets the current pipe as feed file descriptor to be used as a parameter for the next piping operation for the following utcmd
        utinpt_fd = curr_pipe[0];
        //sets the current pipes into the previous pipes for feed 
        ut_pre_pipe[0] = curr_pipe[0];
         //sets the current pipes into the previous pipes for output 
        ut_pre_pipe[1] = curr_pipe[1];
    }

    // Last utcmd in the pipeline
    char *args[ARG_LIMIT];
    //parse utcmd used to perform parsing for the last utcmd in the sequence
    parser(commands[i], args, " ");
    //executes the last utcmd and prints output in the standard output
    executor(args, utinpt_fd, STDOUT_FILENO, 0);
    //choles the feed file descriptor
    if (utinpt_fd != STDIN_FILENO) {
        close(utinpt_fd);
    }
    //closes the previous file desciptors waits for all the process to finish
    for (i = 0; i < utnum_cmds - 1; i++) {
        close(ut_pre_pipe[0]);
        close(ut_pre_pipe[1]);
        wait(NULL);
    }
}

//this utcmd will handle the > redirection
void output_redirection_handler(char *args[ARG_LIMIT], char *ut_outfile) {
    // Parse the utcmd and arguments
    parser(args[0], args, " ");
    //opens the file in truncate and read write mode, in case the file is not present it will be created
    int utout_pd = open(ut_outfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    // in case of failure, error will be printed
    if (utout_pd == -1) {
        perror("output redirection failed");
        exit(1);
    }
    //executes the utcmd while passing the arguments
    executor(args, STDIN_FILENO, utout_pd, 0);
    //closing the utcmd post use
    close(utout_pd);
}

void output_append_handler(char *args[ARG_LIMIT], char *ut_outfile) {
    // Parse the commands and arguments
    parser(args[0], args, " ");
    //opens the file int append mode
    int utout_pd = open(ut_outfile, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    //in case of failure, error is printed
    if (utout_pd == -1) {
        perror("failed out redirection");
        exit(1);
    }
    //executes the utcmd and takes the output file descriptor in order to write into the file
    executor(args, STDIN_FILENO, utout_pd, 0);
    //closes the file post use
    close(utout_pd);
}

//this utcmd will handle feed redirection
void input_redirection_handler(char *args[ARG_LIMIT], char *rrinpfile) {
    //parses the utcmd based on the number of spaces
    parser(args[0], args, " ");
    //opens the file in read only mode
    int utinpt_fd = open(rrinpfile, O_RDONLY);
    //in case of failure prints the message
    if (utinpt_fd == -1) {
        perror("failed feed redirection ");
        exit(1);
    }
    //executes the utcmd and while reading from the file with the given file descriptor
    executor(args, utinpt_fd, STDOUT_FILENO, 0);
    //closes the file posit suse
    close(utinpt_fd);
}

//this is used to handle ; charater and sequential execution done by it
void sequential_redirection_handler(char *args[ARG_LIMIT], int utnum_cmds){
    //for the given number of commands, iterating over the while loop and executing it
    for(int i = 0; i < utnum_cmds; i++){
        //takes the arguments as temp arguments
        char *ut_tempargs[ARG_LIMIT];
        //parses the arguments and fills in the temporary argument array
        parser(args[i], ut_tempargs, " ");
        //executes the utcmd and outputs it to the screen
        executor(ut_tempargs, STDIN_FILENO, STDOUT_FILENO, 0);
    }
}

// Function to handle conditional execution of commands based on utoperators like '&&' and '||'
void handle_conditional_execution(char *args[ARG_LIMIT], char *utoperators[COMMAND_LIMIT], int utnum_cmds, int utnum_operators) {
    bool utprev_cmd_status = true;  // Flag to track the status of the previous utcmd
    
    // Loop through each utcmd to execute
    for (int i = 0; i < utnum_cmds; i++) {
        // Check the operator between the commands
        if (i < utnum_operators) {
            // Skip executing the utcmd if the previous utcmd failed and the operator is "&&"
            if (strcmp(utoperators[i], "&&") == 0 && !utprev_cmd_status) {
                break;
            }
            // Skip executing the utcmd if the previous utcmd succeeded and the operator is "||"
            else if (strcmp(utoperators[i], "||") == 0 && utprev_cmd_status) {
                break;
            }
        }

        // Execute the utcmd
        // Create a separate array to hold the parsed utcmd arguments
        char *utparsed_args[ARG_LIMIT];
        
        // Parse the utcmd into separate arguments using a space as a utdelimeter
        parser(args[i], utparsed_args, " ");
       
        // Execute the parsed utcmd and get the status
        int command_status = executor(utparsed_args, STDIN_FILENO, STDOUT_FILENO, 0);

        // Print the status of the executed utcmd
        printf("stat = %d\n", command_status);

        // Track the status of the executed utcmd
        if (command_status == 0) {
            utprev_cmd_status = true; // utcmd succeeded, update status
        } else if (command_status != 0) {
            utprev_cmd_status = false; // utcmd failed, update status
        }
    }
}

// Function to expand the arguments that start with '~' or contain wildcards using wordexp and glob
void args_expander(char *args[ARG_LIMIT]) {
    for (int i = 0; args[i] != NULL; i++) {
        char *arg = args[i];

        // Check if the argument starts with '~'
        if (arg[0] == '~') {
            // Expand the argument using wordexp
            wordexp_t p;
            if (wordexp(arg, &p, 0) == 0) {
                if (p.we_wordc > 0) {
                    // Replace the argument with the expanded version
                    free(args[i]);
                    args[i] = strdup(p.we_wordv[0]);
                }
                wordfree(&p);
            }
        } else {
            // Expand the argument containing wildcards using glob
            glob_t glob_result;
            if (glob(arg, GLOB_NOCHECK | GLOB_TILDE, NULL, &glob_result) == 0) {
                for (size_t j = 0; j < glob_result.gl_pathc; j++) {
                    // Replace the argument with the expanded version
                    free(args[i]);
                    args[i] = strdup(glob_result.gl_pathv[j]);
                }
            }
            globfree(&glob_result);
        }
    }
}

// Function to split the initial feed into separate commands based on a special character (e.g., ';', '|')
char** command_splitter(char* utinitInput, int* utnum_cmds, char *utspecial_char) {
    char **commands = malloc(COMMAND_LIMIT * sizeof(char *));
    *utnum_cmds = 0;
    int uterror_flag = 0;
    int uttotal_words = 0;

    // Split the feed string into commands using the specified special character as a utdelimeter
    char *coms = strtok(utinitInput, utspecial_char);
    while (coms != NULL) {
        // Trim leading and trailing spaces from each utcmd
        while (*coms == ' ') {
            coms++;
        }
        int len = strlen(coms);
        while (len > 0 && coms[len - 1] == ' ') {
            coms[len - 1] = '\0';
            len--;
        }

        // Check for utcmd length errors
        if (strlen(coms) < 1) {
            printf("error is occuring '%s' is way tooo short. min len allowed is 1 character.\n", coms);
            uterror_flag = 1;
            break;
        }

        // Store the utcmd in the commands array
        commands[(*utnum_cmds)++] = coms;

        // Move to the next utcmd
        coms = strtok(NULL, utspecial_char);
    }

    // Free memory and return NULL if there was an error during utcmd splitting
    if (uterror_flag) {
        for (int i = 0; i < *utnum_cmds; i++) {
            free(commands[i]);
        }
        free(commands);
        return NULL;
    }

    return commands;
}


// Main function, the entry point of the program
int main(int argc, char *argv[]) {
    // Declare variables to be used in the main function
    char feed[MAX_BUF_SZ]; // Buffer to store user feed
    char *utinitInput = NULL; // Pointer to hold a deep copy of the feed
    char *commands[COMMAND_LIMIT]; // Array to store individual commands
    char *utoperators[COMMAND_LIMIT] = {NULL}; // Array to store special characters (utoperators) between commands
    int utnum_cmds = 0; // Counter for the number of commands
    char *utspecial_char[] = {"|", ">", "<", ">>", "&&", "||", "&", ";"}; // Array of special characters (utoperators)
    char *utfoundspecial_char = NULL; // Store the found special character during parsing
    int SpcChrCnt = sizeof(utspecial_char) / sizeof(utspecial_char[0]); // Calculate the number of special characters
    bool utisMultspecialCharPresent = false; // Flag to indicate if multiple special characters are found
    int uttotalSameSpecialCharingCommand = 0; // Counter to track the number of consecutive occurrences of the same special character
    int utcurrArgcnt = 0; // Counter for the number of arguments in the current utcmd
    char *rrinpfile = NULL; // Pointer to hold the feed file for feed redirection
    char *ut_outfile = NULL; // Pointer to hold the output file for output redirection

    // Infinite loop to keep the shell running until the user decides to exit
    while (1) {
        printf("mshell$ "); // Display the shell prompt
        fgets(feed, sizeof(feed), stdin); // Read the user feed
        feed[strlen(feed) - 1] = '\0'; // Remove the newline character at the end of the feed

        // Check if the feed is empty, if yes, ignore and continue to the next iteration of the loop
        if (strlen(feed) == 0) {
            continue;
        }

        // Allocate memory for the deep copy of the feed
        utinitInput = malloc(strlen(feed) + 1);
        strcpy(utinitInput, feed); // Create a deep copy of the feed

        char *ut_token;

        // Split the feed string using space as the utdelimeter
        ut_token = strtok(feed, " ");

        // Loop to tokenize the feed and check for special characters (utoperators)
        while (ut_token != NULL) {
            // Check for any occurrence of special characters in the ut_token
            for (int i = 0; i < SpcChrCnt; i++) {
                if (strcmp(ut_token, utspecial_char[i]) == 0) {
                    // If a special character is found
                    if (utfoundspecial_char == NULL) {
                        utfoundspecial_char = utspecial_char[i]; // Store the found special character
                        uttotalSameSpecialCharingCommand++;
                    } else if (strcmp(utfoundspecial_char, utspecial_char[i]) == 0) {
                        uttotalSameSpecialCharingCommand++;
                        // If there are more than 7 consecutive occurrences of the same special character, raise an error
                        if (uttotalSameSpecialCharingCommand > 7) {
                            utisMultspecialCharPresent = true;
                            printf("Error: More than 7 occurrences of the special character '%s' found.\n",
                                   utspecial_char[i]);
                            utfoundspecial_char = NULL;
                            break;
                        }
                    } else if ((strcmp(utfoundspecial_char, "&&") == 0 && strcmp(utspecial_char[i], "||") == 0) ||
                               (strcmp(utfoundspecial_char, "||") == 0 && strcmp(utspecial_char[i], "&&") == 0)) {
                        uttotalSameSpecialCharingCommand++;
                        // If there are more than 7 consecutive occurrences of different special characters, raise an error
                        if (uttotalSameSpecialCharingCommand > 7) {
                            utisMultspecialCharPresent = true;
                            printf("Error: More than 7 occurrences of the special character '%s' found.\n",
                                   utspecial_char[i]);
                            utfoundspecial_char = NULL;
                            break;
                        }
                    } else {
                        utisMultspecialCharPresent = true;
                        printf("Error: Multiple types of special characters found.\n");
                        utfoundspecial_char = NULL; // Reset the found special character
                        break;
                    }
                }
            }

            ut_token = strtok(NULL, " "); // Move to the next ut_token
        }

        // If a special character is found, execute commands based on the type of special character
        if (utfoundspecial_char != NULL) {
            // Check which special character is found and execute the corresponding utcmd
            if (strcmp(utfoundspecial_char, "|") == 0) {
                // Execute pipe utcmd
                char **commands = command_splitter(utinitInput, &utnum_cmds, "|");
                if (commands != NULL) {
                    execute_pipeline(commands, utnum_cmds);
                }
            } else if (strcmp(utfoundspecial_char, ">") == 0) {
                // Output redirection (truncate)
                char **commands = command_splitter(utinitInput, &utnum_cmds, ">");
                if (commands != NULL) {
                    ut_outfile = commands[1];
                    output_redirection_handler(commands, ut_outfile);
                }
            } else if (strcmp(utfoundspecial_char, ">>") == 0) {
                // Output redirection (append)
                char **commands = command_splitter(utinitInput, &utnum_cmds, ">>");
                if (commands != NULL) {
                    ut_outfile = commands[1];
                    output_append_handler(commands, ut_outfile);
                }
            } else if (strcmp(utfoundspecial_char, "<") == 0) {
                // feed redirection
                char **commands = command_splitter(utinitInput, &utnum_cmds, "<");
                if (commands != NULL) {
                    rrinpfile = commands[1];
                    input_redirection_handler(commands, rrinpfile);
                }
            } else if (strcmp(utfoundspecial_char, "||") == 0 || strcmp(utfoundspecial_char, "&&") == 0) {
                // Conditional execution using "||" and "&&"
                int utnum_cmds = 0;
                int utnum_operators = 0;
                char *coms = strtok(utinitInput, "|&"); // Example feed: "ls -l || date"
                while (coms != NULL) {
                    // Trim leading and trailing spaces from each utcmd
                    while (*coms == ' ') {
                        coms++;
                    }
                    int len = strlen(coms);
                    while (len > 0 && coms[len - 1] == ' ') {
                        coms[len - 1] = '\0';
                        len--;
                    }

                    if (strlen(coms) > 0) {
                        if (strcmp(coms, "&&") == 0 || strcmp(coms, "||") == 0) {
                            // It's an operator, store it in the utoperators array
                            utoperators[utnum_operators++] = coms;
                        } else {
                            // It's a utcmd, store it in the commands array
                            commands[utnum_cmds++] = coms;
                        }
                    }

                    coms = strtok(NULL, "|&"); // Move to the next utcmd or operator
                }

                handle_conditional_execution(commands, utoperators, utnum_cmds, utnum_operators);

            } else if (strcmp(utfoundspecial_char, "&") == 0) {
                // Background execution using "&"
                char **commands = command_splitter(utinitInput, &utnum_cmds, "&");
                if (commands != NULL) {
                    parser(commands[0], commands, " ");
                    executor(commands, STDIN_FILENO, STDOUT_FILENO, 1);
                }
            } else if (strcmp(utfoundspecial_char, ";") == 0) {
                // Sequential execution using ";"
                char **commands = command_splitter(utinitInput, &utnum_cmds, ";");
                if (commands != NULL) {
                    sequential_redirection_handler(commands, utnum_cmds);
                }
            } else {
                printf(" special character is invalid here '%s'.\n", utfoundspecial_char);
            }
        } else if (!utisMultspecialCharPresent) {
            // No special characters found, execute regular utcmd
            char *ut_token = strtok(utinitInput, " ");
            while (ut_token != NULL) {
                // Check if the maximum number of commands has been exceeded, print an error message if true
                if (utnum_cmds >= COMMAND_LIMIT) {
                    printf("max commands exceeded.\n");
                    break;
                }

                // Expand the tilde (~) in the argument using wordexp if present
                if (ut_token[0] == '~') {
                    wordexp_t p;
                    if (wordexp(ut_token, &p, 0) == 0) {
                        if (p.we_wordc > 0) {
                            commands[utnum_cmds++] = strdup(p.we_wordv[0]); // Store the expanded argument
                        }
                        wordfree(&p);
                    }
                } else {
                    commands[utnum_cmds++] = strdup(ut_token); // Store the argument as is
                }

                ut_token = strtok(NULL, " "); // Move to the next ut_token
            }

            if (utnum_cmds > 0 && utnum_cmds < COMMAND_LIMIT) {
                commands[utnum_cmds] = NULL; // Add NULL as the last element to mark the end of the array

                executor(commands, STDIN_FILENO, STDOUT_FILENO, 0); // Execute the utcmd with feed and output redirection if any
            } else {
                printf("valid commands not  provided.\n");
            }
        }

        // Reset variables for the next iteration of the loop
        uttotalSameSpecialCharingCommand = 0;
        utnum_cmds = 0;
        utfoundspecial_char = NULL;
        utisMultspecialCharPresent = false;

        free(utinitInput); // Free the memory allocated for the deep copy of the feed
    }

    return 0;
}
