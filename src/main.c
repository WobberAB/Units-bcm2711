#define REVISION "13RC6"
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
#include "tables.h"
#include "prefix.h"
#include "update.h"
#include "pid.h"
#include "invert.h"
#include "gpio.h"

int main(){
 //check if another "units" is already running
 if(getpid() != proc_find("units-bcm2711")){
  printf("Error, another instance of units-bcm2711 is already running, exiting! \n");
  exit(EXIT_FAILURE);
 }

 if (createPrefix()){
  fprintf(stderr, "Failed to create prefix\n");
  return EXIT_FAILURE;
 }

 if (loadConfiguration()){
  fprintf(stderr, "Failed to load configuration\n");
  return EXIT_FAILURE;
 }

 printf("Using table prefix: %s", config.prefix);
 config.pi = pigpio_start(config.pigpiodHost, config.pigpiodPort);
 createTables();
 printf("Delay between automatic(non-interrupt) update: %d Sec", config.delay);
 printf("gpio SQL Daemon v%s started successfully", REVISION);

 initGpio();
 update();

 while(1){
 sleep(config.delay);
 update();
 updateOutput();
 }
}
