char LOGFILE[250] =  "/mnt/ramdisk/log/units-bcm2711.log";
#define REVISION "13RC2"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <pthread.h>
#include <pigpiod_if2.h>
#include <signal.h>
#include <math.h>
#include "config.h"
#include "bwlog.h"
#include "tables.h"
#include "prefix.h"
#include "update.h"
#include "pid.h"
#include "invert.h"
#include "gpio.h"

static void hdl (int sig, siginfo_t *siginfo, void *context);
//Signal handler for SIGTERM, SIGINT, SIGUSR1
static void hdl (int sig, siginfo_t *siginfo, void *context){
 if(siginfo->si_signo == SIGABRT){
  bwlog("prob some glibc trouble! Exiting");
  exit(EXIT_FAILURE);
 }

 if(siginfo->si_signo == SIGQUIT){
  exit(EXIT_FAILURE);
 }

 if(siginfo->si_signo == SIGUSR1){
  update();
  return;
 }

 if(siginfo->si_signo == SIGUSR2){
  updateOutput();
  return;
 }

 if(siginfo->si_signo == SIGSEGV){
  bwlog("Segmentaion fault! Exiting");
  exit(EXIT_FAILURE);
 }

 bwlog("Service was killed by UID: %ld", (long)siginfo->si_uid);
 exit(EXIT_FAILURE);
}

int main(){
 //check if another "units" is already running
 if(getpid() != proc_find("units-bcm2711")){
  printf("Error, another instance of units-bcm2711 is already running, exiting! \n");
  exit(EXIT_FAILURE);
 }

 struct sigaction act;
 memset (&act, '\0', sizeof(act));
 /* Use the sa_sigaction field because the handles has two additional parameters */
 act.sa_sigaction = &hdl;
 /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
 act.sa_flags = SA_SIGINFO;

 // Bind to SIGINT (ctrl+c)
 sigaction(SIGINT, &act, NULL);
 // Bind to SIGTERM (reboot)
 sigaction(SIGTERM, &act, NULL);
 // Bind to SIGUSR1
 sigaction(SIGUSR1, &act, NULL);
 // Bind to SIGUSR2
 sigaction(SIGUSR2, &act, NULL);
 // Bind to SIGSEGV (segmentation failt)
 sigaction(SIGSEGV, &act, NULL);
 // Bind to SIGABRT
 sigaction(SIGABRT, &act, NULL);
 // Bind to SIGQUIT
 sigaction(SIGINT, &act, NULL);

 loadConfiguration();

 bwlog("Using table prefix: %s", config.prefix);
 config.pi = pigpio_start(config.pigpiodHost, config.pigpiodPort);
 createTables();
 bwlog("Delay between automatic(non-interrupt) update: %d Sec", config.delay);
 bwlog("gpio SQL Daemon v%s started successfully", REVISION);

 initGpio();
 update();

 while(1){
 sleep(config.delay);
 update();
 updateOutput();
 }
}
