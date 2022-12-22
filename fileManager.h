//
// Created by Matúš on 21. 12. 2022.
//

#include<stdio.h>
#include<string.h>
#include<dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>


typedef struct GlobalPath{
    char *globaPath;			/* Global path like C:\....  */

} GP;


int printOptions();
int makeDir(char *path,char *name);
int delDir(char *path, char * name);
int renameDirectory(char *path, char *path2);