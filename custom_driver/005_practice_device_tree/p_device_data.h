#ifndef DEVICE_DATA_H
#define DEVICE_DATA_H

typedef struct
{
    int size;
    int perm;
    const char *serial_number;
}device_data_t;

#define RDWR 0x11
#define RDONLY 0x01
#define WRONLY 0x10
#endif