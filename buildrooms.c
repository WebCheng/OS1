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

/*
*EXAMPLE:
*   LIST room [7];
*
*   room[0]{
*       room name = "XXXX";
*       connect number = 2;
*       connect =>point to connection room list
*       room type = "END_ROOM"
*           }
*
*   Connect[0-1]
*       room idx = will be the LIST room position index
*       room name => point to the roomlist[X] position
*/
struct Room
{
    /*room name (MAX len 8)*/
    char *room_name; 
    /*Connecting room number*/
    int conNum;
    /*Connecting room number*/
    struct Connect *connect;
    /*1st:START_ROOM 2-:MID_ROOM Last:END_ROOM*/
    char* room_type;
};

struct Connect
{
    /*connecting room number*/
    int room_idx;
    /*point to the*/
    struct Room *connect_room;
};

int isContainIdx(int *list, int num, int len)
{
    int i;
    for (i = 0; i < len; i++)
        if (list[i] == num)
            return 1;

    return 0;
}

void createRooms(struct Room *rmArr)
{
    int numRm = 0;
    int *usedIdx = malloc(sizeof(int) * ROOM_USED_NUM);

    while (numRm < ROOM_USED_NUM)
    {
        int idx = rand() % 9 + 0;

        while (isContainIdx(usedIdx, idx, ROOM_USED_NUM))
        {
            idx += 1;
            if (idx == ROOM_LIST_NUM)
                idx = 0;
        }

        /*Open the all space at first!!*/
        struct Connect *con = malloc(sizeof(struct Connect) * (ROOM_USED_NUM - 1));
        struct Room *rm = malloc(sizeof(struct Room));

        rm->room_name = room_name_list[idx];
        rm->conNum = 0;
        rm->connect = con;

        if (numRm == 0)
            rm->room_type = "START_ROOM";
        else if (numRm == (ROOM_USED_NUM - 1))
            rm->room_type = "END_ROOM";
        else
            rm->room_type = "MID_ROOM";

        usedIdx[numRm] = idx;
        rmArr[numRm] = *rm;
        numRm += 1;
    }
    free(usedIdx);
}

/*
* idx1 = connecting node
* idx2 = connected node
*/
void setOtherConRoom(struct Room *rmArr, int idx1, int idx2)
{
    rmArr[idx2].connect[rmArr[idx2].conNum].connect_room = &rmArr[idx1];
    rmArr[idx2].connect[rmArr[idx2].conNum].room_idx = idx1;
    rmArr[idx2].conNum += 1;
}

int isConnect(struct Room *rmArr, int idx1, int idx2)
{
    int i;
    for (i = 0; i < rmArr[idx1].conNum; i++)
        if (rmArr[idx1].connect[i].room_idx == idx2)
            return 1;

    return 0;
}

void connectRoom(struct Room *rmArr)
{
    int i, j;

    for (i = 0; i < ROOM_USED_NUM; i++)
    {
        int conNum = rand() % (5 - 3 + 1) + 3;
        /*i= remain how many room need to connect*/
        for (j = rmArr[i].conNum; j < conNum; j++)
        {
            int idx2 = rand() % 6 + 1;
            /*isContainIdx(usedIdx, idx, ROOM_USED_NUM) ||*/
            /*idx2 等於自己*/
            while (idx2 == i || isConnect(rmArr, i, idx2))
            {
                idx2 += 1;
                if (idx2 == ROOM_USED_NUM)
                    idx2 = 0;
            }
            rmArr[i].conNum += 1;
            rmArr[i].connect[j].connect_room = &rmArr[idx2];
            rmArr[i].connect[j].room_idx = idx2;

            setOtherConRoom(rmArr, i, idx2);
        }
    }
} 

/*
*   1. create dir [chengwe.rooms.pid]
*   2. cd to dir 
*   3. create the file and write content
*   4. cd ..
*/
void generateFile(struct Room *rmArr)
{
    pid_t pid = getpid();
    int fileLen = strlen("chengwe") + strlen(".rooms.") + 10;
    char fDir [fileLen];
    sprintf(fDir, "%s.rooms.%d", "chengwe", pid);
    
    mkdir(fDir, 0700);
    chdir(fDir);

    int i, j;
    for(i = 0; i < ROOM_USED_NUM; i++){
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

int main()
{
    struct Room *rmArr = malloc(sizeof(struct Room) * ROOM_USED_NUM);
    srand((unsigned)time(NULL));

    createRooms(rmArr);

    connectRoom(rmArr); 

    generateFile(rmArr);

    free(rmArr);

    return 0;
}
