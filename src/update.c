#include <mysql/mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <pigpiod_if2.h>
#include <string.h>
#include "config.h"
#include "update.h"
#include "invert.h"
#include "gpio.h"

int gpioPins[28] = {-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001,-1001};	// DEFAULT SET AS UNINITALIZED

void update(){
 MYSQL *con = mysql_init(NULL);
  if (con == NULL){
   printf("update: %s", mysql_error(con));
   exit(EXIT_FAILURE);
  }

 if (mysql_real_connect(con, config.host, config.user, config.pass, config.database, 0, NULL, 0) == NULL){
  printf("update: %s", mysql_error(con));
  exit(EXIT_FAILURE);
 }

 char query[500] = {0};
 for(int x=0;x<=27;x++){
  snprintf(query, sizeof query, "SELECT `inverted`, `enabled`, `direction` FROM `gpio-conf` WHERE `enabled`=1 AND `address` LIKE '%%%sgpio%%' AND `pin` = %d", config.prefix, x);
  if (mysql_query(con, query)){
   printf("update: %s", mysql_error(con));
   exit(EXIT_FAILURE);
  }
  MYSQL_RES *result = mysql_store_result(con);
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result))){
   //check if pin is normal input or output
   switch(atoi(row[2])){

   case 0: //pin is normal output

   case 1: //pin is normal input
    //check if pin is enabled
    if(atoi(row[1])==1){
      //check if pin is inverted
      if(atoi(row[0])==1){
       gpioPins[x] = invert(gpio_read(config.pi,x));
      }else{
       gpioPins[x] = gpio_read(config.pi,x);
      }
    //pin is disabled set dummy value
    }else{
     gpioPins[x] = -1001;
    }
   break;
   case 2://pin is pwm-output
    //check if pin is enabled
    if(atoi(row[1])==0){ gpioPins[x] = -1001; }
   break;

   default://pin direction is unknown
    gpioPins[x] = -1001;
   break;
   }
  }
  mysql_free_result(result);
  memset(&query[0], 0, sizeof(query));
  snprintf(query, sizeof query, "INSERT INTO `gpio` (`address`, `pin`, `value`, `req`, `timestamp`) \
				  VALUES ('%sgpio', %d, %d, -1001, UNIX_TIMESTAMP(NOW(6))*1000000) \
				  ON DUPLICATE KEY UPDATE `value` = %d, `timestamp` = UNIX_TIMESTAMP(NOW(6))*1000000", config.prefix, x, gpioPins[x], gpioPins[x]);
  if (mysql_query(con, query)){
   printf("update: %s", mysql_error(con));
   exit(EXIT_FAILURE);
  }
 }
 mysql_close(con);
}

void updateOutput(void){
 MYSQL *con = mysql_init(NULL);
 if (con == NULL){
   printf("updateOutput: %s", mysql_error(con));
   exit(EXIT_FAILURE);
 }

 if (mysql_real_connect(con, config.host, config.user, config.pass,
  config.database, 0, NULL, 0) == NULL){
   printf("updateOutput: %s", mysql_error(con));
   exit(EXIT_FAILURE);
 }

 char query[500] = {0};
 snprintf(query, sizeof query, "Select `gpio-conf`.`inverted`, `gpio`.`pin`, `gpio`.`req`, `gpio-conf`.`direction` FROM `gpio` \
				INNER JOIN `gpio-conf` \
				WHERE `gpio-conf`.`address` = `gpio`.`address` \
				AND `gpio`.`pin` = `gpio-conf`.`pin` \
				AND `gpio-conf`.`enabled` = 1 \
				AND `gpio`.`address` LIKE '%%%s%%' \
				AND `gpio`.`req` != -1001", config.prefix);

 if (mysql_query(con, query)){
  printf("updateOutput: %s", mysql_error(con));
  exit(EXIT_FAILURE);
 }

 MYSQL_RES *result = mysql_store_result(con);
 if (result != NULL)
 {
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result))){
   if(atoi(row[3])==0){
    if(atoi(row[0])==0){
     printf("Gpio %d requested value %d", atoi(row[1]), atoi(row[2]));
     gpio_write(config.pi, atoi(row[1]), atoi(row[2]));
    }

    if(atoi(row[0])==1){
     printf("Gpio %d requested value %d", atoi(row[1]), atoi(row[2]));
     gpio_write(config.pi, atoi(row[1]), invert(atoi(row[2])));
    }
   }

   if(atoi(row[3])==2){
    if(atoi(row[2])<0){
     set_PWM_frequency(config.pi, (unsigned int)atoi(row[1]), 100000);
     set_PWM_dutycycle(config.pi, (unsigned int)atoi(row[1]), 0 * 2.55);
     printf("Gpio PWM-OUT pin %d requested value (%d) is to low, setting to 0", atoi(row[1]), atoi(row[2]));
     gpioPins[atoi(row[1])] = 0;
    }

    if(atoi(row[2])>100){
     set_PWM_frequency(config.pi, (unsigned int)atoi(row[1]), 100000);
     set_PWM_dutycycle(config.pi, (unsigned int)atoi(row[1]), 100 * 2.55);
     printf("Gpio PWM-OUT pin %d requested value (%d) is to high, setting to 100", atoi(row[1]), atoi(row[2]));
     gpioPins[atoi(row[1])] = 100;
    }

    if((atoi(row[2])<=100) && (atoi(row[2])>=0)){
     printf("Gpio PWM-OUT pin %d requested value %d", atoi(row[1]), atoi(row[2]));
     set_PWM_frequency(config.pi, (unsigned int)atoi(row[1]), 100000);
     set_PWM_dutycycle(config.pi, (unsigned int)atoi(row[1]), atoi(row[2]) * 2.55);
     gpioPins[atoi(row[1])] = atoi(row[2]);
    }
   }
   memset(&query[0], 0, sizeof(query));
   snprintf(query, sizeof query, "UPDATE `gpio` SET `req` = -1001, `timestamp` = UNIX_TIMESTAMP(NOW(6))*1000000 WHERE `gpio`.`address` = '%sgpio' AND `gpio`.`pin` = %s", config.prefix, row[1]);
   if (mysql_query(con, query)){
    printf("updateOutput: %s", mysql_error(con));
    exit(EXIT_FAILURE);
   }
  }
  mysql_free_result(result);
 }
 memset(&query[0], 0, sizeof(query));

 if (mysql_query(con, "CHECKSUM TABLE `gpio-conf`")){
  printf("updateOutput checksum: %s", mysql_error(con));
  exit(EXIT_FAILURE);
 }

 MYSQL_RES *check = mysql_store_result(con);
 if (check != NULL)
  {
  if (mysql_num_rows(check)){
   MYSQL_ROW row;
   while ((row = mysql_fetch_row(check))){
    if(config.checksum!=atoll(row[1])){
     initGpio();
    }else{
    }
    mysql_free_result(check);
   }
  }
 }
 mysql_close(con);
 return;
}
