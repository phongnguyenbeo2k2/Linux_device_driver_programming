#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <fcntl.h>

#define NUMBER_BYTES_READ 10
char buffer[2048];

int main (int argc, char* argv[])
{
    int fd;
    /*the remaining variable hold the rest of bytes that need to read*/

    int remaining = NUMBER_BYTES_READ;
    int total_read = 0;
    int n = 0;
    int ret = 0;

    if (argc < 2)
    {
        printf ("You have entered wrong!\n");
    }
    remaining = atoi(argv[1]);

    printf ("The number of bytes need to read is %d\n",remaining);
    fd = open("/dev/pcd_device_file-3",O_RDONLY);

    if (fd < 0)
    {
        perror ("open");
        return fd;
    }

    printf ("open file successfully\n");

    while (n!=2 && remaining)
    {
        ret = read(fd,&buffer,NUMBER_BYTES_READ);
        if (ret == 0)
        {
            printf ("You is at the end of file\n");
        }else if (ret <= remaining)
        {
            total_read += ret;
            remaining -= ret;
        }else if (ret < 0){
            printf ("Can't read from the file\n");
            break;
        }else break;
        n ++;
    }
    
    printf ("the number of bytes that program read: %d\n",total_read);

    printf ("the content read is : ");

    for (int i = 0; i <total_read; i++)
    {
        printf ("%c",buffer[i]);
    }
    printf ("\n");

    close(fd);
    return 0;
    
}