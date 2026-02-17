#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "pid.h"

int main(){
 pid_t pid_p = proc_find("units-bcm2711");
 if(pid_p==-1){
  printf ("units-bcm2711-service is not running. Exiting!\n");
  exit(EXIT_FAILURE);
 }else{
  printf ("update requested\n");
  kill(pid_p, SIGUSR1);
  exit(EXIT_SUCCESS);
 }
}
