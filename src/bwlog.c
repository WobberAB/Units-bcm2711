#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "bwlog.h"

static char LOGFILE[250] =  "/mnt/ramdisk/log/units-bcm2711.log";

void bwlog(char *message, ...){
 char buffer[10000];
 FILE *fp;
 fp  = fopen (LOGFILE, "a");
 if (fp == NULL) {
  char homeFile[250];
  strcat(strcpy(homeFile, getenv("HOME")), "/units-bcm2711.log");
  printf("Can't open %s for output, reverting to %s instead.\n", LOGFILE, homeFile);
  memset(&LOGFILE[0], 0, sizeof(LOGFILE));
  snprintf(LOGFILE, sizeof LOGFILE, "%s", homeFile);
  fp = fopen (LOGFILE, "a");
 }

 time_t t = time(NULL);
 struct tm tm = *localtime(&t);

 va_list ap; // points to each unnamed arg in turn
 char *p, *sval;
 int ival;
 double dval;
 long ldval;
 unsigned int ucval;
 va_start(ap, message); // make ap point to 1st unnamed arg
 fprintf(fp, "%02d-%02d %02d:%02d:%02d: ", tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
 for (p = message; *p; p++){
  if (*p != '%'){
   fputc(*p, fp);
   continue;
  }
 switch (*++p){
   case 'd':
    ival = va_arg(ap, int);
    fprintf(fp, "%d", ival);
//    sprintf(buffer + strlen(buffer),"%d", ival);
    break;
   case 'f':
    dval = va_arg(ap, double);
    fprintf(fp, "%f", dval);
//    sprintf(buffer + strlen(buffer),"%f", dval);
    break;
   case 'x':
    ucval = va_arg(ap, unsigned int);
    fprintf(fp, "%02x", ucval);
//    sprintf(buffer + strlen(buffer),"%02x", ucval);
    break;
   case 'l':
    ldval = va_arg(ap, long);
    fprintf(fp, "%ld", ldval);
//    sprintf(buffer + strlen(buffer),"%ld", ldval);
    break;
   case 's':
    for (sval = va_arg(ap, char *); *sval; sval++){
     fputc(*sval, fp);
//     sprintf(buffer + strlen(buffer),"%s", *sval);
    }
    break;
   default:
    fputc(*p, fp);
    sprintf(buffer + strlen(buffer),"%c", *p);
    break;
  }
 }
 va_end(ap); // clean up when done
 fprintf(fp, "\n");
 fclose(fp);
}

