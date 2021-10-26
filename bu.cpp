

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <utmp.h>
#include <ctime>
#define READ 0
#define WRITE 1
char username[50];

int preventDoubleQuit = 0;

void login(char name[], int write_fd)
{
    strcpy(name, name + 2);

    for (int i = 0; i < strlen(name); i++)
    {
        if (name[i] == '\n')
        {
            name[i] = '\0';
            break;
        }
        name[i] = tolower(name[i]);
    }

    char row[256];
    bool userExists = false;
    FILE *f = fopen("users.txt", "r");
    while (fgets(row, 256, f))
    {
        row[strlen(row) - 1] = '\0';
        if (strcmp(row, name) == 0)
        {
            userExists = true;
        }
    }
    if (userExists == true)
    {
        strcpy(username, name);
        char toSendResponse[125];
        strcpy(toSendResponse, "You have been logged in as ");
        strcat(toSendResponse, name);
        strcat(toSendResponse, ".\n");
        write(write_fd, toSendResponse, strlen(toSendResponse));
    }
    else
        write(write_fd, "This username does not exist.\n", 31);
}

void checkLogin(int write_fd)
{
    if (username[0] == '\0')
    {
        write(write_fd, "You are not logged in.\n", 23);
    }
    else
    {
        char toSendResponse[125];
        strcpy(toSendResponse, "You are logged in as ");
        strcat(toSendResponse, username);
        strcat(toSendResponse, ".\n");
        write(write_fd, toSendResponse, strlen(toSendResponse));
    }
}

void logout(int write_fd)
{
    strcpy(username, "\0");
    write(write_fd, "You have been logged out!\n", 27);
}

void getLoggedUsers(int write_fd)
{
    if (username[0] != '\0')
    {
        char toSendResponse[500];
        struct utmp *p = getutent();
        strcat(toSendResponse, "\n\n");

        while (p)
        {
            int epoch = p->ut_tv.tv_sec;
            std::time_t t = (std::time_t)(epoch);

            strcat(toSendResponse, "Username: ");
            if (strlen(p->ut_user) == 0)
                strcat(toSendResponse, "undefined");
            else
                strcat(toSendResponse, p->ut_user);
            strcat(toSendResponse, "\n");
            strcat(toSendResponse, "Hostname: ");
            if (strlen(p->ut_host) == 0)
                strcat(toSendResponse, "undefined");
            else
                strcat(toSendResponse, p->ut_host);
            strcat(toSendResponse, "\n");
            strcat(toSendResponse, "Entry date: ");
            strcat(toSendResponse, std::ctime(&t));
            strcat(toSendResponse, "\n\n");
            p = getutent();
        }

        write(write_fd, toSendResponse, strlen(toSendResponse));
    }
    else
    {
        write(write_fd, "ERROR: You need to be authenticated in order to run this command.\n", 67);
    }
}
void getProcInfo(char procNr[], int write_fd)
{
    if (username[0] != '\0')
    {
        strcpy(procNr, procNr + 2);
        for (int i = 0; i < strlen(procNr); i++)
            if (procNr[i] == '\n')
            {
                procNr[i] = '\0';
                break;
            }

        char path[150];
        strcpy(path, "/proc/");
        strcat(path, procNr);
        strcat(path, "/status");

        int descriptor = open(path, O_RDONLY);
        char content[100];
        int len = read(descriptor, content, 100);
        if (len != -1)
        {
            char row[256];
            char toSendMessage[600];
            strcpy(toSendMessage, "\n\n");
            FILE *f = fopen(path, "r");
            while (fgets(row, 256, f))
            {
                if (strstr(row, "Name:") || strstr(row, "State:") != NULL || strstr(row, "PPid:") != NULL || strstr(row, "Uid:") != NULL || strstr(row, "VmSize:") != NULL)
                {
                    strcat(toSendMessage, row);
                }
            }
            printf("%ld", strlen(toSendMessage));
            strcat(toSendMessage, "\n\0");
            write(write_fd, toSendMessage, strlen(toSendMessage));
        }
        else
        {
            write(write_fd, "The entered process does not seem to be running\n", 49);
        }
    }
    else
    {
        write(write_fd, "ERROR: You need to be authenticated in order to run this command.\n", 67);
    }
}
void quit(int write_fd)
{
    char toSendResponse[125];
    if (preventDoubleQuit == 0)
    {
        preventDoubleQuit = 1;
        if (username[0] != '\0')
        {
            strcpy(toSendResponse, "The user ");
            strcat(toSendResponse, username);
            strcat(toSendResponse, " has been logged out of the server because he has quit the client app.\n");
            strcpy(username, "\0");
        }
        else
        {
            strcpy(toSendResponse, "An anonymous connection has been terminated.\n");
        }

        write(1, toSendResponse, strlen(toSendResponse));
    }
    else
        preventDoubleQuit = 0;
}
void invalidCommand(int write_fd)
{
    write(write_fd, "Invalid command\n\0", 18);
}
char *detectFunction(char x[], char functionType[])
{
    for (int i = 0; i < strlen(x); i++)
        if (x[i] == '\n')
        {
            x[i] = '\0';
            break;
        }
    char *p = strtok(x, " ");
    for (int i = 0; i < strlen(p); i++)
        p[i] = tolower(p[i]);

    strcpy(functionType, "0");
    if (strcmp(p, "get-logged-users") == 0)
    {
        strcpy(functionType, "2");
    }
    else if (strcmp(p, "login") == 0)
    {
        p = strtok(NULL, " ");
        if (p)
        {
            strcpy(functionType, "1 ");
            strcat(functionType, p);
            strcat(functionType, "\n\0");
        }
        else
            strcpy(functionType, "e1");
    }
    else if (strcmp(p, "get-proc-info") == 0)
    {
        p = strtok(NULL, " ");
        if (p)
        {
            strcpy(functionType, "3 ");
            strcat(functionType, p);
            strcat(functionType, "\n\0");
        }
        else
            strcpy(functionType, "e2");
    }
    else if (strcmp(p, "logout") == 0)
    {
        strcpy(functionType, "4");
    }
    else if (strcmp(p, "quit") == 0)
    {
        strcpy(functionType, "5");
    }
    else if (strcmp(p, "check-login") == 0)
    {
        strcpy(functionType, "6");
    }

    return functionType;
}

void process_parent(int write_fd, int read_fd)
{

    while (1)
    {

        mknod("fifo.txt", S_IFIFO | 0666, 0);

        int fd = open("fifo.txt", O_RDONLY);

        char buff[100];
        int len = read(fd, buff, 100);

        char functionType[25];
        detectFunction(buff, functionType);
        write(write_fd, functionType, strlen(functionType));
    }
}

void process_child(int write_fd, int read_fd)
{

    while (1)
    {
        char buff[256];
        int len;

        int wfd = open("fifor.txt", O_WRONLY);

        len = read(read_fd, buff, 256);

        if (buff[0] == '2')
        {
            getLoggedUsers(wfd);
        }
        else if (buff[0] == '1' || buff[0] == '3')
        {
            if (buff[0] == '1')
            {
                login(buff, wfd);
            }
            else
            {
                getProcInfo(buff, wfd);
            }
        }
        else if (buff[0] == '4')
        {
            logout(wfd);
        }
        else if (buff[0] == '5')
        {
            quit(wfd);
        }
        else if (buff[0] == '6')
        {
            checkLogin(wfd);
        }
        else if (buff[0] == 'e')
        {
            if (buff[1] == '1')
                write(wfd, "The login command requires a second parameter. (username)\n", 59);
            else if (buff[1] == '2')
                write(wfd, "The get-proc-info command requires a second parameter. (Pid)\n", 62);
        }
        else
            invalidCommand(wfd);
    }
}

int main()
{
    int pipe_fds[2];
    int pipe_fdp[2];
    int len;
    strcpy(username, "\0");
    pipe(pipe_fds);
    pipe(pipe_fdp);

    pid_t pid = fork();

    if (pid == -1)
    {
        printf("ERROR\n");
    }

    if (pid > 0)
    { //parent
        close(pipe_fds[READ]);
        close(pipe_fdp[WRITE]);

        process_parent(pipe_fds[WRITE], pipe_fdp[READ]);
    }

    if (pid == 0)
    { // child
        close(pipe_fds[WRITE]);
        close(pipe_fdp[READ]);

        process_child(pipe_fdp[WRITE], pipe_fds[READ]);
    }
}