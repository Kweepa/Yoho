#include <stdio.h>
#include <conio.h>
#include <string.h>

#include <cbm.h>

#include "fontdata.h"

// skip the first 256 bytes (it's the screen definitions)
#define defbase 0x1100
#define RasterValue *((char *)0x9004)

static char sx = 1;
static char sy = 183;
static char statusLowerY = 0;
static char stringIsCbm = 0;

static void __fastcall__ propfont_clearlines(char startY, char count)
{
   char _startY = startY;
   char _count = count;
   __asm__("ldy %v", _startY);
   __asm__("lda #0");
   __asm__("ldx %v", _count);
loop_clearlines:
  __asm__("sta $1100+ 0*192,y");
  __asm__("sta $1100+ 1*192,y");
  __asm__("sta $1100+ 2*192,y");
  __asm__("sta $1100+ 3*192,y");
  __asm__("sta $1100+ 4*192,y");
  __asm__("sta $1100+ 5*192,y");
  __asm__("sta $1100+ 6*192,y");
  __asm__("sta $1100+ 7*192,y");
  __asm__("sta $1100+ 8*192,y");
  __asm__("sta $1100+ 9*192,y");
  __asm__("sta $1100+10*192,y");
  __asm__("sta $1100+11*192,y");
  __asm__("sta $1100+12*192,y");
  __asm__("sta $1100+13*192,y");
  __asm__("sta $1100+14*192,y");
  __asm__("sta $1100+15*192,y");
  __asm__("sta $1100+16*192,y");
  __asm__("sta $1100+17*192,y");
  __asm__("sta $1100+18*192,y");
  __asm__("sta $1100+19*192,y");
   __asm__("iny");
   __asm__("dex");
   __asm__("bne %g", loop_clearlines);
}

void propfont_startstatus(void)
{
   char count = statusLowerY > 9 ? statusLowerY : 9;
   sy = 0;
   sx = 1;
   propfont_clearlines(0, count);
}

static void __fastcall__ drawRule(char pattern)
{
   char _pattern = pattern;
   __asm__("ldy %v", sy);
   __asm__("lda %v", _pattern);
  __asm__("sta $1100+ 0*192,y");
  __asm__("sta $1100+ 1*192,y");
  __asm__("sta $1100+ 2*192,y");
  __asm__("sta $1100+ 3*192,y");
  __asm__("sta $1100+ 4*192,y");
  __asm__("sta $1100+ 5*192,y");
  __asm__("sta $1100+ 6*192,y");
  __asm__("sta $1100+ 7*192,y");
  __asm__("sta $1100+ 8*192,y");
  __asm__("sta $1100+ 9*192,y");
  __asm__("sta $1100+10*192,y");
  __asm__("sta $1100+11*192,y");
  __asm__("sta $1100+12*192,y");
  __asm__("sta $1100+13*192,y");
  __asm__("sta $1100+14*192,y");
  __asm__("sta $1100+15*192,y");
  __asm__("sta $1100+16*192,y");
  __asm__("sta $1100+17*192,y");
  __asm__("sta $1100+18*192,y");
  __asm__("sta $1100+19*192,y");
}

void propfont_endstatus(void)
{
   sy += 2;
   drawRule(255);
   ++sy;
   drawRule(0xAA);

  statusLowerY = sy + 1;
  sy = 183;
}

static void scrollscreen(void)
{
  __asm__("ldy %v", statusLowerY);
loop_scroll:
  __asm__("lda $1100+ 0*192+10,y");
  __asm__("sta $1100+ 0*192,y");
  __asm__("lda $1100+ 1*192+10,y");
  __asm__("sta $1100+ 1*192,y");
  __asm__("lda $1100+ 2*192+10,y");
  __asm__("sta $1100+ 2*192,y");
  __asm__("lda $1100+ 3*192+10,y");
  __asm__("sta $1100+ 3*192,y");
  __asm__("lda $1100+ 4*192+10,y");
  __asm__("sta $1100+ 4*192,y");
  __asm__("lda $1100+ 5*192+10,y");
  __asm__("sta $1100+ 5*192,y");
  __asm__("lda $1100+ 6*192+10,y");
  __asm__("sta $1100+ 6*192,y");
  __asm__("lda $1100+ 7*192+10,y");
  __asm__("sta $1100+ 7*192,y");
  __asm__("lda $1100+ 8*192+10,y");
  __asm__("sta $1100+ 8*192,y");
  __asm__("lda $1100+ 9*192+10,y");
  __asm__("sta $1100+ 9*192,y");
  __asm__("lda $1100+10*192+10,y");
  __asm__("sta $1100+10*192,y");
  __asm__("lda $1100+11*192+10,y");
  __asm__("sta $1100+11*192,y");
  __asm__("lda $1100+12*192+10,y");
  __asm__("sta $1100+12*192,y");
  __asm__("lda $1100+13*192+10,y");
  __asm__("sta $1100+13*192,y");
  __asm__("lda $1100+14*192+10,y");
  __asm__("sta $1100+14*192,y");
  __asm__("lda $1100+15*192+10,y");
  __asm__("sta $1100+15*192,y");
  __asm__("lda $1100+16*192+10,y");
  __asm__("sta $1100+16*192,y");
  __asm__("lda $1100+17*192+10,y");
  __asm__("sta $1100+17*192,y");
  __asm__("lda $1100+18*192+10,y");
  __asm__("sta $1100+18*192,y");
  __asm__("lda $1100+19*192+10,y");
  __asm__("sta $1100+19*192,y");
  __asm__("iny");
  __asm__("cpy #182");
  __asm__("bne %g", loop_scroll);

  propfont_clearlines(183, 9);
}

void propfont_cls(void)
{
   propfont_clearlines(0, 192);
}

void __fastcall__ propfont_setcol(char _col)
{
   char col = _col;
   __asm__("ldx #0");
   __asm__("lda %v", col);
loop_setcol:
   __asm__("sta $9400,x");
   __asm__("inx");
   __asm__("cpx #240");
   __asm__("bne %g", loop_setcol);
}

void propfont_setup(void)
{
   static char offsets[] = { 0x02, 0xfe, 0xfe, 0xeb, 0, 0x0c };
   char i;
   char row, column, udgIndex;
   char screenIndex = 0;

   for (i = 0; i < 6; ++i)
   {
      ((char *)0x9000)[i] = ((char *)0xede4)[i] + offsets[i];
   }

   for (row = 0; row < 12; ++row)
   {
      // skip over the first 16
      udgIndex = row + 16;
      for (column = 0; column < 20; ++column)
      {
         ((char *)0x1000)[screenIndex] = udgIndex;
         udgIndex += 12;
         screenIndex++;
      }
   }
}

void cr(void)
{
   if (sy == 183)
   {
      scrollscreen();
   }
   else
   {
      sy += 9;
      propfont_clearlines(sy, 9);
   }
   sx = 1;
}

static void __fastcall__ __propfont_putc(char *f)
{
  char fl = ((unsigned int)f)&0xff;
  char fh = ((unsigned int)f)>>8;
  char ol, oh;
  unsigned int sxc = (sx&0xf8)<<3;
  char *off = (char *)(defbase + sy);
  off += sxc;
  sxc <<= 1;
  off += sxc;
  ol = ((unsigned int)off)&0xff;
  oh = ((unsigned int)off)>>8;

  __asm__("lda %v", fl);
  __asm__("sta $c1");
  __asm__("lda %v", fh);
  __asm__("sta $c2");

  __asm__("lda %v", ol);
  __asm__("sta $63");
  __asm__("lda %v", oh);
  __asm__("sta $64");

  __asm__("clc");
  __asm__("lda $63");
  __asm__("adc #192");
  __asm__("sta $65");
  __asm__("lda $64");
  __asm__("adc #0");
  __asm__("sta $66");

  __asm__("lda %v", sx);
  __asm__("and #7");
  __asm__("sta $60");

  __asm__("ldy #0");
go_on:
  __asm__("lda #0");
  __asm__("sta $62");
  __asm__("lda ($c1),y"); // fontdata ptr

  __asm__("ldx $60");
  __asm__("beq %g", endofshift);
shift:
  __asm__("lsr");
  __asm__("ror $62");
  __asm__("dex");
  __asm__("bne %g", shift);
endofshift:
  __asm__("sta $61");
      // XOR allows me to erase it with a second call
  __asm__("lda ($63),y");
  __asm__("eor $61");
  __asm__("sta ($63),y");
  __asm__("lda ($65),y");
  __asm__("eor $62");
  __asm__("sta ($65),y");
  __asm__("iny");
  __asm__("cpy #8");
  __asm__("bne %g", go_on);
}

static char __fastcall__ hasdescender(char c)
{
   char pet = c + 'a' - 97;
   if (pet == 'g' || pet == 'p' || pet == 'q' || pet == 'y')
   {
      return 1;
   }
   return 0;
}

void __fastcall__ propfont_putc(char c)
{
   char index = c - 32;
   char *f = &fontData[8*index];
   char d = hasdescender(c);
   if (d) sy++;
   __propfont_putc(f);
   sx += charWidth[index];
   if (d) sy--;
}

void __fastcall__ propfont_unputc(char c)
{
   char index = c - 32;
   char *f = &fontData[8*index];
   char d = hasdescender(c);
   if (d) sy++;
   sx -= charWidth[index];
   __propfont_putc(f);
   if (d) sy--;
}

static void waitforkey(void)
{
   char r, rc;
   rc = 0;
   r = 100;
   while (!kbhit() && rc < 20)
   {
      if (RasterValue == r)
      {
         rc++;
         r--;
      }
   }
}

char propfont_getx(void)
{
   return sx;
}

char propfont_getc(void)
{
   static char cursordef[] = { 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8 };
   while (!kbhit())
   {
      __propfont_putc(cursordef);
      waitforkey();
      __propfont_putc(cursordef);
      waitforkey();
   }
   return cgetc();
}

static char __fastcall__ translatechar(char uc)
{
   char c = uc;
   if (stringIsCbm)
   {
      if (c >= 'A' && c <= 'Z')
      {
         c -= 'A' - 65;
      }
      else if (c >= 'a' && c <= 'z')
      {
         c += 97 - 'a';
      }
   }
   return c;
}

void __fastcall__ propfont_putstring(char *str)
{
   char i = 0;
   char lastChar = 0;
   while (1) 
   {
      char c = str[i];
      if (c == 0) break;
      c = translatechar(c);
      if (lastChar == 97 && c == 116) // a t
      {
         sx--;
      }
      propfont_putc(c);
      lastChar = c;
      ++i;
   }
}

static char __fastcall__ propfont_wordwidth(char *str)
{
   char i = 0;
   char w = 0;
   char lastChar = 0;
   while (1) 
   {
      char c = str[i];
      if (c == 0) break;
      c = translatechar(c);
      if (lastChar == 97 && c == 116) // a t
      {
         w--;
      }
      w += charWidth[c - 32];
      lastChar = c;
      ++i;
   }
   return w;
}

static void __fastcall__ printword(char *str, char a, char b)
{
   char word[22];
   char len = b - a;
   char w;
   strncpy(word, str + a, len);
   word[len] = 0;
   w = propfont_wordwidth(word);
   if (sx + 3 + w >= 160)
   {
      cr();
   }
   if (sx > 1)
   {
      sx += 3;
   }
   propfont_putstring(word);
}

static void __fastcall__ _print(char *str)
{
   char rw = 0;
   char a = 0;
   char sw, c;

   while (1)
   {
      c = str[a];
      if (c == 0)
      {
         break;
      }
      if (c != ' ' && !rw)
      {
         // start of word
         sw = a;
         rw = 1;
      }
      if (c == ' ' && rw)
      {
         // end of word
         printword(str, sw, a);
         rw = 0;
      }
      ++a;
   }
   if (rw)
   {
      // straggler
      printword(str, sw, a);
   }
}

void __fastcall__ printCbm(char *as)
{
   stringIsCbm = 1;
   _print(as);
}

void __fastcall__ printAsc(char *as)
{
   stringIsCbm = 0;
   _print(as);
}

void __fastcall__ printCbm_cr(char *as)
{
   printCbm(as);
   cr();
}

void __fastcall__ printAsc_cr(char *as)
{
   printAsc(as);
   cr();
}

void __fastcall__ printInt(int x)
{
   char caInt[5];
   char s = 0;
   char *p = caInt;
   if (x < 0)
   {
      *p++ = '-';
      x = -x;
   }
   if (x >= 100)
   {
      *p++ = '0' + x/100;
      s = 1;
   }
   x = x%100;
   if (s == 1 || x >= 10)
   {
      *p++ = '0' + x/10;
   }
   x = x%10;
   *p++ = '0' + x;
   *p++ = 0;
   printCbm(caInt);
}
