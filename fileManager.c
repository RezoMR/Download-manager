//
// Created by Matúš on 21. 12. 2022.
//

#include "fileManager.h"

//#include<stdio.h>
//#include<string.h>
//#include<dirent.h>
//#include <unistd.h>
//#include <sys/stat.h>
//#include <stdlib.h>



int getPath() {
    printf("Write the path: \n");
    char * string = malloc(sizeof(char) * (255 + 1));

    while (1) {
        if (scanf("%d", &string) == 1)
            break;
        else
            printf("You put in wrong value... please retry\n");
        while (getchar() != '\n')
            continue;
    }
}

int printOptions() {
    int choice;

    printf("Press 1 to make a directory\n");
    printf("Press 2 to remove a directory\n");
    printf("Press 3 to move a file\n");
    printf("Press 0 to return\n");

    while (1) {
        if (scanf("%d", &choice) == 1)
            break;
        else
            printf("You put in wrong value... please retry\n");
        while (getchar() != '\n')
            continue;
    }
    return choice;
}


int makeDir(char *path, char * name) {
    GP * gp = path;
    int status;
    char * ReverseSlash = "\\";
    strcat(path, ReverseSlash);
    strcat(path, name);

    status = mkdir(path, 777);

    if (status == 0)
        printf("Directory was created successfully\n");
    else
        printf("Failed to create a Directory\n");

    return 0;

}

int delDir(char *path, char * name) {
    GP * gp = path;
    char * ReverseSlash = "\\";
    strcat(path, ReverseSlash);
    strcat(path, name);

    int status;
    status = rmdir(name);

    if (status == 0)
        printf("Directory was removed successfully\n");
    else
        printf("Failed to remove the Directory\n");

    return 0;

}

int renameDirectory(char *path, char *path2) {
    int status = 2;
    status = rename(path, path2);

    if (status == 0)
        printf("Directory was renamed successsfully\n");
    else
        printf("Failed to rename the Directory\n");

    return 0;
}

