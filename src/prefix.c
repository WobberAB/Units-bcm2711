#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "prefix.h"
#include "config.h"

 char hostname[20] = {0};

 int createPrefix(void){
  if(gethostname(hostname, 20)==0){
   size_t len = strlen(hostname);
   /* Guard against hostnames shorter than 2 characters to avoid
      out-of-bounds reads on hostname[1] and hostname[strlen-1]. */
   if(len < 2){
    printf("Hostname is less than 3 characters");
    return 1;
   }
   config.prefix[0] = tolower((unsigned char)hostname[0]);
   config.prefix[1] = tolower((unsigned char)hostname[1]);
   config.prefix[2] = tolower((unsigned char)hostname[len - 1]);
   config.prefix[3] = '\0';   /* explicit null terminator — safe even if struct is not zeroed */
   return 0;
  }else{
   printf("createPrefix: could not get hostname.");
   return 1;
  }
 }
