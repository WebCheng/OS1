#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

int StrToInt(char *str)
{
    int rt = 0;

    /*ASCII to count*/
    int i;
    for (i = 0; i < strlen(str); i++)
        rt = rt * 10 + (str[i] - '0');

    return rt;
}

int main(int argc, char *argv[])
{
    /*Err input number*/
    if (argc != 2)
        return 1;

    /*Randon seed*/
    srand((unsigned)time(NULL));

    /*Rand pwk*/
    int i;
    for (i = 0; i < StrToInt(argv[1]); i++)
    {
        int x = rand() % (90 - 64 + 1) + 64;
        printf("%c", x == 64 ? ' ': x);
    }

    // printf("\n");
    return 0;
}
