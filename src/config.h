/* config.h */
#ifndef ___CONFIG_H
#define ___CONFIG_H
#define ERRNO -1001

typedef struct config_t {
   char  host[20];
   char  prefix[20];
   char  database[20];
   char  user[20];
   char  pass[20];
   char  pigpiodHost[20];
   char  pigpiodPort[20];
   int   pi;
   int   delay;
   long long int checksum;
}config_t;

extern struct config_t config;
extern void loadConfiguration(void);
#endif
