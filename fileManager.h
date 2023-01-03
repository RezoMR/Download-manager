#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

#ifdef	__cplusplus
extern "C" {
#endif

char * scanner();
void fileManager();
int printDir();
int printDirE(char * name);

int printOptions();

int makeDir(char *path);
int delDir(char *path);
int moveFile(char * target, char * source);
int removeFile(char * path);

#ifdef	__cplusplus
}
#endif
