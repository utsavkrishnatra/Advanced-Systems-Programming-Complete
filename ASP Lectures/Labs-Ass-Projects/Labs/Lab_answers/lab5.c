#include <signal.h>
#include <time.h>
#include <stdio.h>


//Declare the variables 
//this signifies the control  C count
 int ccnt = 0;
//this variable signifies the last control c time
 time_t lct = 0;

// This is the custom C handler defined in the program for signal function
void HandlerForSIGINT(int signum) {
    //setting ct(current time) with null as a parameter 
    time_t ct = time(NULL);
    //checking if the time elapsed is less than 5 seconds or more
    if (ct - lct > 5) {
       // else you can reset the count to 1
        ccnt = 1;
    } else {

        //increase the count before checking the count number
        ccnt++;
    if (ccnt >= 2) {
        //if the current control c count is more than or equal to 2, terminate
        printf("Terminating the current program\n\n");
        //this is for normal exit
        exit(0);
    }
    }
    //reset the count to the current time for next prompt from the user
    lct = ct;
}
//defining the main method
int main() {
    // Registering user-defined signal handler for SIGINT
    signal(SIGINT, HandlerForSIGINT);

	//this runs an infinite loop
    while (1) {
        //this will print the line "Welcome to Labs-Signals"
        printf("Welcome to Labs-Signals\n");
        //sleep for 1 seconds
        sleep(1);
    }
    

    //this is considered the normal exit
    return 0;
}
