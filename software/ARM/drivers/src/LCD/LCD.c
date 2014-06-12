/***************************************************************************
 *                                                                         *
 *  FILE: LCD106x56.C                                                     *
 *   LCD Display Controller Interface Routines for use with Tian-ma        *
 *   106x56 Graphics module with onboard S6B0724X01-B0CY controller        *
 *                                                                         *
 *   Copyright (C) 2003 by Carousel Design Solutions                       *
 *                                                                         *
 *         Written by:                            *
 *         Michael J. Karas                       *
 *         Carousel Design Solutions              *
 *         4217 Grimes Ave South                  *
 *         Edina MN 55416                         *
 *         (952) 929-7537                         *
 *                                                                         *
 ***************************************************************************/

#include "LPC11Uxx.h"			/* LPC11xx Peripheral Registers */
#include "stddef.h"
#include "stdbool.h"

#include "LCD/LCD.h"
#include "fonts.h"

#include "GPIO/gpio.h"
#include "SSP/ssp.h"
#include "config.h"
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include <cr_section_macros.h>

/* pixel level bit masks for display */
/* this array is setup to map the order */
/* of bits in a byte to the vertical order */
/* of bits at the LCD controller */
const uint8_t l_mask_array[8] =
    {
        0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
/* the LCD display image memory */
/* buffer arranged so page memory is sequential in RAM */
__BSS(RAM3) uint8_t l_display_array[Y_BYTES][X_BYTES];

/*
**
** Low level LCD Controller Interface Support Routines
** The LCD Controller interfaces to the microcontroller
** using the connections as shown below. 
**
**  P3^0  LCD Controller Reset (RST/) signal
**  P3^1  LCD Controller Chip Select (CS/) signal
**  P3^2  LCD Controller Ctl/Data Select (C/D) signal
**  P3^2  LCD Controller Serial Clcok (SCLK) signal
**  P3^3  LCD Controller Serial Data (SDAT) signal
**
**
*/

FONT_DEF * getFontData(uint8_t font)
{
    return (FONT_DEF *)&g_asFonts[font];
}


static void lcd_command_send(uint8_t command)
{
	GPIOSetBitValue(PIN_LCD_CD,0);
	SSP_Send( LCD_SPI_PORT, &command, 1 ,0);
}
static void lcd_data_send(uint8_t * buffer, uint8_t len)
{
	GPIOSetBitValue(PIN_LCD_CD,1);
	SSP_Send( LCD_SPI_PORT, buffer, len ,0);
}

/*
** 
** routine to initialize the operation of the LCD display subsystem
**
*/

void lcd_init(void)
{
    int i = 0;
    uint8_t const data[] =
                   { LCD_SET_ADC_REV,     /* set ADC reverse */
                     LCD_SET_SHL_REV,     /* set SHL reverse */
                     LCD_SET_BIAS_0,      /* set for the low bias mode */
                     LCD_REF_VOLT_MODE,   /* prime for the reference voltage */
                     LCD_REF_VOLT_REG+0x28, /* set default ref. voltage select */
                     LCD_REG_RESISTOR+3,  /* set default resistor ratio */
                     //LCD_PWR_CTL+4,       /* turn on the VC bit */
                     //0xff,
                     //LCD_PWR_CTL+6,       /* now turn on VC+VR bits */
                     0xff,
                     LCD_PWR_CTL+7,       /* now turn on the VC+VR+VF */
                     0xff,
                     };
    uint8_t const on[] =
                   { 
                     LCD_DISP_ON,         /* put the display on */
                     LCD_SET_LINE+0,     /* set line for row 0 of display */
                     LCD_SET_PAGE+0,      /* set page 0 */
                     LCD_SET_COL_HI+0,    /* set column 0 */
                     LCD_SET_COL_LO+0,
                     };
 
    LPC_IOCON->TDI_PIO0_11 = 0x91;
    GPIOSetDir( PIN_LCD_RESET, 1 );
    GPIOSetDir( PIN_LCD_CD, 1 );
    GPIOSetDir( PIN_LCD_LED, 1 );

    /* reset the LCD controller chip */
    vTaskDelay(20);
    GPIOSetBitValue(PIN_LCD_RESET,0);
    vTaskDelay(20);
    GPIOSetBitValue(PIN_LCD_RESET,1);        /* release reset to back high */
    vTaskDelay(20);
    for (i=0;i < (sizeof(data)/sizeof(data[0]));i++)
    {
        /* program the controller operational state */
        if (data[i]==0xff)
        	vTaskDelay(40);
        else
        	lcd_command_send(data[i]);
    }
    vTaskDelay(40);
    for (i=0;i < (sizeof(on)/sizeof(on[0]));i++)
    {
    	lcd_command_send(on[i]);
    }
    lcd_erase();

}


/*
**
** routine to erase the LCD screen, This erases whole
** display memory of the S6B0724 LCD controller.
**
*/
void lcd_erase(void)
{
    uint8_t p, index;
    uint8_t cmd[3];

    memset(l_display_array,sizeof(l_display_array),0);
    cmd[1] = LCD_SET_COL_HI + 0;
    cmd[2] = LCD_SET_COL_LO + 0;
    for(p=0; p<8; p++)
    {
        cmd[0] = LCD_SET_PAGE + p;
        for(index=0; index< sizeof(cmd)/sizeof(cmd[0]) ; index++)
        {
        	lcd_command_send(cmd[index]);
        }
        lcd_data_send( l_display_array[0], X_BYTES);
    }
}

/*
**
** routine to display a test pattern on the LCD screen,
**
*/
uint8_t testpat[4][8]={
                                {0x0F,0x0F,0x0F,0x0F,0xF0,0xF0,0xF0,0xF0},
                                {0xF0,0xF0,0xF0,0xF0,0x0F,0x0F,0x0F,0x0F},
                                {0xFF,0x81,0xBD,0xBD,0xBD,0xBD,0x81,0xFF},
                                {0x00,0x7E,0x42,0x42,0x42,0x42,0x7E,0x00}
                            };
void lcd_test(uint8_t pattern)
{
    uint8_t p;
    uint8_t index;
    uint8_t cmd[3];

    cmd[1] = LCD_SET_COL_HI + 0/16; /* set column 0 */
    cmd[2] = LCD_SET_COL_LO + 0%16;
    for(p=0; p<8; p++)
    {
        cmd[0] = LCD_SET_PAGE + p; /* set page */
        for(index=0; index< sizeof(cmd)/sizeof(cmd[0]) ; index++)
        {
        	lcd_command_send(cmd[index]);
        }
        for(index=0; index< 128; index++)
        {
        	lcd_data_send(&testpat[pattern][index%8],1);
        }
    }
}


void lcd_clear_all(void)
{
    uint8_t i,j;

    for (i=0;i<Y_BYTES;i++)
        for (j=0;j<X_BYTES;j++)
            l_display_array[i][j] = 0;
}
/*
**
**  Clears the display memory starting at the left/top  and going to
**  the right/bottom . No runtime error checking is performed. It is 
**  assumed that left is less than right and that top is less than 
**  bottom
**
*/
void lcd_clear_area(uint8_t left,  uint8_t top,
                    uint8_t right, uint8_t bottom)
{
    uint8_t bit_pos;
    uint8_t x;
    uint8_t byte_offset;
    uint8_t y_bits;
    uint8_t remaining_bits;
    uint8_t mask;


    bit_pos = top & 0x07;     /* get starting bit offset into byte */

    for(x = left; x <= right; x++)
    {
        byte_offset = top >> 3;    /* get byte offset into y direction */
        y_bits = (bottom - top) + 1;  /* get length in the y direction to write */
        remaining_bits = 8 - bit_pos;  /* number of bits left in byte */
        mask = l_mask_array[bit_pos];  /* get mask for this bit */

        while(y_bits)      /* while there are still bits to write */
        {
            if((remaining_bits == 8) && (y_bits > 7))
            {
                /* here if we are byte aligned and have at least 1 byte to write */
                /* do the entire byte at once instead of bit by bit */
                while(y_bits > 7)   /* while there are at least 8 more bits to do */
                {
                    l_display_array[byte_offset][x] = 0x00;
                    byte_offset++;
                    y_bits -= 8;
                }
            }
            else
            {
                /* here if not byte aligned or an entire byte does not need written */
                /* thus do bit by bit */
                l_display_array[byte_offset][x] &= ~mask;
                if(l_mask_array[0] & 0x80)
                {
                    mask >>= 1;
                }
                else
                {
                    mask <<= 1;
                }
                y_bits--;
                remaining_bits--;
                if(remaining_bits == 0)
                {
                    /* might have bust gotton byte aligned */
                    /* so reset for beginning of a byte */
                    remaining_bits = 8;
                    byte_offset++;
                    mask = l_mask_array[0];
                }
            }
        }
    }
}

/*
**
** Inverts the display memory starting at the left/top and going to
** the right/bottom. No runtime error checking is performed. It is 
** assumed that left is less than right and that top is less than 
** bottom 
** 
*/

void lcd_invert_area(uint8_t left,  uint8_t top,
                     uint8_t right, uint8_t bottom)
{
    uint8_t bit_pos;
    uint8_t x;
    uint8_t byte_offset;
    uint8_t y_bits;
    uint8_t remaining_bits;
    uint8_t mask;


    bit_pos = top & 0x07;     /* get starting bit offset into byte */

    for(x = left; x <= right; x++)
    {
        byte_offset = top >> 3;    /* get byte offset into y direction */
        y_bits = (bottom - top) + 1;  /* get length in the x direction to write */
        remaining_bits = 8 - bit_pos;  /* number of bits left in byte */
        mask = l_mask_array[bit_pos];  /* get mask for this bit */

        while(y_bits)      /* while there are still bits to write */
        {
            if((remaining_bits == 8) && (y_bits > 7))
            {
                /* here if we are byte aligned and have at least 1 byte to write */
                /* do the entire byte at once instead of bit by bit */
                while(y_bits > 7)   /* while there are at least 8 more bits to do */
                {
                    l_display_array[byte_offset][x] ^= 0xFF;
                    byte_offset++;
                    y_bits -= 8;
                }
            }
            else
            {
                /* here if not byte aligned or an entire byte does not need written */
                /* thus do bit by bit */
                l_display_array[byte_offset][x] ^= mask;
                if(l_mask_array[0] & 0x80)
                {
                    mask >>= 1;
                }
                else
                {
                    mask <<= 1;
                }
                y_bits--;
                remaining_bits--;
                if(remaining_bits == 0)
                {
                    /* might have bust gotton byte aligned */
                    /* so reset for beginning of a byte */
                    remaining_bits = 8;
                    byte_offset++;
                    mask = l_mask_array[0];
                }
            }
        }
    }
}

/*
**
** Draws a line into the display memory starting at left going to
** right, on the given row. No runtime error checking is performed.  
** It is assumed that left is less than right.
**
*/

void lcd_horz_line(uint8_t left, uint8_t right,
                   uint8_t row)
{
    uint8_t bit_pos;
    uint8_t byte_offset;
    uint8_t mask;
    uint8_t col;


    bit_pos = row & 0x07;   /* get the bit offset into a byte */
    byte_offset = row >> 3;      /* get the byte offset into x array */
    mask = l_mask_array[bit_pos];  /* get the mask for this bit */

    for(col = left; col <= right; col++)
    {
        l_display_array[byte_offset][col] |= mask;
    }
}

/*
**
** Draws a vertical line into display memory starting at the top
** going to the bottom in the given column. No runtime error checking 
** is performed. It is assumed that top is less than bottom and that 
** the column is in range.
**
*/

void lcd_vert_line(uint8_t top, uint8_t bottom,
                   uint8_t column)
{
    uint8_t bit_pos;
    uint8_t byte_offset;
    uint8_t y_bits;
    uint8_t remaining_bits;
    uint8_t mask;


    bit_pos = top & 0x07;     /* get starting bit offset into byte */

    byte_offset = top >> 3;     /* get byte offset into y direction */
    y_bits = (bottom - top) + 1;   /* get length in the x direction to write */
    remaining_bits = 8 - bit_pos;  /* number of bits left in byte */
    mask = l_mask_array[bit_pos];  /* get mask for this bit */

    while(y_bits)       /* while there are still bits to write */
    {
        if((remaining_bits == 8) && (y_bits > 7))
        {
            /* here if we are byte aligned and have at least 1 byte to write */
            /* do the entire byte at once instead of bit by bit */
            while(y_bits > 7)   /* while there are at least 8 more bits to do */
            {
                l_display_array[byte_offset][column] = 0xFF;
                byte_offset++;
                y_bits -= 8;
            }
        }
        else
        {
            /* we are not byte aligned or an entire byte does not need written */
            /* do each individual bit                                          */
            l_display_array[byte_offset][column] |= mask;
            if(l_mask_array[0] & 0x80)
            {
                mask >>= 1;
            }
            else
            {
                mask <<= 1;
            }
            y_bits--;
            remaining_bits--;
            if(remaining_bits == 0)
            {
                /* might have bust gotton byte aligned */
                /* so reset for beginning of a byte */
                remaining_bits = 8;
                byte_offset++;
                mask = l_mask_array[0];
            }
        }
    }
}

/*
**
** Clears a line into the display memory starting at left going to
** right, on the given row. No runtime error checking is performed.  
** It is assumed that left is less than right.
**
*/

void lcd_clr_horz_line(uint8_t left, uint8_t right,
                       uint8_t row)
{
    uint8_t bit_pos;
    uint8_t byte_offset;
    uint8_t mask;
    uint8_t col;



    bit_pos = row & 0x07;   /* get the bit offset into a byte */
    byte_offset = row >> 3;      /* get the byte offset into x array */
    mask = l_mask_array[bit_pos];  /* get the mask for this bit */

    for(col = left; col <= right; col++)
    {
        l_display_array[byte_offset][col] &= ~mask;
    }
}


/*
**
** Clears a vertical line into display memory starting at the top
** going to the bottom in the given column. No runtime error checking 
** is performed. It is assumed that top is less than bottom and that 
** the column is in range.
**
*/

void lcd_clr_vert_line(uint8_t top, uint8_t bottom,
                       uint8_t column)
{
    uint8_t bit_pos;
    uint8_t byte_offset;
    uint8_t y_bits;
    uint8_t remaining_bits;
    uint8_t mask;

    bit_pos = top & 0x07;     /* get starting bit offset into byte */

    byte_offset = top >> 3;     /* get byte offset into y direction */
    y_bits = (bottom - top) + 1;   /* get length in the x direction to write */
    remaining_bits = 8 - bit_pos;  /* number of bits left in byte */
    mask = l_mask_array[bit_pos];  /* get mask for this bit */

    while(y_bits)       /* while there are still bits to write */
    {
        if((remaining_bits == 8) && (y_bits > 7))
        {
            /* here if we are byte aligned and have at least 1 byte to write */
            /* do the entire byte at once instead of bit by bit */
            while(y_bits > 7)   /* while there are at least 8 more bits to do */
            {
                l_display_array[byte_offset][column] = 0x00;
                byte_offset++;
                y_bits -= 8;
            }
        }
        else
        {
            /* we are not byte aligned or an entire byte does not need written */
            /* do each individual bit                                          */
            l_display_array[byte_offset][column] &= ~mask;
            if(l_mask_array[0] & 0x80)
            {
                mask >>= 1;
            }
            else
            {
                mask <<= 1;
            }
            y_bits--;
            remaining_bits--;
            if(remaining_bits == 0)
            {
                /* might have bust gotton byte aligned */
                /* so reset for beginning of a byte */
                remaining_bits = 8;
                byte_offset++;
                mask = l_mask_array[0];
            }
        }
    }
}

/*
**
**  Draws a box in display memory starting at the left/top and going
**  to the right/bottom. No runtime error checking is performed.
**  It is assumed that left is less than right and that top is less 
**  than bottom.
** 
*/

void lcd_box(uint8_t left, uint8_t top,
             uint8_t right, uint8_t bottom)
{
    /* to draw a box requires two vertical lines */
    lcd_vert_line(top,bottom,left);
    lcd_vert_line(top,bottom,right);

    /* and two horizonal lines */
    lcd_horz_line(left,right,top);
    lcd_horz_line(left,right,bottom);
}

/*
**
** Clears a box in display memory starting at the Top left and going
** to the bottom right. No runtime error checking is performed and
** it is assumed that Left is less than Right and that Top is less 
** than Bottom.
**
*/

void lcd_clr_box(uint8_t left, uint8_t top,
                 uint8_t right, uint8_t bottom)
{

    /* to undraw the box undraw the two vertical lines */
    lcd_clr_vert_line(top,bottom,left);
    lcd_clr_vert_line(top,bottom,right);

    /* and the two horizonal lines that comprise it */
    lcd_clr_horz_line(left,right,top);
    lcd_clr_horz_line(left,right,bottom);
}

/*
**
** Writes a glyph to the display at location x,y
**
** Arguments are:
**    column     - x corrdinate of the left part of glyph          
**    row        - y coordinate of the top part of glyph       
**    width    - size in pixels of the width of the glyph    
**    height   - size in pixels of the height of the glyph   
**    glyph      - an uint8_t pointer to the glyph pixels
**                 to write assumed to be of length "width"
**
*/

void lcd_glyph(uint8_t left, uint8_t top,
               uint8_t width, uint8_t height,
               const uint8_t *glyph, uint8_t store_width,
               uint8_t reverse)
{
    uint8_t bit_pos;
    uint8_t byte_offset;
    uint8_t y_bits;
    uint8_t remaining_bits;
    uint8_t mask;
    uint8_t char_mask;
    uint8_t x;
    const uint8_t *glyph_scan;
    uint8_t glyph_offset;

    bit_pos = top & 0x07;  /* get the bit offset into a byte */

    glyph_offset = 0;   /* start at left side of the glyph rasters */
    char_mask = 0x80;   /* initial character glyph mask */

    if (left<128)
    {
		for (x = left; x < (left + width)&& x<128; x++)
		{
			byte_offset = top >> 3;         /* get the byte offset into y direction */
			y_bits = height;    /* get length in y direction to write */
			remaining_bits = 8 - bit_pos; /* number of bits left in byte */
			mask = l_mask_array[bit_pos]; /* get mask for this bit */
			glyph_scan = glyph + glyph_offset;  /* point to base of the glyph */

			/* boundary checking here to account for the possibility of  */
			/* write past the bottom of the screen.                        */
			while((y_bits) && (byte_offset < Y_BYTES)) /* while there are bits still to write */
			{
				/* check if the character pixel is set or not */
				if(  ((*glyph_scan & char_mask) && !reverse) ||
						(!(*glyph_scan & char_mask) &&  reverse)  )
				{
					l_display_array[byte_offset][x] |= mask; /* set image pixel */
				}
				else
				{
					l_display_array[byte_offset][x] &= ~mask; /* clear the image pixel */
				}

				if(l_mask_array[0] & 0x80)
				{
					mask >>= 1;
				}
				else
				{
					mask <<= 1;
				}

				y_bits--;
				remaining_bits--;
				if(remaining_bits == 0)
				{
					/* just crossed over a byte boundry, reset byte counts */
					remaining_bits = 8;
					byte_offset++;
					mask = l_mask_array[0];
				}

				/* bump the glyph scan to next raster */
				glyph_scan += store_width;
			}

			/* shift over to next glyph bit */
			char_mask >>= 1;
			if(char_mask == 0)    /* reset for next byte in raster */
			{
				char_mask = 0x80;
				glyph_offset++;
			}
		}
    }
}

/*
**
** Prints the given string at location x,y in the specified font.
**  Prints each character given via calls to lcd_glyph. The entry string
**  is null terminated and non 0x20->0x7e characters are ignored.
**
**  Arguments are:                                                   
**      left       coordinate of left start of string.                
**      top        coordinate of top of string.
**      font       font number to use for display                
**      str       text string to display
**
*/

void lcd_text(uint8_t left, uint8_t top, uint8_t font, char *str, uint8_t reverse)
{
	uint8_t x = left;
    uint8_t glyph;
    uint8_t width;
    uint8_t height;
    uint8_t store_width;
    const uint8_t *glyph_ptr;
    uint8_t  localReverse;

    while(*str != 0x00 && x<128)
    {
        glyph = (uint8_t)*str;

        /* check to make sure the symbol is a legal one */
        /* if not then just replace it with a "." character */
        if((glyph < ASCII_BOT) || ((glyph > ASCII_TOP)&&(glyph < (ASCII_BOT+128))) || (glyph > (ASCII_TOP+128)))
        {
            glyph = '.';
        }
        if (glyph&128)
        {
            glyph&=127;
            localReverse = !reverse;
        }
        else
            localReverse = reverse;

        /* the fonts start at ASCII_BOT, so to get the index into the font array */
        /* subtract ASCII_BOT from the glyph                  */
        glyph -= ASCII_BOT;
        width = g_asFonts[font].fixed_width; /* check if it is a fixed width */
        if(width == 0)
        {
            width=g_asFonts[font].width_table[glyph]; /* get the variable width instead */
        }

        height = g_asFonts[font].glyph_height;
        store_width = g_asFonts[font].store_width;

        glyph_ptr = g_asFonts[font].glyph_table + (glyph * store_width * height);

        lcd_glyph(x,top,width,height,glyph_ptr,store_width, localReverse);  /* plug symbol into buffer */

        x += width;       /* move right for next character */
        str++;        /* point to next character in string */
    }
}

/*
**
** Updates area of the display. Writes data from display 
** RAM to the lcd display controller.
** 
** Arguments Used:                                      
**    top     top line of area to update.         
**    bottom  bottom line of area to update.
**
*/
void lcd_update(uint8_t top, uint8_t bottom)
{
    uint8_t y;
    uint8_t yt;
    uint8_t yb;
    uint8_t data[3];
    uint8_t index;
    yt = top >> 3;                   /* setup bytes of range */
    yb = bottom >> 3;

    /* setup column of update to left side */
    data[1] = LCD_SET_COL_HI+(0/16); /* set column 26 */
    data[2] = LCD_SET_COL_LO +(4%16);
    for(y = yt; y <= yb; y++)
    {
        /* setup the page number for the y direction */
        data[0] = LCD_SET_PAGE + y; /* set page */
        for(index=0; index< sizeof(data)/sizeof(data[0]) ; index++)
        {
        	lcd_command_send(data[index]);
        }
        lcd_data_send(&l_display_array[y][0], X_BYTES);
    }
}

void lcd_led_power(uint8_t on)
{
    if (on == true)
    	GPIOSetBitValue(PIN_LCD_LED,0);  /* LCD Led On */
    else
    	GPIOSetBitValue(PIN_LCD_LED,1);  /* LCD Led Off */
}


void lcd_power(uint8_t on)
{
    uint8_t data ;

    if (on == true)
        data = LCD_DISP_ON;   /* LCD display On */
    else
        data = LCD_DISP_OFF;  /* LCD display Off */
       
    lcd_command_send(data);
}

