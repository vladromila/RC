#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/uio.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>

#define FIFO_NAME "MyTest_FIFO"

int main()
{
  int sockets[2], child;
  char s[1024];

  socketpair(AF_UNIX, SOCK_STREAM, 0, sockets);
  child = fork();

  int fd = open("fifo.txt", O_WRONLY);
  int quit = 0;

  mknod("fifor.txt", S_IFIFO | 0666, 0);

  int rfd = open("fifor.txt", O_RDONLY);

  if (child)
  { // parent

    printf("Client command: ");
    while (quit == 0 && fgets(s, 300, stdin))
    {
      if (strcmp(s, "quit\n") == 0)
      {
        quit = 1;
        write(sockets[1], "quit\0", 6);
        write(fd, s, strlen(s));
        return 0;
      }
      else if (quit == 0)
      {
        printf("Client Command: ");
        write(fd, s, strlen(s));

        char buff[1024];
        int len = read(rfd, buff,1024);
        write(1, buff, len);
      }
    }
  }
  else
  {                   
    close(sockets[1]); 

    read(sockets[0], s, 1024);
    if (strcmp(s, "quit\0") == 0)
      printf("Thank you for using the app.\n");

    close(sockets[0]);
  }
}

//  printf("Client Command: ");
//         write(fd, s, strlen(s));

//         int nr = 0;

//         char buff[1024];
//         printf("%c",buff[0]);
//         if (isdigit(buff[0]))
//           while (buff[0] != '~')
//           {
//             nr = nr * 10 + (int)buff[0];
//             read(rfd, buff, 1);
//           }
//         else
//           nr = 1024;
//         int len = read(rfd, buff, nr);
//         write(1, buff, len);