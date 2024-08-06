#include "gpio.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/*set direction of gpio is either OUT or IN*/
/*
 *  GPIO configure direction
 *  dir_value : 1 means 'out' , 0 means "in"
 */

int gpio_configure_dir(char *label, uint8_t dir_value)
{
    int fd;
    char buff[SOME_BYTES];
    snprintf(buff, sizeof(buff), SYS_GPIO_PATH"%s/direction", label);

    /*open file direction which is attribute of gpio*/
    fd = open(buff, O_WRONLY| O_SYNC);
    if (fd < 0)
    {
        perror("Failure when configure direction open!\n");
        return fd;
    }
    if (dir_value == GPIO_DIR_OUT)
    {
        write(fd,"out", 4);
    }else if (dir_value == GPIO_DIR_IN) 
    {
        write(fd, "in", 3);
    }
    close(fd);
    return 0;
}
/*
 *  GPIO write value
 *  out_value : can be either 0 or 1
 */
int gpio_write_value(char *label, uint8_t out_value)
{
    int fd;
    char buff[SOME_BYTES];
    snprintf(buff,sizeof(buff), SYS_GPIO_PATH"%s/value",label);

    fd = open(buff, O_WRONLY|O_SYNC);
    if (fd < 0)
    {
        perror("Failure when gpio write value open!\n");
        return fd;
    }
    if (out_value == HIGH_VALUE)
    {
        write(fd, "1", 2);
    }else if (out_value == LOW_VALUE)
    {
        write(fd, "0", 2);
    }
    close(fd);
    return 0;    
}
/*
 *  GPIO read value
 */
int gpio_read_value(char *label)
{
    int fd;
    char buff[SOME_BYTES];
    uint8_t read_value = 0;
    snprintf(buff,sizeof(buff), SYS_GPIO_PATH"%s/value",label);
    fd = open(buff, O_RDONLY | O_SYNC);
    if (fd < 0)
    {
        perror("Failure when gpio read value open!\n");
        return fd;        
    }
    read(fd, &read_value, 1);
    read_value = atoi(&read_value);
    close(fd);
    return read_value;
}