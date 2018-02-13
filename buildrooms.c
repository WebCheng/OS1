#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ROOM_LIST_NUM 10
#define ROOM_USED_NUM 7

char *room_name_list[ROOM_LIST_NUM] = {
    "aa",
    "bb",
    "cc",
    "dd",
    "ee",
    "ff",
    "gg",
    "hh",
    "ii",
    "jj"};

struct Room
{
    /*room name (MAX len 8)*/
    char *room_name;
    /*Connecting room number*/
    int conNum;
    /*Connecting room ( Array )*/
    struct Connect *connect;
    /*START_ROOM; MID_ROOM; END_ROOM;*/
    char *room_type;
};

struct Connect
{
    /*connecting room number*/
    int room_idx;
    /*point back to the room array.*/
    struct Room *connect_room;
};

/*Checking the used room names*/
int isUsedIdx(int *list, int num, int len)
{
    int i;
    for (i = 0; i < len; i++)
        if (list[i] == num)
            return 1;

    return 0;
}

/*
* 1. malloc temp list(usedIdx) for checking the already used name.
* 2. rand the room name
* 3. check the room if already has be used than used next one name
* 4. set the room info
*/
void randRooms(struct Room *rmArr)
{
    int numRm;
    int *usedIdx = malloc(sizeof(int) * ROOM_USED_NUM);

    for (numRm = 0; numRm < ROOM_USED_NUM; numRm++)
    {
        int idx = rand() % 9 + 0;

        /*  Checking the room already be used or not             *
         *  if is Used than (index += 1) until get non-used room.*/
        while (isUsedIdx(usedIdx, idx, ROOM_USED_NUM))
        {
            idx += 1;
            if (idx == ROOM_LIST_NUM)
                idx = 0;
        }

        strcpy(rmArr[numRm].room_name, room_name_list[idx]);

        /*First room = START_ROOM; Last room = "END_ROOM"; Others room = "MID_ROOM"*/
        if (numRm == 0)
            strcpy(rmArr[numRm].room_type, "START_ROOM");
        else if (numRm == (ROOM_USED_NUM - 1))
            strcpy(rmArr[numRm].room_type, "END_ROOM");
        else
            strcpy(rmArr[numRm].room_type, "MID_ROOM");

        usedIdx[numRm] = idx;
    }
    free(usedIdx);
}

/*Checking the room has been conected */
int isConnect(struct Room *rmArr, int idx1, int idx2)
{
    int i;
    for (i = 0; i < rmArr[idx1].conNum; i++)
        if (rmArr[idx1].connect[i].room_idx == idx2)
            return 1;

    return 0;
}

/*
 * idx1 = connecting node
 * idx2 = connected node
 */
void setConRoom(struct Room *rmArr, int idx1, int idx2)
{
    rmArr[idx2].connect[rmArr[idx2].conNum].connect_room = &rmArr[idx1];
    rmArr[idx2].connect[rmArr[idx2].conNum].room_idx = idx1;
    rmArr[idx2].conNum += 1;
}

/*
* 1. rand the connect room number
* 2. check  the room has been connected or not if YES than used next one 
* 3. Connect bothe rooms
*/
void connectRoom(struct Room *rmArr)
{
    int i, j;
    for (i = 0; i < ROOM_USED_NUM; i++)
    {
        /*Random connect room numbers(3 ~ 6)*/
        int conNum = rand() % (6 - 3 + 1) + 3; 

        /*(conNum - rmArrp[i].conNum) => remain number need to connect*/
        for (j = rmArr[i].conNum; j < conNum; j++)
        {
            /*Random number to get the room in the rmArr[]*/
            int idx2 = rand() % 6 + 1;
            /* Check the room has been connected or not
             * If connected than find the next room to connect.*/
            while (idx2 == i || isConnect(rmArr, i, idx2))
            {
                idx2 += 1;
                if (idx2 == ROOM_USED_NUM)
                    idx2 = 0;
            }

            /*Connect both room*/
            setConRoom(rmArr, idx2, i);
            setConRoom(rmArr, i, idx2);
        }
    }
}

/*  using fopen for easy write the file;
 *   1. create dir [chengwe.rooms.pid]
 *   2. cd to dir 
 *   3. create the file and write content
 *   4. cd ..
 */
void generateFile(struct Room *rmArr)
{
    pid_t pid = getpid();
    int fileLen = strlen("chengwe") + strlen(".rooms.") + 10;
    char fDir[fileLen];
    sprintf(fDir, "%s.rooms.%d", "chengwe", pid);

    mkdir(fDir, 0700);
    chdir(fDir);

    int i, j;
    for (i = 0; i < ROOM_USED_NUM; i++)
    {
        FILE *file = fopen(rmArr[i].room_name, "w");

        fprintf(file, "ROOM NAME: %s\n", rmArr[i].room_name);

        for (j = 0; j < rmArr[i].conNum; j++)
        {
            fprintf(file, "CONNECTION %d: %s\n", j + 1, rmArr[i].connect[j].connect_room->room_name);
        }

        fprintf(file, "ROOM TYPE: %s", rmArr[i].room_type);

        fclose(file);
    }

    chdir("..");
}

/*Malloc mmemory for the array*/
struct Room *mallocRooms()
{
    int i, j;
    struct Room *rmArr = malloc(sizeof(struct Room) * ROOM_USED_NUM);
    for (i = 0; i < ROOM_USED_NUM; i++)
    {
        rmArr[i].conNum = 0;
        rmArr[i].room_name = malloc(sizeof(char) * 8);
        rmArr[i].connect = malloc(sizeof(struct Connect) * 6);
        rmArr[i].room_type = malloc(sizeof(char) * 10);
    }
    return rmArr;
}

/*Free the memmory*/
void freeArr(struct Room *rmArr)
{
    int i;
    for (i = 0; i < ROOM_USED_NUM; i++)
    {

        free(rmArr[i].room_name);
        free(rmArr[i].connect);
        free(rmArr[i].room_type);
    }
    free(rmArr);
}

int main()
{
    /*random seed*/
    srand((unsigned)time(NULL));

    struct Room *rmArr = mallocRooms();

    randRooms(rmArr);
    connectRoom(rmArr);
    generateFile(rmArr);
    freeArr(rmArr);

    return 0;
}
