#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

// Function to replace a string with another string
char *replaceWord(char *s, char *oldW, char *newW)
{
    char *result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);

    // Counting the number of times old word
    // occur in the string
    for (i = 0; s[i] != '\0'; i++)
    {
        if (strstr(&s[i], oldW) != &s[i])
            continue;

        cnt++;
        i += oldWlen - 1;
    }

    // Making new string of enough length
    result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1);

    i = 0;
    while (*s)
    {
        // compare the substring with the result
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

void errExit(char *errMsg, int i, char *subStr)
{
    if (strstr(errMsg, "%s") == 0 && subStr != NULL)
        replaceWord(errMsg, "%s", subStr);

    printf(errMsg);
    exit(i);
}

char *readFileData(char *fileName)
{
    fflush(stdout);
    fflush(stdin);
    char *buffer;
    int len = 0;
    FILE *file = fopen(fileName, "r");
    if (file < 0)
        errExit(fileName, 1, NULL);

    //可能很多到時候要改
    fseek(file, 0, SEEK_END);
    len = ftell(file);
    fseek(file, 0, SEEK_SET);
    //printf("%d\n",len);
    if (len > 0)
    {
        buffer = malloc(sizeof(char) * len);
        fread(buffer, 1, len, file);
    }
    printf("%s\n", buffer);

    fclose(file);
    return buffer;
}



// otp_enc plaintext1 myshortkey 57171 > ciphertext1
// Error: key ‘myshortkey’ is too short
int main(int argc, char *argv[])
{
    char *encStr, *pwdStr, *portStr;
    struct sockaddr_in serAddr = {0};

    // ssize_t read;
    encStr = readFileData(argv[1]);
    pwdStr = readFileData(argv[2]);
    portStr = argv[3];

    if (strlen(encStr) > strlen(pwdStr))
        errExit("Error: key ‘%s’ is too short", 1, argv[2]);

    // printf("%s\n", pwd);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        errExit("Error: could not contact otp_enc_d on port %s\n", 2, portStr);
 
    serAddr = gethostbyname("localhost");
    if (serAddr == NULL)
        errExit("Error: could not connect to otp_enc_d\n", 2, NULL); 

    serAddr.sin_family = AF_INET;

    // bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);         
    // serv_addr.sin_port = htons(portno);

  

    

    free(pwdStr);
    free(encStr);
    return 0;
}
