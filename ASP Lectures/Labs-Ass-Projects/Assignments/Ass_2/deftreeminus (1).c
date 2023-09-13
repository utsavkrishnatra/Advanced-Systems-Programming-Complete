//header files declared
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/signal.h>

 
//custom macros declared 

//this is for defining size buffer when reading buffered input
#define MAX_PROC_STAT_LINE_LENGTH 256
//this is for defining the maximum proceses to be entertained while executing the program
#define MAX_PROCESSES 100

 
//this function removes the starting and ending brackets from the given pointer to character array and returns a pointer to character array made on heap
char* removeBrackets(const char* str) {
//setting length of line for tye string to be read
    int len = strlen(str);
    //reserving the memory for the new string without braces
    char* result = malloc((len - 1) * sizeof(char)); // Allocate memory for the new string

 
    //iterating through the function and creating a new not bracketed string
    int j = 0;
    for (int i = 0; i < len; i++) {
        if (str[i] != '(' && str[i] != ')') {
            result[j++] = str[i];
        }
    }
    //terminating the string with the null character
    result[j] = '\0';

 
    //returning the string
    return result;
}

 
 
//this function is a utility function to get all the process ids for a given pid, it takes pointer to an array and returns void
void getProcessIDs(int* processIDs, pid_t pid) {
 // command to be accepted as a character array to be fed to piped open functed
    char command[256];
    //copying the command into command character array using sprintf function
    snprintf(command, sizeof(command), "pstree -p %d | grep -o '([0-9]\\+)' | grep -o '[0-9]\\+'", pid);

 
    // accepting the command as a file pointer
    FILE* commandOutput = popen(command, "r");
    // if file pointer is null, return
    if (commandOutput == NULL) {
        perror("Failed to execute command");
        return;
    }

 
    //creating a new buffered input to accept popen File pointer's content
    char buffer[256];
    //creating a count to iterative over processID array
    int count = 0;
    //iterate over the file pointer fp until it reaches NULL
    while (fgets(buffer, sizeof(buffer), commandOutput) != NULL && count < MAX_PROCESSES) {
    	//each line contains pid of its child in top down fashion, we compute the pids by converting them into integers from strings using atoi function and assign it to processID array line-by-line
        processIDs[count] = atoi(buffer);
        //incrementing the count
        count++;
    }

//closing the file

    pclose(commandOutput);
}

 
//to get status of the file and return it as a character
char getStatus(pid_t pid) {
//to take command as procStatPath
    char procStatPath[64];
    //copying the command into sprintf for further execution
    snprintf(procStatPath, sizeof(procStatPath), "/proc/%d/stat", pid);

 
 //collecting the contents of the the given /proc/pid/stat file using file pointer
 
 //if null then return
    FILE* statFile = fopen(procStatPath, "r");
    if (statFile == NULL) {
        printf("For %d", pid);
        perror("Failed to open proc stat file for");
        return 1;
    }

 
//creating a buffer to collect the contents of statfile
   char statLine[MAX_PROC_STAT_LINE_LENGTH];
    
    //reading first statLine from the stat file and closing it
    if (fgets(statLine, sizeof(statLine), statFile) == NULL) {
        perror("Failed to read proc stat file");
        fclose(statFile);
        return 1;
    }

 
//closing the file
    fclose(statFile);

 
//tokening the statline and collecting the first token
    char* token = strtok(statLine, " ");
    int fieldIndex = 1;

 
//tokeninzing the first line and retrieving the status field, 3rd token is the status filed
    while (token != NULL && fieldIndex < 3) {
        token = strtok(NULL, " ");
        //incrementing the token index until we reach the right index
        fieldIndex++;
    }

 
 
//if token was sussesssfully collected, check for its value.If it turns out to be z, then label it as defunct, otherwise non-defunct,and return the status as well.
    if (token != NULL) {
        char status = (char)*token;
        if (status == 'Z')
            printf("Process status of process %d is defunct: %c\n", pid, status);
        else
            printf("Process status of process %d is non-defunct: %c\n", pid, status);
        return status;
    } else {
        printf("Failed to find the desired field (utime) in proc stat file\n");
    }

 
  // returns null, if nothing is found
    return '\0';
}

 
 
//this utility function us used to get the parent id of the process
pid_t getParentid(pid_t pid) {
//creating the array for executing the command and storing it
    char procStatPath[64];
    snprintf(procStatPath, sizeof(procStatPath), "/proc/%d/stat", pid);

 
 //collecing the file pointer
    FILE* statFile = fopen(procStatPath, "r");
    if (statFile == NULL) {
        printf("For %d", pid);
        perror("Failed to open proc stat file for");
        return 1;
    }

 
//array for collecting the contents of the executed commands
    char statLine[MAX_PROC_STAT_LINE_LENGTH];
    if (fgets(statLine, sizeof(statLine), statFile) == NULL) {
        perror("Failed to read proc stat file");
        fclose(statFile);
        return 1;
    }

 
//closing tht file
    fclose(statFile);

 
//collecting the token and skipping the first token
    char* token = strtok(statLine, " ");
    int fieldIndex = 1;

 
//collecting the 4th index as token for parent id
    while (token != NULL && fieldIndex < 4) {
        token = strtok(NULL, " ");
        fieldIndex++;
    }

 
//if toekn is null, return else return the ppid
    if (token != NULL) {
        pid_t ppid = (pid_t)atoi(token);
        return ppid;
    } else {
        printf("Failed to find the desired field (utime) in proc stat file\n");
    }

 

    return -1;
}

 
//get status and collect ppid,status and proc_name suing this utility funcion
void getStat(pid_t pid, pid_t* ppid, char* status, char** proc_name) {
    char procStatPath[64];
    snprintf(procStatPath, sizeof(procStatPath), "/proc/%d/stat", pid);

 

    FILE* statFile = fopen(procStatPath, "r");
    if (statFile == NULL) {
        printf("For %d", pid);
        perror("Failed to open proc stat file");
        return;
    }

 

    char statLine[MAX_PROC_STAT_LINE_LENGTH];
    if (fgets(statLine, sizeof(statLine), statFile) == NULL) {
        perror("Failed to read proc stat file");
        fclose(statFile);
        return;
    }

 

    fclose(statFile);

 

    char* token = strtok(statLine, " ");
    int fieldIndex = 1;

 

    while (token != NULL && fieldIndex < 4) {
        token = strtok(NULL, " ");
        if (fieldIndex == 1) {
            *proc_name = removeBrackets(token);
            printf("%s ", *proc_name);
        } else if (fieldIndex == 2) {
            *status = *token;
            printf("%c ", *status);
        } else if (fieldIndex == 3) {
            *ppid = (pid_t)atoi(token);
            printf("%d ", (int)*ppid);
        }
        fieldIndex++;
    }
}

 
//this function retrieves the proc name into t character array denoted by poc_name
void getProcName(pid_t pid, char** proc_name) {
    char procStatPath[64];
    snprintf(procStatPath, sizeof(procStatPath), "/proc/%d/stat", pid);

 

    FILE* statFile = fopen(procStatPath, "r");
    if (statFile == NULL) {
        printf("For %d", pid);
        perror("Failed to open proc stat file");
        return;
    }

 

    char statLine[MAX_PROC_STAT_LINE_LENGTH];
    if (fgets(statLine, sizeof(statLine), statFile) == NULL) {
        perror("Failed to read proc stat file");
        fclose(statFile);
        return;
    }

 
// closes the file
    fclose(statFile);

 
//collects the token and skips the proc_id
    char* token = strtok(statLine, " ");
    int fieldIndex = 1;

 
//go the the right field and collect it, the proc_name
    while (token != NULL && fieldIndex <= 1) {
        token = strtok(NULL, " ");
        if (fieldIndex == 1) {
            *proc_name = removeBrackets(token);
            printf("%s ", *proc_name);
        }
        fieldIndex++;
    }
}

 
//this is the driver function that dtermines which parent to delete for a defunct process
void deftreeminusDefault(int pid, int parent_to_ignore) {
//check wheter the given process is defunct or not before deleting it
    if (getStatus(pid) == 'Z') {
     // get the parent of the given process
        pid = getParentid(pid);
        //get parent's name of the given process
        char* proc_name = NULL;
        getProcName(pid, &proc_name);
        //keep moving up the process tree until it reaches the bash process
        while (strcmp(proc_name, "bash") != 0) {
        //if process is there to ignore, then ignore it, if not it will be mentioned as -1
            if (parent_to_ignore != -1 && parent_to_ignore == pid) {
                printf("\n leaving parent process with pid:%d \n", parent_to_ignore);
            } else {
            //system call to kill the process
                kill(pid, SIG_IGN);
                //print th killed process
                printf("\n parent process with pid:%d killed \n", pid);
            }
            //condition to repeatedly iterate up the chain
            pid = getParentid(pid);
            getProcName(pid, &proc_name);
        }
    }
}

 
//creating the main function
int main(int argc, char* argv[]) {

//if number of argument count falls beyond it, return the function
    if (argc > 5 || argc < 2) {
        printf("Provide a valid number of arguments (1 to 5 max)");
        exit(1);
    }

 
   //conveert string input to pid_t input type
    pid_t root = (pid_t)atoi(argv[1]);
    //stores the process id that has to be ignored
    pid_t parent_to_ignore;
    //creates and array that stores the processIDs of all processes running or defunct
    pid_t processIDs[MAX_PROCESSES] = {0};
    //stores the amount of time elapsed since the process's creation
    int maxElapsedTime;
    //stores count of number of defunct children in the array
    int maxDefunctChildren;

 
 // if agrvs are less then 1 return
    if (root <= 1) {
        printf("Root process PID is Invalid\n");
        return 1;
    } else if (argc == 2) {  //if equals 2, kill  parents until it reaches bash
        deftreeminusDefault(root, -1);
    } else if (argc == 3 && atoi(argv[2]) < 0) { // kill  parents until it reaches bash ignoring those mentioned int the list
        printf("parent to ignore:%d\n", atoi(argv[2]));
        deftreeminusDefault(root, -atoi(argv[2]));
    } else {
    //else option given is invalid
        printf("Option is Invalid\n");
        return 1;
    }
    return 0;
}