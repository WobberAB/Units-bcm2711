 #include <unistd.h>
 #include <ctype.h>
 #include <string.h>
 #include "prefix.h"
 #include "config.h"

 char hostname[20] = {0};

 int createPrefix(void){
  if(gethostname(hostname, 20)==0){
   config.prefix[0] = toupper(hostname[0]);
   config.prefix[1] = toupper(hostname[1]);
   config.prefix[2] = toupper(hostname[strlen(hostname)-1]);
   return 0;
  }else{
   return 1;
  }
 }
