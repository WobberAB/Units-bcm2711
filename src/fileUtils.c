#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include "fileUtils.h"
#include "config.h"

void fileToString(char h[100], char filename[100]){
 char buff[100];
 if(filename != NULL){
  FILE* ptr_file;
  ptr_file = fopen(filename,"r");
  if (ptr_file == NULL) {
   openlog("units config", LOG_PID, LOG_USER);
   syslog(LOG_INFO, "%s was not found. Exiting!", filename);
   closelog();
   exit(EXIT_FAILURE);
  }else{
   fscanf(ptr_file,"%s", buff);
   fclose(ptr_file);
   strcat(h, buff);
  }
 }
}

double fileTodouble(char filename[100]){
 if(filename != NULL){
  char buff[2];
  FILE* ptr_file;
  ptr_file =fopen(filename,"r");
  if (ptr_file == NULL) {
   openlog("units config", LOG_PID, LOG_USER);
   syslog(LOG_INFO, "%s was not found", filename);
   closelog();
   return ERRNO;
  }else{
   fscanf(ptr_file,"%s", buff);
   fclose(ptr_file);
   return atof(buff);
  }
 }
 return ERRNO;
}

int fileToInt(char filename[100]){
 if(filename != NULL){
  char buff[2];
  FILE* ptr_file;
  ptr_file =fopen(filename,"r");
  if (ptr_file == NULL) {
   openlog("units config", LOG_PID, LOG_USER);
   syslog(LOG_INFO, "%s was not found", filename);
   closelog();
   return ERRNO;
  }else{
   fscanf(ptr_file,"%s", buff);
   fclose(ptr_file);
   return atoi(buff);
  }
 }
 return ERRNO;
}

