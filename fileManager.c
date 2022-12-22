//
// Created by Matúš on 21. 12. 2022.
//

#include "fileManager.h"


int fileManager() {
    char * string = malloc(sizeof(char) * (255 + 1));
    char * file = malloc(sizeof(char) * (255 + 1));
    char * dir = malloc(sizeof(char) * (255 + 1));
    char * targetDir = malloc(sizeof(char) * (255 + 1));
    int choice = printOptions();

    switch (choice) {

        case 1://-------------MAKE DIRECTORY------------
            printf("You choose make Directory\n");

            while(true) {
                printf("Enter the name: \n");
                if (scanf("%255s", string) != 1) {
                    printf("Bad input, please retry");
                    continue;
                }
                break;
            }
            makeDir(string);
            break;
        case 2:   //-------------DELETE DIRECTORY------------

            printf("You choose delete Directory\n");
            printDir();
            while(true) {
                printf("Enter the name: \n");
                if (scanf("%255s", string) != 1) {
                    printf("Bad input, please retry");
                    continue;
                }
                break;
            }
            delDir(string);
            break;



        case 3:           //dorobit------------------------------------
            printf("You choose rename Directory\n");

            break;



        case 5:     //dorobit move file --------------------------
            printDir();

            printf("Choose Directory: \n");
            while(true) {
                printf("Enter the name of Directory: \n");
                if (scanf("%255s", dir) != 1) {
                    printf("Bad input, please retry");
                    continue;
                }
                break;
            }
            printDirE(dir);

            while(true) {
                printf("Enter the name of File: \n");
                if (scanf("%255s", file) != 1) {
                    printf("Bad input, please retry");
                    continue;
                }
                break;
            }


            strcat(dir, "/");
            strcat(dir, file);

            while(true) {
                printf("Enter the name of Target Directory: \n");
                if (scanf("%255s", targetDir) != 1) {
                    printf("Bad input, please retry");
                    continue;
                }
                break;
            }


            moveFile(targetDir, dir);

            break;
        case 6:   //-------------WRITE FILES IN DIRECTORY------------
            printDir();
            printf("Choose Directory: \n");
            while(true) {
                printf("Enter the name of Directory: \n");
                if (scanf("%255s", dir) != 1) {
                    printf("Bad input, please retry");
                    continue;
                }
                break;
            }
            printDirE(dir);

            break;
    }

free(file);
free(dir);
free(string);
free(targetDir);
}


int printOptions() {
    int choice;
    printf("Press 1 to Make Directory\n");
    printf("Press 2 to Delete Directory\n");
    printf("Press 3 to Rename Directory\n");

    printf("Press 5 to Move a File to other directory\n");
    printf("Press 6 write files in Directory\n");

    printf("Press 0 to end application\n");
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


int makeDir(char *path) {
    int status;

    status = mkdir(path, 777);

    if (status == 0)
        printf("Directory was created successfully\n");
    else
        printf("Failed to create a Directory\n");

    return 0;

}

int delDir(char *path) {
    int status;
    status = rmdir(path);

    if (status == 0)
        printf("Directory was removed successfully\n");
    else
        printf("Failed to remove the Directory\n");

    return 0;

}
//
//int renameDirectory(char *path, char *path2) {
//    int status = 2;
//    status = rename(path, path2);
//
//    if (status == 0)
//        printf("Directory was renamed successsfully\n");
//    else
//        printf("Failed to rename the Directory\n");
//
//    return 0;
//}

int printDir(){
    DIR *dir;
    struct dirent *entry;
    dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
    return 0;
}

int printDirE(char * dirr){
    DIR *dir;
    struct dirent *entry;
    dir = opendir(dirr);
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
    return 0;
}


int moveFile(char * dir, char * file) {
    char * filee = "./";
    strcat(filee, file);


    char * dest = "./";
    strcat(dest, dir);
    strcat(dest, "/");
    strcat(dest, file);

    if (rename(file, dest) != 0) {
        perror("rename");
        return 1;
    }
    return 0;
}




//presun suborov medzi directories,
