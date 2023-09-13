#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>

//setting up stop flag
int ut_stp_flg = 0;

//setting up siginthandler
void sigintHandler(int signum) {
    pid_t pid;
    int status;
//iterating over the while loop and checking which processes have terminated
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Process %d with PPID %d has been terminated.\n", pid, getppid());
    }
}
//defining sigstophandler
void sigstopHandler(int signum) {
    ut_stp_flg = 1;
}
//defining sigconthandler
void sigcontHandler(int signum) {
    ut_stp_flg = 0;
}

//defininig int main
int main() {
	//defining child processes
    pid_t ch1, ch2, ch3;
    int cnt = 0;
    //registering the signal handler for sigint
    signal(SIGINT, sigintHandler);
    //registering the signal handler for sigstop
    signal(SIGSTOP, sigstopHandler);
    //registering the signal handler for sigcont
    signal(SIGCONT, sigcontHandler);

    //defining the fork process
    ch1 = fork();
    //defining the logic for child process
    if (ch1 == 0) {
    //setting up gpid
        setpgid(0, 0);
        
        //setting up an infinite loop for the process
        while (1) {
            if (!ut_stp_flg) {
                printf("From process %d every two seconds.\n", getpid());
                sleep(2);
            }
        }
    } else {
        ch2 = fork();
        if (ch2 == 0) {
            setpgid(0, 0);
            prctl(PR_SET_PDEATHSIG, SIGINT);
            while (1) {
                if (!ut_stp_flg) {
                    printf("From process %d every two seconds.\n", getpid());
                    sleep(2);
                }
            }
        } else {
            ch3 = fork();
            if (ch3 == 0) {
                setpgid(0, 0);
                prctl(PR_SET_PDEATHSIG, SIGINT);
                while (1) {
                    if (!ut_stp_flg) {
                        printf("From process %d every two seconds.\n", getpid());
                        sleep(2);
                    }
                }
            } else {
            //setting up an infinite loop
                while (1) {
                //toggling between the flags
                    if (!ut_stp_flg) {
                    //if cnt is divisble by 2, we send, SIGSTOP signal
                        if (cnt % 2 == 0) {
                            printf("This is from process %d.\n", getpid());
                        } else {
                           
                            kill(ch1, SIGSTOP);
                            kill(ch2, SIGSTOP);
                            kill(ch3, SIGSTOP);
                        }
                        cnt++;
                        //making the processes sleep
                        sleep(2);
                        //if cnt is divisble by 2, we send, SIGSTOP signal
                        if (cnt % 2 == 0) {
                            kill(ch1, SIGCONT);
                            kill(ch2, SIGCONT);
                            kill(ch3, SIGCONT);
                        }
                    }
                }
            }
        }
    }

    return 0;
}
