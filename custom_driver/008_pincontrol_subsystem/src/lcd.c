#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include "lcd.h"
#include "gpio.h"
#include <stdio.h>

/*
 * @brief call this function to make LCD latch the data lines in to its internal registers.
 */
void lcd_enable(void)
{
    gpio_write_value(LCD_PIN_EN, LOW_VALUE);
    usleep(1);
    gpio_write_value(LCD_PIN_EN, HIGH_VALUE);
    usleep(1);
    gpio_write_value(LCD_PIN_EN, LOW_VALUE);
    usleep(100); /*Wait for executing of CMD. the time must be greater than 37 micro seconds*/
}

static void write_4_bits(uint8_t data)
{
    gpio_write_value(LCD_PIN_D4, (data >> 0) & 0x01);
    gpio_write_value(LCD_PIN_D5, (data >> 1) & 0x01);
    gpio_write_value(LCD_PIN_D6, (data >> 2) & 0x01);
    gpio_write_value(LCD_PIN_D7, (data >> 3) & 0x01);
    lcd_enable();
}

void lcd_init(void)
{
    /*configure direction of gpio is out*/
    gpio_inits();

    usleep(40*1000);
    /*RS pin = 0 for LCD command*/
    gpio_write_value(LCD_PIN_RS, LOW_VALUE);
    /*R/W pin = 0 for write*/
    gpio_write_value(LCD_PIN_RW, LOW_VALUE);

    write_4_bits(0x03);
    usleep(5 * 1000);

    write_4_bits(0x03);
    usleep(100);

    write_4_bits(0x03);
    write_4_bits(0x02);
    /*setting mode for LCD*/
    /*4 bit data mode, 2 lines selection , font size 5x8 */
    lcd_send_command(LCD_CMD_4DL_2N_5X8F);

    /* Display ON, Cursor ON */
    lcd_send_command(LCD_CMD_DON_CURON);

    lcd_display_clear();
    /*address auto increment*/
    lcd_send_command(LCD_CMD_INCADD);

}

void lcd_send_command(uint8_t command)
{
    /*RS=0 for LCD command*/
    gpio_write_value(LCD_PIN_RS, LOW_VALUE);

    /*R/W pin = 0 for write*/
    gpio_write_value(LCD_PIN_RW, LOW_VALUE);

    write_4_bits(command >> 4); /*higher nibble*/
    write_4_bits(command); /*lower nibble*/     
}
void lcd_display_clear(void)
{
    lcd_send_command(LCD_CMD_DIS_CLEAR);
    usleep(2000);
}
void lcd_display_return_home(void)
{
    lcd_send_command(LCD_CMD_DIS_RETURN_HOME);
    usleep(2 * 1000);
}

/**
  * @brief  Set Lcd to a specified location given by row and column information
  * @param  Row Number (1 to 2)
  * @param  Column Number (1 to 16) Assuming a 2 X 16 characters display
  */

void lcd_set_cursor(uint8_t row, uint8_t column)
{
    /*Because the cordinate begin at 0*/
    column --;
    switch (row)
    {
    case 1:
        lcd_send_command(DDRAM_FIRST_LINE_BASE_ADDR | column);
        break;
    case 2:
        lcd_send_command(DDRAM_SECOND_LINE_BASE_ADDR | column);
        break;
    default:
        fprintf(stderr, "The row is provied incorrectly: %d.\n",row);
        break;
    }
}

/*
 *This function sends a character to the LCD 
 *Here we used 4 bit parallel data transmission. 
 *First higher nibble of the data will be sent on to the data lines D4,D5,D6,D7
 *Then lower niblle of the data will be set on to the data lines D4,D5,D6,D7
 */
void lcd_print_char(uint8_t ascii_Value)
{
    /*set RS = 1 for data not command*/
    gpio_write_value(LCD_PIN_RS, HIGH_VALUE);
    /*set R/W = 0 for write*/
    gpio_write_value(LCD_PIN_RW, LOW_VALUE);

    write_4_bits(ascii_Value >> 4); /*Higher nibble*/
    write_4_bits(ascii_Value); /*Lower nibble*/
}

void lcd_printf(const char *fmt, ...)
{
    int i;
    uint32_t text_size, letter;
    static char text_buffer[32];
    va_list args;
    va_start(args, fmt);
    text_size = vsprintf(text_buffer, fmt, args);

          // Process the string
    for (i = 0; i < text_size; i++)
    {
        letter = text_buffer[i];

        if (letter == 10)
          break;
        else
        {
          if ((letter > 0x1F) && (letter < 0x80))
              lcd_print_char(letter);
        }
    }
}
void lcd_deinit(void)
{
    lcd_display_clear(); /*Clear display*/
    lcd_display_return_home(); /*Cusor at home position*/
}

void gpio_inits(void)
{
    gpio_configure_dir(LCD_PIN_D4, GPIO_DIR_OUT);
    gpio_configure_dir(LCD_PIN_D5, GPIO_DIR_OUT);
    gpio_configure_dir(LCD_PIN_D6, GPIO_DIR_OUT);
    gpio_configure_dir(LCD_PIN_D7, GPIO_DIR_OUT);
    gpio_configure_dir(LCD_PIN_EN, GPIO_DIR_OUT);
    gpio_configure_dir(LCD_PIN_RS, GPIO_DIR_OUT);
    gpio_configure_dir(LCD_PIN_RW, GPIO_DIR_OUT);

    /*set default value of each pin is low value*/
    gpio_write_value(LCD_PIN_D4, LOW_VALUE);
    gpio_write_value(LCD_PIN_D5, LOW_VALUE);
    gpio_write_value(LCD_PIN_D6, LOW_VALUE);
    gpio_write_value(LCD_PIN_D7, LOW_VALUE);
    gpio_write_value(LCD_PIN_EN, LOW_VALUE);
    gpio_write_value(LCD_PIN_RS, LOW_VALUE);
    gpio_write_value(LCD_PIN_RW, LOW_VALUE);
}