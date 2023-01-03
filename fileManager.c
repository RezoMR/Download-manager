#include "fileManager.h"

char * scanner(){
    char * string = malloc(sizeof(char) * (255 + 1));
    while(1) {
        if (scanf("%255s", string) != 1) {
            printf("Bad input, please retry\n");
            continue;
        }
        break;
    }
    return string;
}

void fileManager() {
    int choice = printOptions();

    switch (choice) {
        case 1://-------------CREATE DIRECTORY------------
            printf("Put in the name of the directory to create: \n");
            char * dirToCreate = scanner();
            makeDir(dirToCreate);
            free(dirToCreate);
            break;
        case 2://-------------DELETE DIRECTORY------------
            printf("Put in name of directory to delete: \n");
            char * dirToDelete = scanner();
            delDir(dirToDelete);
            free(dirToDelete);
            break;
        case 3://--------------------MOVE FILE BETWEEN DIRS----------------
            printf("Put in the name of a directory from which you want to move the file: \n");
            char * dirFrom = scanner();
            printDirE(dirFrom);
            printf("Put in name of the file to move: \n");
            char * fileName = scanner();
            printf("Put in the name of the target directory: \n");
            char * dirTo = scanner();

            strcat(dirFrom, "/");
            strcat(dirFrom, fileName);

            strcat(dirTo, "/");
            strcat(dirTo, fileName);
            moveFile(dirTo, dirFrom);

            free(fileName);
            free(dirTo);
            free(dirFrom);
            break;
        case 4://-------------PRINT FILES IN DIRECTORY------------
            printDir();
            printf("Put in the name of a directory to print the contents of: \n");
            char * dirToPrint = scanner();
            printDirE(dirToPrint);
            free(dirToPrint);
            break;
        case 5://-----------REMOVE FILE-------------
            printDir();
            printf("Put in the name of a directory to remove the file from: \n");
            char * dirToRemoveFrom = scanner();

            printDirE(dirToRemoveFrom);
            printf("Put in the name of the file to remove: \n");
            char * fileToRemove = scanner();

            strcat(dirToRemoveFrom, "/");
            strcat(dirToRemoveFrom, fileToRemove);

            removeFile(dirToRemoveFrom);

            free(dirToRemoveFrom);
            free(fileToRemove);
            break;
    }
}


int printOptions() {
    int choice;
    printf("Press 1 to create directory\n");
    printf("Press 2 to delete directory\n");
    printf("Press 3 to move a file to other directory\n");
    printf("Press 4 print files in directory\n");
    printf("Press 5 remove a file\n");
    printf("Press 0 to exit\n");
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

    status = mkdir(path, 0777);
    if (status == 0)
        printf("Directory was created successfully\n");
    else
        printf("Failed to create the directory\n");

    return 0;

}

int delDir(char *path) {
    int status = rmdir(path);

    if (status == 0)
        printf("Directory was removed successfully\n");
    else
        printf("Failed to remove the directory\n");

    return 0;
}

int printDir(){
    printDirE(".");
    return 0;
}

int printDirE(char * dirr){
    DIR *dir;
    struct dirent *entry;
    dir = opendir(dirr);
    if (dir == NULL) {
        printf("You put in the wrong name\n");
        return 0;
    }
    printf("\n");
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }
    printf("\n");

    closedir(dir);
    return 0;
}

int moveFile(char * target, char * source) {
    int result = rename(source, target);
    if (result != 0) {
        printf("Failed to move the file\n");
    } else {
        printf("File was moved successfully\n");
    }

    return 0;
}

int removeFile(char * path){
    if (remove(path) != 0) {
        printf("Failed to remove the file\n");
        return 1;
    }

    printf("File removed successfully\n");
    return 0;
}
