#include <stdio.h>
#include <string.h>

// the following code is adapted from
// http://sleepingelephant.com/ipw-web/bulletin/bb/viewtopic.php?t=2247
// and http://www.codebase64.org/doku.php?id=base:saving_a_file

static void __fastcall__ setlfs(char _logicalFile, char _device, char _secondaryAddress)
{
   static char *filename = "savegame";
   char logicalFile = _logicalFile;
   char device = _device;
   char secondaryAddress = _secondaryAddress;
   // call SETLFS
   __asm__("lda %v", logicalFile); // logical file
   __asm__("ldx %v", device); // 1 for tape, 8 for disk
   __asm__("ldy %v", secondaryAddress); // 0 to specify address
   __asm__("jsr $ffba");
}
static void __fastcall__ setnam(char *filename)
{
   char len = strlen(filename);
   char fnl = ((unsigned int)filename) & 0xff;
   char fnh = ((unsigned int)filename>>8) & 0xff;
   // call SETNAM
   __asm__("lda %v", len);
   __asm__("ldx %v", fnl);
   __asm__("ldy %v", fnh);
   __asm__("jsr $ffbd");
}

static void __fastcall__ deletesave(void)
{
   // OPEN15,8,15,"S:savegame":CLOSE15
   setlfs(15, 8, 15);
   setnam("s:save.bin"); // scratch
   __asm__("jsr $ffc0"); // OPEN
   __asm__("lda #15");
   __asm__("jsr $ffc3"); // CLOSE
}

char __fastcall__ save(char *name, char *startaddr, char *endaddr, char device)
{
   char sah, sal, eah, eal;
   char success;
   if (device == 8)
   {
      deletesave();
   }
   setlfs(1, device, 0);
   setnam(name);
   success = 1;
   sah = ((unsigned int)startaddr>>8) & 0xff;
   sal = ((unsigned int)startaddr) & 0xff;
   eah = ((unsigned int)endaddr>>8) & 0xff;
   eal = ((unsigned int)endaddr) & 0xff;
   // call SAVE
   __asm__("ldx %v", sal);
   __asm__("ldy %v", sah);
   __asm__("stx $c1");
   __asm__("sty $c2");
   __asm__("lda #$c1");
   __asm__("ldx %v", eal);
   __asm__("ldy %v", eah);
   __asm__("jsr $ffd8");
   __asm__("bcc %g", nosaveerror);
   __asm__("lda #0");
   __asm__("sta %v", success);
nosaveerror:
   return success;
}

char __fastcall__ load(char *name, char *startaddr, char device)
{
   char sah, sal;
   char success;
   setlfs(1, device, 0);
   setnam(name);
   success = 1;
   sah = ((unsigned int)startaddr>>8) & 0xff;
   sal = ((unsigned int)startaddr) & 0xff;
   // call LOAD
   __asm__("lda #0");
   __asm__("ldx %v", sal);
   __asm__("ldy %v", sah);
   __asm__("jsr $ffd5");
   __asm__("bcc %g", noloaderror);
   __asm__("lda #0");
   __asm__("sta %v", success);
noloaderror:
   return success;
}
