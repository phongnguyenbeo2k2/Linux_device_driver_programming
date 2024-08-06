#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

#define HIGH_VALUE          1
#define LOW_VALUE           0
/*This path is stored attribute of GPIO sysfs*/
#define SYS_GPIO_PATH       "/sys/class/bone_gpios" 
#define GPIO_DIR_OUT        HIGH_VALUE
#define GPIO_DIR_IN         LOW_VALUE
#define GPIO_LOW_VALUE      LOW_VALUE
#define GPIO_HIGH_VALUE     HIGH_VALUE
#define SOME_BYTES          100

/*define function prototype*/
int gpio_configure_dir(char *label, uint8_t dir_value);
int gpio_write_value(char *label, uint8_t out_value);
int gpio_read_value(char *label);

#endif