#include <stdio.h>
#include "lcd.h"
#include "gpio.h"
#include <unistd.h>

int main(void)
{
    lcd_init();
    sleep(1);
    lcd_print_char('A');
    return 0;
}
