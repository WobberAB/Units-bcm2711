#include <stdio.h>
#include <unistd.h>
#include "fileUtils.h"
#include "prefix.h"
#include "config.h"
#include "pid.h"
struct config_t config;

void loadConfiguration(void){
 createPrefix();

 config.delay = fileToInt("/usr/wobber/gpio2sql/delay.cnf");
 if (config.delay == ERRNO) { config.delay = 5; }

 fileToString(config.pigpiodHost, "/usr/wobber/gpio2sql/pigpiod/host.cnf");
 fileToString(config.pigpiodPort, "/usr/wobber/gpio2sql/pigpiod/port.cnf");
 fileToString(config.host, "/usr/wobber/gpio2sql/mysql/host.cnf");
 fileToString(config.user, "/usr/wobber/gpio2sql/mysql/user.cnf");
 fileToString(config.pass, "/usr/wobber/gpio2sql/mysql/pass.cnf");
 fileToString(config.database, "/usr/wobber/gpio2sql/mysql/db.cnf");
}
