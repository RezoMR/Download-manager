//
// Created by Matúš on 21. 12. 2022.
//

#include "fileManager.h"

char * scanner(){
    char * string = malloc(sizeof(char) * (255 + 1));
    while(true) {
        //printf("Enter the name: \n");
        if (scanf("%255s", string) != 1) {
            printf("Bad input, please retry\n");
            continue;
        }
        break;
    }
    return string;
}

int fileManager() {
    int choice = printOptions();

    switch (choice) {

        case 1://-------------MAKE DIRECTORY------------
            printf("You choose make Directory\n");
            printf("Put in the name: \n");
            char * string = scanner();
            makeDir(string);

            free(string);
            break;
        case 2:   //-------------DELETE DIRECTORY------------

            printf("You choose delete Directory\n");
            printDir();
            printf("Put in name of directory\n");
            char * string1 = scanner();
            delDir(string1);
            free(string1);
            break;

        case 3:     //--------------------MOVE FILE BETWEEN DIRS----------------

            printDir();
            printf("Choose Directory from which you want to move a File: \n");
            char * string2 = scanner();
            printDirE(string2);
            printf("Choose the File to move: \n");
            char * string3 = scanner();
            printf("Put in the name of Target Directory: \n");
            char * string4 = scanner();


            strcat(string2, "/");
            strcat(string2, string3);

            strcat(string4, "/");
            strcat(string4, string3);


            moveFile(string4, string2);

            free(string3);
            free(string4);
            free(string2);
            break;

        case 4:   //-------------PRINTOUT FILES IN DIRECTORY------------
            printDir();
            printf("Choose Directory: \n");
            char * string5 = scanner();
            printDirE(string5);
            free(string5);
            break;
        case 5: //-----------REMOVE FILE-------------
            printDir();
            printf("Choose Directory: \n");
            char * string6 = scanner();


            printDirE(string6);
            printf("Choose the File to remove: \n");
            char * string7 = scanner();

            strcat(string6, "/");
            strcat(string6, string7);

            removeFile(string6);

            free(string6);
            free(string7);
            break;

    }




}


int printOptions() {
    int choice;
    printf("Press 1 to Make Directory\n");
    printf("Press 2 to Delete Directory\n");
    printf("Press 3 to Move a File to other directory\n");
    printf("Press 4 print files in Directory\n");
    printf("Press 5 Remove a File\n");
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

int printDir(){
    DIR *dir;
    struct dirent *entry;
    dir = opendir(".");
    if (dir == NULL) {
        printf("You wrote wrong name");
        return 0;
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
        printf("You wrote wrong name");
        return 0;
    }

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
    return 0;
}


int moveFile(char * target, char * source) {
    int result = rename(source, target);
    if (result != 0) {
        printf("Failed to move the File\n");
        return 0;
    } else {
        printf("File was moved successfully\n");
        return 0;
    }
}

int removeFile(char * path){
    if (remove(path) != 0) {
        printf("Failed to remove the file\n");
        return 1;
    }

    printf("File removed successfully.\n");
    return 0;
}
