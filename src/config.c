#include <stdio.h>
#include <unistd.h>
#include "fileUtils.h"
#include "prefix.h"
#include "config.h"
#include "pid.h"
struct config_t config;

void loadConfiguration(void){
 createPrefix();

 config.delay = fileToInt("/etc/units/bcm2711/delay.cnf");
 if (config.delay == ERRNO) { config.delay = 1; }

 fileToString(config.pigpiodHost, sizeof(config.pigpiodHost), "/etc/units/bcm2711/pigpiod/host.cnf");
 fileToString(config.pigpiodPort, sizeof(config.pigpiodPort), "/etc/units/bcm2711/pigpiod/port.cnf");
 fileToString(config.host, sizeof(config.host), "/etc/units/bcm2711/mysql/host.cnf");
 fileToString(config.user, sizeof(config.user), "/etc/units/bcm2711/mysql/user.cnf");
 fileToString(config.pass, sizeof(config.pass), "/etc/units/bcm2711/mysql/pass.cnf");
 fileToString(config.database, sizeof(config.database), "/etc/units/bcm2711/mysql/db.cnf");
}
