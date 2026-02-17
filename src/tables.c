#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "bwlog.h"
#include "tables.h"

void createTables(void){
 MYSQL *con = mysql_init(NULL);
 if (con == NULL){
  bwlog("createTables: %s ", mysql_error(con));
  exit(EXIT_FAILURE);
 }

 if (mysql_real_connect(con, config.host, config.user, config.pass,
  config.database, 0, NULL, 0) == NULL){
  bwlog("createTables: %s ", mysql_error(con));
  exit(EXIT_FAILURE);
 }


 bwlog("Creating gpio table...");
 char query[2000] ="";
 snprintf(query, sizeof query, "CREATE TABLE IF NOT EXISTS `gpio` (   \
				`address` VARCHAR(20) DEFAULT NULL, \
				`pin` INT(2) DEFAULT NULL, \
				`value` int(1) DEFAULT -1001 COMMENT '-1001=unassigned, 0=low, 1=high', \
				`req` INT(2) DEFAULT -1001 COMMENT '-1001=no request, 0=low, 1=high, 0-100 for pwm', \
		                `text` VARCHAR(100) DEFAULT NULL, \
				`timestamp` bigint(20) DEFAULT NULL, \
				PRIMARY KEY (`address`,`pin`)) \
				ENGINE=MEMORY DEFAULT CHARSET=ascii COLLATE=ascii_general_ci");

 if (mysql_query(con, query)){
  bwlog("There was a problem creating gpio-table!");
  bwlog("createTables: %s ", mysql_error(con));
  exit(EXIT_FAILURE);
 }else{
  bwlog("gpio table created successfully!");
 }

 memset(&query[0], 0, sizeof(query));

 bwlog("Creating gpio-conf table...");
 snprintf(query, sizeof query, "CREATE TABLE IF NOT EXISTS `gpio-conf` (   \
				`address` VARCHAR(20) DEFAULT NULL, \
				`pin` INT(2) DEFAULT NULL, \
				`direction` INT(2) DEFAULT 1 COMMENT '-1001=unassigned, 0=output, 1=input 2=pwm-out 3=freq-in', \
				`pullup` INT(2) DEFAULT 2 COMMENT '0=off, 1=pull down, 2=pull up', \
				`interrupt` INT(2) DEFAULT 0 COMMENT '0=disabled, 1=falling edge, 2=rising edge, 3=either edge', \
				`inverted` INT(2) DEFAULT 1 COMMENT '0=false, 1=true', \
				`glitch` INT(2) DEFAULT 100000 COMMENT '-1001=No glitch filter, default is 100000', \
				`enabled` INT(2) DEFAULT 1 COMMENT '0=disabled, 1=enabled', \
				PRIMARY KEY (`address`,`pin`)) DEFAULT CHARSET=ascii COLLATE=ascii_general_ci");
 if (mysql_query(con, query)){
  bwlog("There was a problem creating gpio-conf table!");
  bwlog("createTables: %s ", mysql_error(con));
  exit(EXIT_FAILURE);
 }else{
  bwlog("gpio-conf table created successfully!");
 }

 bwlog("Creating trigger gpioReqUpdated in db %s", config.database);
 memset(&query[0], 0, sizeof(query));
 snprintf(query, sizeof query, "DROP TRIGGER IF EXISTS `gpioReqUpdated`;");

 if (mysql_query(con, query)){
  bwlog("There was a problem creating trigger gpioReqUpdated!");
  bwlog("createTables: %s ", mysql_error(con));
  exit(EXIT_FAILURE);
 }
 memset(&query[0], 0, sizeof(query));
 snprintf(query, sizeof query, "CREATE TRIGGER gpioReqUpdated \
                                AFTER UPDATE ON gpio \
                                FOR EACH ROW \
                                BEGIN DECLARE cmd CHAR(255); \
                                DECLARE result int(10); \
                                IF NEW.`req` <> OLD.`req` THEN \
				 IF NEW.`req`!=-1001 THEN \
                                  SET cmd=('/usr/wobber/gpio2sql/binaries/gpio-req-update'); \
                                  SET result = sys_exec(cmd); \
                                 END IF; \
                                END IF; \
                                END;");
 if (mysql_query(con, query)){
  bwlog("There was a problem creating gpio-conf req-trigger!");
  bwlog("createTables: %s ", mysql_error(con));
  exit(EXIT_FAILURE);
 }
 mysql_close(con);
 return;
}
