#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
/*
Several complex and weird parts.

Load ALL the file in the special directory.
Because of getting the last modifie time.

Load ALL the file and create the room data including the connect information.
Because of getting the "START_ROOM" info.

Running the code using the while loop and several if to control the game output.
Cannot figur out more quickly way to do it and no time to refine it.

Mutiple thread
Dont understand all of the thread part.
Just fallowing the professor hint.
Others quesiton: parallel 
*/

#define ROOM_USED_NUM 7

pthread_t tid;
pthread_mutex_t lock;

struct Room
{
    /*room name (MAX len 8)*/
    char *room_name;
    /*Connecting room number*/
    int conNum;
    /*Connecting room number*/
    struct Connect *connect;
    /*START_ROOM; MID_ROOM; END_ROOM;*/
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

/*
* 1. load the file (NAME LIKE "chengwe.rooms.")
* 2. get the last mod time file
*/
void getlastDir(char *dir)
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

            /*First value can pass it &&&&&&   get the last mod time file*/
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

/*Load the data to the ROOM ARRAY
* 1. read the data
* 2. get the substring
*/
void loadFileContent(char *dirFile, struct Room *rmArr, int i)
{
    char *splitC = ":", *line = NULL, *rmName, *rmType, *rmCon;
    FILE *fp;
    size_t len = 0;
    ssize_t read;

    fp = fopen(dirFile, "r");

    while ((read = getline(&line, &len, fp)) != -1)
    {
        /*EX: "ROOM NAME: XYZZY\n"*/
        if (strstr(line, "ROOM NAME") != NULL)
        {
            rmName = strtok(line, ":");
            rmName = strtok(NULL, " \n");
            strcpy(rmArr[i].room_name, rmName);
        }
        /*EX: "CONNECTION 1: PLOVER\n"*/
        else if (strstr(line, "CONNECTION") != NULL)
        {
            rmCon = strtok(line, " :");
            rmCon = strtok(NULL, " :");
            rmCon = strtok(NULL, " :\n");
            strcpy(rmArr[i].connect[rmArr[i].conNum].con_name, rmCon);
            rmArr[i].conNum += 1;
        }
        /*EX: "ROOM TYPE: START_ROOM\n"*/
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

/*
* 1. get the last modified directory (file_name like "chengwe.roo%")
* 2. load data from the file under the directory to the Array.
*/
void loadFile(struct Room *rmArr)
{
    char *dir = malloc(sizeof(char) * 256);
    char *dirFile = malloc(sizeof(char) * 256);
    getlastDir(dir);

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
    free(dir);
    free(dirFile);
}

/*Malloc mmemory for the array*/
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

/*Free the memmory*/
void freeArr(struct Room *rmArr)
{
    int i, j;
    for (i = 0; i < ROOM_USED_NUM; i++)
    {
        for (j = 0; j < 6; j++)
            free(rmArr[i].connect[j].con_name);
        free(rmArr[i].room_name);
        free(rmArr[i].connect);
        free(rmArr[i].room_type);
    }
    free(rmArr);
}

/* Using three layer loop to set the connecting infor
* 1. setting the connect room( pointer to the room array[i] )
* 2. setting the room id (the index of connecting room)
* 3. setting the room type (START_ROOM, MID_ROOM, END_ROOM)
* 4. Return the start room array index
*/
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

/*
* print the game information
*/
void printFileinfo(struct Room *rmArr, int rmIdx)
{
    int i;
    printf("CURRENT LOCATION: %s\n", rmArr[rmIdx].room_name);
    printf("POSSIBLE CONNECTIONS:");
    for (i = 0; i < rmArr[rmIdx].conNum; i++)
    {
        /*last connect room => change the line*/
        if (i != rmArr[rmIdx].conNum - 1)
            printf(" %s,", rmArr[rmIdx].connect[i].connect_room->room_name);
        else
            printf(" %s.\n", rmArr[rmIdx].connect[i].connect_room->room_name);
    }
}

/*
* second thread to generate txt file
* file content:  1:03pm, Tuesday, September 13, 2016
*/
void *genCurrentTimeFile(void *arg)
{
    /*lock the main thread*/
    pthread_mutex_lock(&lock);

    char outstr[200];
    time_t t;
    struct tm *tmp;

    FILE *file = fopen("./currentTime.txt", "w");

    t = time(NULL);
    tmp = localtime(&t);
    strftime(outstr, sizeof(outstr), "%I:%M%p, %A, %B %d, %Y", tmp);
    fprintf(file, "%s", outstr);
    fclose(file);

    pthread_mutex_unlock(&lock);
    return NULL;
}

/*
* read file and print the content in the file
*/
void readDataPrint(char *file)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen(file, "r");

    while ((read = getline(&line, &len, fp)) != -1)
    {
        printf("\n%s\n", line);
    }
    fclose(fp);
    if (line)
        free(line);
}

/*
* Using the while loop for continue running the game script
* 
*/
void runTheGame(struct Room *rmArr, int curIdx)
{
    /*playerPath = the room play been; playerMove = the room */
    char *playerPath[100], playerMove[100];
    int numOfSteps = 0, i, j, exists;
    while (1)
    {
        exists = 0;

        /*For the END Room step=> print all room been before;  break;*/
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

        /*If command == "time" not print the game info*/
        if (strcmp(playerMove, "time") != 0)
            printFileinfo(rmArr, curIdx);

        /*scan the palyer command*/
        printf("WHERE TO? >");
        scanf("%99s", playerMove);

        if (strcmp(playerMove, "time") != 0)
        {
            /*check command room is in the connect list*/
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
                printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
        }
        else
        {
            /* unlock the thread line*/
            pthread_mutex_unlock(&lock);
            /* start the thead */
            pthread_join(tid, NULL);
            /*lock the thread line*/
            pthread_mutex_lock(&lock);
            /*create the thread line for run*/
            pthread_create(&tid, NULL, &genCurrentTimeFile, NULL);
            /*Get the time generate from the thread*/
            readDataPrint("./currentTime.txt");
        }

        printf("\n");
    }
}

int main()
{
    struct Room *rmArr = creatRmArr();
    
    loadFile(rmArr);
    int curIdx = conRmPointGetStartRm(rmArr);
     
    /*lock thread and create */
    pthread_mutex_lock(&lock);
    pthread_create(&tid, NULL, &genCurrentTimeFile, NULL); 

    runTheGame(rmArr, curIdx);

    freeArr(rmArr);

    return 0;
}
