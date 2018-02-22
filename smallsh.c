#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

char* INPUT;
char* OUTPUT;
char* tmpStr;
int _shellStatus;
int _isForeground = 0;

void freeMalloc()
{
    if(INPUT != NULL)
        free(INPUT);
    if(OUTPUT != NULL)
        free(OUTPUT);
}

char *replaceWord(const char *s, const char *oldW,const char *newW)
{
    char *result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);
    for (i = 0; s[i] != '\0'; i++)
    {
        if (strstr(&s[i], oldW) == &s[i])
        {
            cnt++;
            i += oldWlen - 1;
        }
    }
    result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1);

    i = 0;
    while (*s)
    {
        if (strstr(s, oldW) == s)
        {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }

    result[i] = '\0';
    return result;
}

char** splitInput(char *line, int* isBackRun )
{
    INPUT = NULL;
    OUTPUT = NULL;
    char* delim = " \n";
    int argNum = 512, idx = 0;
    char** argArr = malloc(sizeof(char*) * 512);
    char* token;

    token = strtok(line, delim);
    while(token){

        if(strcmp(token, "<") == 0)
        {   
            token = strtok(NULL, delim);
            if(token == NULL)
                return argArr;

            INPUT = malloc(sizeof(char) * strlen(token));
            strcpy(INPUT, token);

            token = strtok(NULL, delim);
        }
        else if(strcmp(token, ">") == 0)
        {
            token = strtok(NULL, delim);
            if(token == NULL)
                return argArr;

            OUTPUT = malloc(sizeof(char) * strlen(token));
            strcpy(OUTPUT, token);

            token = strtok(NULL, delim);
        }
        else if(strcmp(token, "&") == 0)
        {
            if(_isForeground == 0)
                *isBackRun = 1;
            token = strtok(NULL, delim);
        }
        else if(strstr(token, "$$") != 0)
        {
            pid_t p = getpid();
            sprintf(tmpStr, "%d", p);

            argArr[idx] = replaceWord(token,"$$",tmpStr);
//            argArr[idx] = tmpStr;
            idx+=1;
            token = strtok(NULL, delim);
        }
        else
        {
            argArr[idx] = token;
            idx+=1;
            token = strtok(NULL, delim);
        }
    };

    /*    printf("INPUT %s\n",INPUT);
          printf("OUTPUT %s\n", OUTPUT);
          int i;
          for(i = 0 ;i < 4; i++)
          printf("argArr[%d]: %s\n", i , argArr[i]);
          */
    return argArr;
}


void exitCmd(char** argArr, char* line)
{
    freeMalloc();
    free(argArr);
    free(line);
    exit(0);
}

void cdCmd(char** argArr)
{
    if(argArr[2])
    {
        printf("cd - Too many arguments.");
        return;
    }

    char* dir = argArr[1];

    if(!dir)
        dir = getenv("HOME");

    if(chdir(dir) == -1)
        perror("chdir");
}

void statusCmd()
{
    //If the child exited, print its status
    if (WIFEXITED(_shellStatus))
        printf("exit value %d\n", WEXITSTATUS(_shellStatus));
    // If it was signalled, print the signal it received.
    if (WIFSIGNALED(_shellStatus) && WSTOPSIG(_shellStatus) != 0)
        printf("stop signal %d\n", WSTOPSIG(_shellStatus));

    // If it was terminated, print the termination signal.
    if (WTERMSIG(_shellStatus))
        printf("terminated by signal %d\n",WTERMSIG(_shellStatus));
}


void exeStdFile(char* fileName, FILE* io)
{
    int file = 0;
    if( io == stdout)
    {
        file = open(fileName,O_WRONLY | O_CREAT, 0744);
    }
    else
    {
        file = open(fileName, O_RDONLY);
        if( file == -1)
        {
            perror(fileName);
            exit(1);
        }
    }
    dup2(file,fileno(io));
    close(file);
}

void exeOthers(char** argArr, int* isBackRun)
{
    pid_t pid, wpid;

    pid = fork(); 
    if (pid == 0)
    {
        if(INPUT != NULL)
            exeStdFile(INPUT, stdin);


        if(OUTPUT != NULL)
            exeStdFile(OUTPUT, stdout);

        fflush(stdout);

        execvp(argArr[0],argArr);
        perror(argArr[0]);
        free(argArr);
        exit(1);
    }
    else if(pid < 0)
    {
        perror("Fork");
    }
    else
    {
        if(*isBackRun)
            printf("background pid is %d\n", pid);       

        //WUNTRACED => a child is stoppted(exit or ctrl+C ) then return
        //Return a nonzero value if child terminated nomally
        //WIFSIGNALED => Return a nonzero value if child terminated by signal       
        else
        {
            do{
                wpid = waitpid(pid, &_shellStatus, WUNTRACED);
            }while(!WIFEXITED(_shellStatus) && !WIFSIGNALED(_shellStatus));
        }
    }
}

void exeCmd(char** argArr, int* isBackRun, char* line)
{
    if(!argArr[0] || strcmp(argArr[0],"#") == 0)
        return;

    char* cmd = argArr[0];

    if(strcmp(cmd,"exit") == 0)
        exitCmd(argArr, line);
    else if(strcmp(cmd,"cd") == 0)
        cdCmd(argArr);
    else if(strcmp(cmd,"status") == 0)
        statusCmd();
    else
        exeOthers(argArr, isBackRun);
}

void traceDefunctPid()
{
    pid_t pid = waitpid(-1, &_shellStatus, WNOHANG);
    if(pid == -1 || pid == 0)
        return;

    printf("background pid %d is done: ", pid);
    statusCmd();
}


void shellLoop()
{
    char *line = NULL, **argArr;
    size_t len = 0;
    ssize_t line_len;
    int t, isBackRun;
    tmpStr = malloc(sizeof(char) * 10);

    do
    {
        isBackRun = 0;
        printf(":");

        line_len = getline(&line, &len, stdin);
        argArr = splitInput(line, &isBackRun);
        exeCmd(argArr, &isBackRun, line);
        traceDefunctPid();

        freeMalloc();
    }while(1);
}

void trapInterrupt(int signal)
{
    pid_t pid = waitpid(-1, &_shellStatus, WUNTRACED);
    if( pid == 0 || pid == -1)
        return;

    kill(pid, SIGKILL);
    printf("terminated by signal %d\n", WTERMSIG(signal)); 
}

void changeMode(int sigNum)
{
    signal(SIGTSTP, changeMode);
    if(_isForeground)
    {
        _isForeground = 0;
        printf("\nExiting foreground-only mode\n:");
    }
    else
    {
        _isForeground = 1;
        printf("\nEntering foreground-only mode (& is now ignored)\n:");
    }
    fflush(stdout);
}

int main() 
{
  //  signal(SIGINT, trapInterrupt);
    signal(SIGTSTP, changeMode);
    shellLoop();
    return 0;
}
