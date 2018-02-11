#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ROOM_USED_NUM 7

struct Room
{
    /*room name (MAX len 8)*/
    char *room_name;
    /*Connecting room number*/
    int conNum;
    /*Connecting room number*/
    struct Connect *connect;
    /*1st:START_ROOM 2-:MID_ROOM Last:END_ROOM*/
    char *room_type;
};

struct Connect
{
    /*connecting room number*/
    int room_idx;
    char *con_name;
    /*point to the*/
    struct Room *connect_room;
};

void getDir(char *dir)
{
    struct stat fileState;
    time_t lastModified;
    DIR *d = opendir(".");
    struct dirent *dp;
    int count = 0;
    char tmp[256] = "./";

    if (d)
    {
        while ((dp = readdir(d)) != NULL)
        {
            if (strstr(dp->d_name, "chengwe.rooms.") == NULL)
                continue;

            stat(dp->d_name, &fileState);

            if (count != 0 && lastModified >= fileState.st_mtime)
                continue;

            lastModified = fileState.st_mtime;
            strcpy(dir, dp->d_name);
            count += 1;
        }
        closedir(d);
    }
    strcat(dir, "/");
    strcat(tmp, dir);
    strcpy(dir, tmp);
}


void loadFileContent(char *dirFile, struct Room *rmArr, int i)
{
    char *splitC = ":", *line = NULL, *rmName, *rmType, *rmCon;
    FILE *fp;
    size_t len = 0;
    ssize_t read;

    fp = fopen(dirFile, "r");

    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (strstr(line, "ROOM NAME") != NULL)
        {
            rmName = strtok(line, ":");
            rmName = strtok(NULL, " \n");
            strcpy(rmArr[i].room_name, rmName);
        }
        else if (strstr(line, "CONNECTION") != NULL)
        {
            rmCon = strtok(line, " :");
            rmCon = strtok(NULL, " :");
            rmCon = strtok(NULL, " :\n");
            strcpy(rmArr[i].connect[rmArr[i].conNum].con_name, rmCon);
            rmArr[i].conNum += 1;
        }
        else if (strstr(line, "ROOM TYPE") != NULL)
        {
            rmType = strtok(line, ":");
            rmType = strtok(NULL, " \n");
            strcpy(rmArr[i].room_type, rmType);
        }
    }

    fclose(fp);
    if (line)
        free(line);
}

void loadFile(struct Room *rmArr)
{
    char *dir = malloc(sizeof(char) * 256);
    char *dirFile = malloc(sizeof(char) * 256);
    getDir(dir);

    DIR *d = opendir(dir);
    struct dirent *dp;
    int i = 0;
    if (d)
    {
        while ((dp = readdir(d)) != NULL)
        {
            if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
                continue;

            strcpy(dirFile, dir);
            strcat(dirFile, dp->d_name);
            loadFileContent(dirFile, rmArr, i);
            i += 1;
        }
    }
}

struct Room *creatRmArr()
{
    int i, j;
    struct Room *rmArr = malloc(sizeof(struct Room) * ROOM_USED_NUM);
    for (i = 0; i < ROOM_USED_NUM; i++)
    {
        rmArr[i].conNum = 0;
        rmArr[i].room_name = malloc(sizeof(char) * 8);
        rmArr[i].connect = malloc(sizeof(struct Connect) * 6);
        rmArr[i].room_type = malloc(sizeof(char) * 10);

        for (j = 0; j < 6; j++)
            rmArr[i].connect[j].con_name = malloc(sizeof(char) * 8);
    }

    return rmArr;
}

int conRmPointGetStartRm(struct Room *rmArr)
{
    int i, j, k, curIdx;
    for (i = 0; i < ROOM_USED_NUM; i++)
    {
        for (j = 0; j < rmArr[i].conNum; j++)
        {
            for (k = 0; k < ROOM_USED_NUM; k++)
            {
                if (strstr(rmArr[i].connect[j].con_name, rmArr[k].room_name) != NULL)
                {
                    rmArr[i].connect[j].room_idx = k;
                    rmArr[i].connect[j].connect_room = &rmArr[k];
                }
            }
        }

        if (strstr(rmArr[i].room_type, "START_ROOM") != NULL)
            curIdx = i;
    }
    return curIdx;
}

void printFileinfo(struct Room *rmArr, int rmIdx)
{
    int i;
    printf("CURRENT LOCATION: %s\n", rmArr[rmIdx].room_name);
    printf("PSSIBLE CONNECTIONS:");
    for (i = 0; i < rmArr[rmIdx].conNum; i++)
    {
        if (i != rmArr[rmIdx].conNum - 1)
            printf(" %s,", rmArr[rmIdx].connect[i].connect_room->room_name);
        else
            printf(" %s.\n", rmArr[rmIdx].connect[i].connect_room->room_name);
    }
    printf("WHERE TO? >");
}

int main()
{
    char *playerPath[100];
    char playerMove[100];
    int numOfSteps = 0, i, j, exists;
    struct Room *rmArr = creatRmArr();
    loadFile(rmArr);

    int curIdx = conRmPointGetStartRm(rmArr);

    while (1)
    {

        exists = 0;
        if (strcmp(rmArr[curIdx].room_type, "END_ROOM") == 0)
        {
            printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
            printf("YOU TOOK %i STEPS. YOUR PATH TO VICTORY WAS:\n", numOfSteps);
            for (i = 0; i < numOfSteps; i++)
            {
                printf("%s\n", playerPath[i]);
            }

            break;
        }
        printFileinfo(rmArr, curIdx);
        scanf("%99s", playerMove);

        for (j = 0; j < rmArr[curIdx].conNum; j++)
        {
            if (strcmp(playerMove, rmArr[curIdx].connect[j].con_name) == 0)
            {
                playerPath[numOfSteps] = rmArr[curIdx].connect[j].con_name;
                numOfSteps += 1;
                curIdx = rmArr[curIdx].connect[j].room_idx;
                exists = 1;
            }
        }
        if (!exists)
            printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n");

        printf("\n");
    }
 
    return 0;
}
