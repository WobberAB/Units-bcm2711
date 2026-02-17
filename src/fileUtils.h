/* fileUtils.h */
#ifndef ___FILEUTILS_H
#define ___FILEUTILS_H
extern void fileToString(char *h, size_t h_size, const char *filename);
extern double fileTodouble(const char *filename);
extern int fileToInt(const char *filename);
extern int configDirExists(void);
#endif
