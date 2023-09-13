/*
Utsav Krishnatra
110095341
*/

//header files
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// counter variable
int utcnt = 6;
// declaration of lock variable
pthread_mutex_t utkcol;
// varible for get detatch state
int utdetachstat;
// attribution variable declaration 
pthread_attr_t utrtta;
// pthread declaration varibale
pthread_t utthread_id;

void *utfunc(void *utarg)
{
  // deattach state for thread
  pthread_attr_getdetachstate(&utrtta, &utdetachstat);
  if (utdetachstat == PTHREAD_CREATE_DETACHED)
  {
    // self thread id
    pthread_t uttid = pthread_self();
    // lock enagage
    pthread_mutex_lock(&utkcol);
    // decreaement counter
    utcnt--;
    // lock disengage
    pthread_mutex_unlock(&utkcol);

    printf("Here Thread Id: %ld, counter: %d\n", uttid, utcnt);
  }
}

int main(int argc, char const *argv[])
{
  // mutex initialize
  if (pthread_mutex_init(&utkcol, NULL) != 0)
  {
    // if any error exist
    perror("\n error mutex init...\n");
    // exit 
    exit(EXIT_FAILURE);
  }

  // iteration utIndex
  int utIndex = 0;

  // initialize attribute
  pthread_attr_init(&utrtta);
  // while loop
  while (utIndex < 6)
  {
    // if the index is even
    if (utIndex % 2 == 0)
    {
      // Do thread joinable
      // set joinable thread
      pthread_attr_setdetachstate(&utrtta, PTHREAD_CREATE_JOINABLE);
    }
    else
    {
      // thread in deattached state
      // set detatch state for the attribute
      pthread_attr_setdetachstate(&utrtta, PTHREAD_CREATE_DETACHED);
    }

    // thread create with specified attr
    pthread_create(&utthread_id, &utrtta, utfunc, NULL);

    // waiting for thread termination
    pthread_join(utthread_id, NULL);

    // index increamental
    utIndex++;
  }

  // destryoing the thread
  pthread_attr_destroy(&utrtta);

  // mutex destroy
  pthread_mutex_destroy(&utkcol);

  // success with exit
  exit(EXIT_SUCCESS);
}
