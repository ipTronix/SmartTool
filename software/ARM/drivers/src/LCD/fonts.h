#ifndef _FONTS_H
#define _FONTS_H

typedef struct  
{
   unsigned char store_width;            /* glyph storage width in bytes */
   unsigned char glyph_height;  		 /* glyph height for storage */
   const unsigned char *glyph_table;      /* font table start address in memory */
   unsigned char fixed_width;            /* fixed width of glyphs. If zero */
                                         /* then use the width table. */
   const unsigned char *width_table; 	 /* variable width table start adress */
} FONT_DEF;

/* define the range if characters in the font tables */
#define ASCII_BOT	0x20
#define ASCII_TOP	0x7E

/* font definition tables for the three fonts */
extern const FONT_DEF g_asFonts[];

/* glyph bitmap tables for the three fonts */ 
extern const unsigned char ISO8859[];
extern const unsigned char normal_glyph_table[];
extern const unsigned char bold_glyph_table[];
extern const unsigned char bigbold_glyph_table[];

/* width tables for the two small fonts */
extern const unsigned char normal_width_table[];
extern const unsigned char bold_width_table[];


#endif
