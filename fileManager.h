//
// Created by Matúš on 21. 12. 2022.
//

#include<stdio.h>
#include<string.h>
#include<dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

char * scanner();
int fileManager();
int printDir();
int printDirE(char * name);


int printOptions();


int makeDir(char *path);
int delDir(char *path);
int moveFile(char * target, char * source);
int removeFile(char * path);
int removeDirFile(char * path);