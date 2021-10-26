#include <utmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <ctime>
int main()
{
    struct utmp *p = getutent();

    while (p)
    {
        int epoch = p->ut_tv.tv_sec;
        std::time_t t = (std::time_t)(epoch);
        printf("%s", std::ctime(&t));
    }
}