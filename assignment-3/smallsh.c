#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>


/*  using  "signal" function to catch the interupt cmd 
 *  		=> need to use WIFEXITED and WIFSIGNALED two function to check pid
 *  fflush function for stdout => some part buffer cannot out put as usual
 *	dup2	=> stdout file;
 *	fork ,waitpid => for child process
 *	
 * */

/* Setting file for stdin and stdout*/
char* INPUT;
char* OUTPUT;
/* For input argument with $$ to replace pid*/
char* tmpStr;
/* Shell status*/
int _shellStatus;
/* ignored run in backgroud cmd */
int _isForeground = 0;

/* Free the malloc memory*/
void freeMalloc()
{
    if(INPUT != NULL)
        free(INPUT);
    if(OUTPUT != NULL)
        free(OUTPUT);
}

/* Function: replace sub string into new string
 * Cite from: https://www.geeksforgeeks.org/c-program-replace-word-text-another-given-word/
 * s      => original string
 * oldW   => replaced substring
 * newW   => placed substring 
 * return => the new string*/
char *replaceWord(const char *s, const char *oldW,const char *newW)
{
    char *result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);
    /*Loop string until the end*/
    for (i = 0; s[i] != '\0'; i++)
    {
        /*Count the how many place need to be changed*/
        if (strstr(&s[i], oldW) == &s[i])
        {
            cnt++;
            i += oldWlen - 1;
        }
    }

    result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1);

    i = 0;
    /*Loop string until the end*/
    while (*s)
    {
        /*Replacing the new word*/
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

/* Function : split the line into arguments
 * line     => read line 
 * isBackRun=> 0 running child process in the background
 * return   => arguments array*/
char** splitInput(char *line, int* isBackRun)
{
    INPUT = NULL;
    OUTPUT = NULL;
    char* delim = " \n";
    int argNum = 512, idx = 0;
    char** argArr = malloc(sizeof(char*) * 512);
    char* token;

    token = strtok(line, delim);
    while(token){

        /* < => stdin data(a file)*/
        if(strcmp(token, "<") == 0)
        {   
            token = strtok(NULL, delim);
            if(token == NULL)
                return argArr;

            INPUT = malloc(sizeof(char) * strlen(token));
            strcpy(INPUT, token);

            token = strtok(NULL, delim);
        }
        /* > => stdout data(a file)*/
        else if(strcmp(token, ">") == 0)
        {
            token = strtok(NULL, delim);
            if(token == NULL)
                return argArr;

            OUTPUT = malloc(sizeof(char) * strlen(token));
            strcpy(OUTPUT, token);

            token = strtok(NULL, delim);
        }
        /* & => set child program running in background*/
        /*  special echo if => for last checking, this maybe why excvp need to design first two input arg */
        else if(strcmp(token, "&") == 0 && strcmp( argArr[0], "echo") != 0)
        {
            if(_isForeground == 0)
                *isBackRun = 1;
            token = strtok(NULL, delim);
        }
        /* $$ => replace into smallsh pid number*/
        else if(strstr(token, "$$") != 0)
        {
            pid_t p = getpid();
            sprintf(tmpStr, "%d", p);

            /*Replace sub string "$$" into PID number*/
            argArr[idx] = replaceWord(token,"$$",tmpStr);
            idx+=1;
            token = strtok(NULL, delim);
        }
        /*Regular argument*/
        else
        {
            argArr[idx] = token;
            idx+=1;
            token = strtok(NULL, delim);
        }
    };

    
    /*printf("INPUT %s\n",INPUT);
    printf("OUTPUT %s\n", OUTPUT);
    int i;
    for(i = 0 ;i < 4; i++)
        printf("argArr[%d]: %s\n", i , argArr[i]);    */
    return argArr;
}

/*Function: End the shell and Free the malloc memory */
void exitCmd(char** argArr, char* line)
{
    free(argArr);
    free(line);
    exit(0);
}

/*Function: cd to the input place or to the HOME*/
void cdCmd(char** argArr)
{
    if(argArr[2])
    {
        printf("cd - Too many arguments.");
        return;
    }

    char* dir = argArr[1];

    /*No argument than cd home path */
    if(!dir)
        dir = getenv("HOME");

    if(chdir(dir) == -1)
        perror("chdir");
}

/*Get the last time status*/
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

/* Function: create file(stdout) or read the file(stdin)
 * fileName => File name
 * io       => Stdin or stdout*/
void exeStdFile(char* fileName, FILE* io)
{
    int file = 0;
    if( io == stdout)
        file = open(fileName,O_WRONLY | O_CREAT, 0744);
    else
    {
        file = open(fileName, O_RDONLY);
        if( file == -1)
        {
            perror(fileName);
            exit(1);
        }
    }
    /*replace standard out/in with out/in file*/
    dup2(file,fileno(io));
    close(file);
}

/*Function: fork child process
 * argArr   => [1] = Command;  [2~] = argument values
 * isBackRum=> True: run child process in the back ground*/
void exeOthers(char** argArr, int* isBackRun)
{
    pid_t pid, wpid;

    pid = fork(); 
    if (pid == 0)
    {
        /*If have input or output file*/
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

/* Function: exe cmd, Spec command("cd", "exit", "status")
 * argArr   => [1] = command
 * isBackRun=> True running child process in the back ground
 * line     => if exit than for free*/
void exeCmd(char** argArr, int* isBackRun, char* line)
{
    /*if dont have input or catch "#" comment word => return*/
    if(!argArr[0] || strstr(argArr[0],"#") != 0)
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

/*Function: finding defunct process id and release*/
void traceDefunctPid()
{
    /*WNOHANG : find the defunct child process id*/
    pid_t pid = waitpid(-1, &_shellStatus, WNOHANG);
    if(pid == -1 || pid == 0)
        return;

    printf("background pid %d is done: ", pid);
    statusCmd();
}

/*Function: loop for get the command line*/
void shellLoop()
{
    char *line = NULL, **argArr;
    size_t len = 0;
    ssize_t line_len;
    int i, isBackRun;
    /* For the string with $$*/
    tmpStr = malloc(sizeof(char) * 10);

    do
    {
        isBackRun = 0;
        printf(":");

        line_len = getline(&line, &len, stdin);
        /*Clean the buffer*/
        fflush(stdout);
        argArr = splitInput(line, &isBackRun);
        exeCmd(argArr, &isBackRun, line);
        /*If the backgroud process finished print finished info*/
        traceDefunctPid();
       
        freeMalloc();
    }while(1);
}

/*Function: end the running child process
 * child process will recieve signal too*/
void trapInterrupt(int signal)
{
    printf("terminated by signal %d\n", WTERMSIG(signal)); 
}

/* Function: the child process can||cannot run in the process */
void changeMode(int sigNum)
{
    signal(SIGTSTP, changeMode);
    if(_isForeground)
    {
        _isForeground = 0;
        printf("Exiting foreground-only mode\n:");
    }
    else
    {
        _isForeground = 1;
        printf("Entering foreground-only mode (& is now ignored)\n:");
    }
    fflush(stdout);
}

int main() 
{
    /*Trap the signal
     * SIGINT = ctr+c
     * SIGTSTP= ctr+z*/
    signal(SIGINT, trapInterrupt);
    signal(SIGTSTP, changeMode);

    shellLoop();
    return 0;
}
