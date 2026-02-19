#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <pigpiod_if2.h>
#include <math.h>
#include <ctype.h>
#include "config.h"
#include "invert.h"
#include "update.h"
#include "gpio.h"

static int validate_prefix(const char *prefix) {
    if (prefix == NULL || prefix[0] == '\0') return 0;
    for (const char *p = prefix; *p; p++) {
        if (!islower((unsigned char)*p)) {
            printf("SECURITY: Invalid character in prefix: '%c' (0x%02x)\n", *p, (unsigned char)*p);
            return 0;
        }
    }
    return 1;
}

void initGpio(){
 /* Validate prefix before using in SQL queries */
 if (!validate_prefix(config.prefix)) {
  printf("initGpio: Invalid prefix detected, aborting\n");
  return;
 }
 
 MYSQL *con = mysql_init(NULL);
 if (con == NULL){
 printf("initgpioChipSettings: %s", mysql_error(con));
 exit(EXIT_FAILURE);
 }

 if (mysql_real_connect(con, config.host, config.user, config.pass,
  config.database, 0, NULL, 0) == NULL){
  printf("initgpioChipSettings: %s", mysql_error(con));
  exit(EXIT_FAILURE);
 }

 char query[500] = {0};
 snprintf(query, sizeof query, "SELECT `address`, `pin`, `direction`, `pullup`, `interrupt`, `inverted`, `enabled`, `glitch` FROM `gpio-conf`");

 if (mysql_query(con, query)){
  printf("initgpioChipSettings: %s", mysql_error(con));
  exit(EXIT_FAILURE);
 }

 MYSQL_RES *result = mysql_store_result(con);
 if (mysql_num_rows(result)>0)

 {
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result))){
   int pin = atoi(row[1]);
   int direction = atoi(row[2]);
   int pullup = atoi(row[3]);
   int interrupt = atoi(row[4]);
   int enabled = atoi(row[6]);
   int glitch = atoi(row[7]);

   //IF PIN IS ENABLED
   if(enabled==1){
    switch(direction){
    case 0:
     //IF OUTPUT
     set_mode(config.pi, pin, PI_OUTPUT);	// set pin as output
     printf("CREATING OUTPUT: Pin %d", pin);
    break;

    case 1:
     //IF INPUT
     printf("CREATING INPUT: Pin %d, Pullup=%d, Interrupt=%d", pin, pullup, interrupt);
     set_mode(config.pi, pin, PI_INPUT);
     //SET PULL UP/DOWN/OFF
     switch(pullup){
      case 0:
       set_pull_up_down(config.pi, pin, PI_PUD_OFF);
      break;

      case 1:
       set_pull_up_down(config.pi, pin, PI_PUD_DOWN);
      break;

      case 2:
       set_pull_up_down(config.pi, pin, PI_PUD_UP);
      break;

      default:
       set_pull_up_down(config.pi, pin, PI_PUD_OFF);
       printf("Setting inputpin %d default PI_PUD_OFF", pin);
      break;
     }

     //SET GLITCH FILTER
     if (glitch != ERRNO){
      set_glitch_filter(config.pi, pin, glitch);
     }

     //SET INTERRUPT
     switch(interrupt){
      case 0: //DISABLED
      break;
      case 1: //FALLING EDGE
       callback(config.pi, pin, 0, update);
      break;
      case 2: //RISING EDGE
       callback(config.pi, pin, 1, update);
      break;
      case 4: //EITHER EDGE
       callback(config.pi, pin, 2, update);
      break;
      default:
      break;
     }
    break;

    case 2:
    //IF PWM OUTPUT
    /* hardwarePWM function takes care of reinitialisation when pin is activated. */
    /* Set as general input only for saftey purpose during bootstrapping */
     set_mode(config.pi, pin, PI_OUTPUT);					// set pin as output
     printf("CREATING PWM: Pin %d", pin);
    break;

    default:
     //IF ERRORNEUS OR NO DIRECTION GIVEN IN CONF
     set_mode(config.pi, pin, PI_INPUT);
     printf("ERROR IN CONF: Pin %d, Default mode %d", pin, get_mode(config.pi, pin));
     set_pull_up_down(config.pi, pin, PI_PUD_OFF);
    break;
    }
   }
  }
 }else{
  printf("Missing configuration for (%s)gpio. (Re)configuring all missing with default settings", config.prefix);
  for (int pin=0; pin<=27; pin++){
   if((pin!=2)&&(pin!=3)){ // skip i2c pins
    memset(&query[0], 0, sizeof(query));
    snprintf(query, sizeof query,"INSERT INTO `gpio-conf` (`address`, `pin`, `direction`, `pullup`, `interrupt`, `inverted`, `glitch`, `enabled`) \
		  VALUES ('%sgpio', %d, 1, 0, 0, 0, -1001, 1)", config.prefix, pin);
    if (mysql_query(con, query)){
     printf("initgpioChipSettings: %s", mysql_error(con));
     exit(EXIT_FAILURE);
    }
   }else{
    memset(&query[0], 0, sizeof(query));
    snprintf(query, sizeof query,"INSERT INTO `gpio-conf` (`address`, `pin`, `direction`, `pullup`, `interrupt`, `inverted`, `glitch`, `enabled`) \
		  VALUES ('%sgpio', %d, 1, 0, 0, 0, -1001, 0)", config.prefix, pin);
    if (mysql_query(con, query)){
     printf("initgpioChipSettings: %s", mysql_error(con));
     exit(EXIT_FAILURE);
    }
   }

  }
 }
 mysql_free_result(result);
 //Save checksum of table
 if (mysql_query(con, "CHECKSUM TABLE `gpio-conf`")){
  printf("initgpioChipSettings: %s", mysql_error(con));
  exit(EXIT_FAILURE);
 }else{
  MYSQL_RES *check = mysql_store_result(con);
  if (mysql_num_rows(check)==1){
   MYSQL_ROW row;
   while ((row = mysql_fetch_row(check))){
    config.checksum = atoll(row[1]);
   }
  }
 mysql_free_result(check);
 }

 mysql_close(con);
 return;
}
