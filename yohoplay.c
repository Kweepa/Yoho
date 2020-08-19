#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>

#include "display.h"
#include "fontdata.h"
#include "loadsave.h"

#include "yoho.h"

// Scott Adams classic adventure interpreter
// Vic-20 specific

// Vic specific
#define CaseLock *((char *)657)
#define ScreenBorderColor *((char *)0x900f)

// BASIC source here:
// http://www.commodore128.org/index.php?topic=2924.0
// numeric comments refer to line numbers

// stuff from the data file
SHeader *head;
unsigned char *acts;
SVocab *verbs;
SVocab *nouns;
short *messages;
SRoom *rooms;
SObject *objs;
char *strings;

// stuff stored in save game
typedef struct
{
   int lx; // time
   int cnt;
   char r; // current room
   char flags[32]; // sf
   signed char altRoom[6];
   int altCounter[8];
   signed char objLoc[256]; // ia()
}
SSaveData;
SSaveData sd;
// end stuff stored in save game


unsigned char verb; // nv(0)
unsigned char noun; // nv(1)
char finish;

static char *longForm[9] = { "north", "south", "east", "west", "up", "down", "inven", "look", "examine" };

char tellMe = 1;

void showIntro(void)
{
   // 180
   printCbm_cr("WELCOME TO ADVENTURE");
   printCbm_cr("the object of your adventure is to find treasures and return them to their proper place");
   printCbm_cr(" i'm your clone. give me commands that consist of a verb and a noun.");
   printCbm_cr("I.E., GO EAST, TAKE KEY, CLIMB TREE, SAVE GAME, TAKE INVENTORY.");
   printCbm_cr(" you'll need special items to do some things, but I'm sure you'll be a good adventurer and figure these things out.");
   printCbm_cr("HAPPY ADVENTURING!!");
}

void loadGame(void)
{
   // 320
   load("save.bin", (char *)&sd, 8);
   tellMe = 1;
}

void saveGame(void)
{
   // 2020
   save("save.bin", (char *)&sd, (char *)(&sd+1), 8);
}

char lastDescriptionRoom = 0;
char lastDescriptionDark;
char lastDescriptionObjCount;
int lastDescriptionObjHash;
int objHash;

void addToHash(char x)
{
   objHash ^= x;
   objHash *= 11;
}

void describeRoom(void)
{
   char k = 0;
   char objCount;
   char z;

   // 550
   char dark = sd.flags[15] && sd.objLoc[9] != -1 && sd.objLoc[9] != sd.r;
   objHash = 23;
   for (z = 0; z < head->objectCount; ++z)
   {
      if (sd.objLoc[z] == sd.r)
      {
         addToHash(z);
         ++k;
      }
   }
   objCount = k;

   // has anything visibly changed?
   if (lastDescriptionRoom != sd.r
      || lastDescriptionDark != dark
      || (!dark && (lastDescriptionObjCount != k
                  || lastDescriptionObjHash != objHash)))
   {
      propfont_startstatus();
      // 550
      if (dark)
      {
         printCbm_cr("i can't see, its too dark!");
      }
      else
      {
         if (rooms[sd.r].useDescPrefix)
         {
            printCbm("I'm in a ");
         }
         printAsc_cr(strings + rooms[sd.r].desc);
         if (k > 0)
         {
            printCbm("Visible items: ");
            for (z = 0; z < head->objectCount; ++z)
            {
               if (sd.objLoc[z] == sd.r)
               {
                  // 600
                  char buff[128];
                  strcpy(buff, strings + objs[z].desc);
                  if (--k)
                  {
                     strcat(buff, "; ");
                  }
                  else
                  {
                     strcat(buff, ".");
                  }
                  printAsc(buff);
               }
            }
            cr();
         }
         // 680
         k = 1;
         for (z = 0; z < 6; ++z)
         {
            if (rooms[sd.r].exits[z] != 0)
            {
               if (k == 1)
               {
                  printCbm("Obvious exits: ");
                  k = 0;
               }
               printCbm(longForm[z]);
               printCbm(" ");
            }
         }
         if (k == 0)
         {
            cr();
         }
      }
      propfont_endstatus();
   }
   lastDescriptionRoom = sd.r;
   lastDescriptionDark = dark;
   lastDescriptionObjCount = objCount;
   lastDescriptionObjHash = objHash;
}

void go(void)
{
   // 'go'
   // 1190
   char dark = sd.flags[15] && sd.objLoc[9] != sd.r && sd.objLoc[9] != -1;
   if (dark)
   {
      printCbm_cr("dangerous in the dark!");
   }
   if (noun < 1)
   {
      printCbm_cr("give me a direction too.");
   }
   else
   {
      char newRoom = rooms[sd.r].exits[noun-1];
      if (newRoom == 0)
      {
         if (dark)
         {
            printCbm_cr("i fell down and broke my neck.");
            newRoom = head->roomCount-1;
            sd.flags[15] = 0;
         }
         else
         {
            printCbm_cr("i can't go in that direction!!");
         }
      }
      if (newRoom != 0)
      {
         sd.r = newRoom;
      }
   }
}

char haveAnyObject(void)
{
   char i;
   char f1 = 0;
   for (i = 0; i < head->objectCount; ++i)
   {
      if (sd.objLoc[i] == -1)
      {
         f1 = 0;
         break;
      }
   }
   return f1;
}

unsigned char *ap;
unsigned char *apend;
unsigned char counts;
unsigned char numCond;
unsigned char numAct;
unsigned char v;
unsigned char n;

char testConditions(unsigned char numCond)
{
   // 800
   char f2 = 1;
   char k = 0;
   while (k < numCond)
   {
      unsigned char c = *ap++;
      unsigned char d = *ap++;
      char f1 = 1;
      ++k;
      switch (c)
      {
      case kCond_Always:
         break;
      case kCond_HaveObject:
         f1 = sd.objLoc[d] == -1;
         break;
      case kCond_InRoomWithObject:
         f1 = sd.objLoc[d] == sd.r;
         break;
      case kCond_ObjectAvailable:
         f1 = sd.objLoc[d] == -1 || sd.objLoc[d] == sd.r;
         break;
      case kCond_InRoom:
         f1 = d == sd.r;
         break;
      case kCond_NotInRoomWithObject:
         f1 = sd.objLoc[d] != sd.r;
         break;
      case kCond_NotHasObject:
         f1 = sd.objLoc[d] != -1;
         break;
      case kCond_NotInRoom:
         f1 = d != sd.r;
         break;
      case kCond_BitSet:
         f1 = sd.flags[d] != 0;
         break;
      case kCond_BitClear:
         f1 = sd.flags[d] == 0;
         break;
      case kCond_HaveAnyObject:
         f1 = haveAnyObject();
         break;
      case kCond_NotHaveAnyObject:
         f1 = 1 - haveAnyObject();
         break;
      case kCond_NotObjectAvailable:
         f1 = sd.objLoc[d] != -1 && sd.objLoc[d] != sd.r;
         break;
      case kCond_ObjectNotInRoom0:
         f1 = sd.objLoc[d] != 0;
         break;
      case kCond_ObjectInRoom0:
         f1 = sd.objLoc[d] == 0;
         break;
      case kCond_CounterLessOrEqual:
         // need to ensure that <= 0 works
         f1 = sd.cnt <= (int) d;
         break;
      case kCond_CounterGreater:
         // need to ensure that -cnt > d fails
         f1 = sd.cnt > (int) d;
         break;
      case kCond_ObjectInOriginalRoom:
         f1 = sd.objLoc[d] == objs[d].startRoom;
         break;
      case kCond_ObjectNotInOriginalRoom:
         f1 = sd.objLoc[d] != objs[d].startRoom;
         break;
      case kCond_CounterEqual:
         f1 = sd.cnt == d;
         break;
      }
      f2 = f2 && f1;
   }
   return f2;
}

char countCarried(void)
{
   char carry = 0;
   char i;
   for (i = 0; i < head->objectCount; ++i)
   {
      if (sd.objLoc[i] == -1)
      {
         ++carry;
      }
   }
   return carry;
}

char tryGet(char o)
{
   char carry = countCarried();
   if (carry >= head->maxCarry)
   {
      printCbm_cr("i've too much to carry!");
      return 0;
   }
   sd.objLoc[o] = -1;
   return 1;
}

char *verbStr;
char *nounStr;

char cont;
char died;

int doAction(unsigned char cmd)
{
   unsigned char p1 = *ap;
   unsigned char p2 = *(ap + 1U);
   switch (cmd)
   {
   case kCmd_GetObject:
      if (!tryGet(p1))
      {
         return 0;
      }
      break;
   case kCmd_DropObject:
      sd.objLoc[p1] = sd.r;
      break;
   case kCmd_GotoRoom:
      sd.r = p1;
      break;
   case kCmd_StoreObject:
      sd.objLoc[p1] = 0;
      break;
   case kCmd_Night:
      sd.flags[15] = 1;
      break;
   case kCmd_Day:
      sd.flags[15] = 0;
      break;
   case kCmd_SetBit:
      sd.flags[p1] = 1;
      break;
   case kCmd_StoreObject_Dup:
      sd.objLoc[p1] = 0;
      break;
   case kCmd_ClearBit:
      sd.flags[p1] = 0;
      break;
   case kCmd_Dead:
      printCbm_cr("i'm dead...");
      sd.r = head->roomCount-1;
      sd.flags[15] = 0;
      died = 1;
      break;
   case kCmd_MoveObject:
      sd.objLoc[p1] = p2;
      break;
   case kCmd_Finish:
      // set a flag to restart
      finish = 1;
      return 0;
      break;
   case kCmd_DisplayRoom:
      break;
   case kCmd_Score:
      {
         int collected = 0;
         char i;
         for (i = 0; i < head->objectCount; ++i)
         {
            if (objs[i].treasure && sd.objLoc[i] == head->treasureRoom)
            {
               ++collected;
            }
         }
         printCbm("i've stored ");
         printInt(collected);
         printCbm(" treasures. on a scale of 0 TO 100 that rates a ");
         printInt((100*collected)/head->treasureCount);
         cr();
         if (collected == head->treasureCount)
         {
            printCbm_cr("well done.");
            finish = 1;
         }
      }
      break;
   case kCmd_Inventory:
      {
         unsigned char nothing = 1;
         char i;
         printCbm_cr("i'm carrying:");
         for (i = 0; i < head->objectCount; ++i)
         {
            if (sd.objLoc[i] == -1)
            {
               printAsc_cr(strings+objs[i].desc);
               nothing = 0;
            }
         }
         if (nothing)
         {
            printCbm_cr("nothing");
         }
      }
      break;
   case kCmd_SetBit0:
      sd.flags[0] = 1;
      break;
   case kCmd_ClearBit0:
      sd.flags[0] = 0;
      break;
   case kCmd_FillLight:
      sd.lx = head->timeLimit;
      sd.objLoc[9] = -1;
      sd.flags[16] = 0;
      break;
   case kCmd_ClearScreen:
      // does nothing
      break;
   case kCmd_Save:
      saveGame();
      break;
   case kCmd_ExchangeObjects:
      {
         char z = sd.objLoc[p1];
         sd.objLoc[p1] = sd.objLoc[p2];
         sd.objLoc[p2] = z;
      }
      break;
   case kCmd_ContinueToNextAction:
      cont = 1;
      break;
   case kCmd_AlwaysGetObject:
      sd.objLoc[p1] = -1;
      break;
   case kCmd_MoveObjectToObject:
      sd.objLoc[p1] = sd.objLoc[p2];
      break;
   case kCmd_DisplayRoom_Dup:
      break;
   case kCmd_DecrementCounter:
      --sd.cnt;
      if (sd.cnt < -1)
      {
         sd.cnt = -1;
      }
      break;
   case kCmd_DisplayCounter:
      printInt(sd.cnt);
      printCbm(" ");
      break;
   case kCmd_SetCounter:
      sd.cnt = p1;
      break;
   case kCmd_ExchangeRoomRegister0:
      {
         unsigned char z = sd.r;
         sd.r = sd.altRoom[0];
         sd.altRoom[0] = z;
      }
      break;
   case kCmd_ExchangeCounter:
      {
         int z = sd.cnt;
         sd.cnt = sd.altCounter[p1];
         sd.altCounter[p1] = z;
      }
      break;
   case kCmd_AddToCounter:
      sd.cnt += p1;
      break;
   case kCmd_SubtractFromCounter:
      sd.cnt -= p1;
      if (sd.cnt < -1)
      {
         sd.cnt = -1;
      }
      break;
   case kCmd_SayNoun:
      printCbm(nounStr);
      break;
   case kCmd_SayNounCR:
      printCbm_cr(nounStr);
      break;
   case kCmd_SayCR:
      cr();
      break;
   case kCmd_ExchangeRoomRegister:
      {
         unsigned char z = sd.r;
         sd.r = sd.altRoom[p1];
         sd.altRoom[p1] = z;
      }
      break;
   case kCmd_Delay:
      break;
   }
   return 1;
}

unsigned char numParms[38] =
{
   1, 1, 1, 1, 0, 0, 1, 1,
   1, 0, 2, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 2, 0, 1, 2,
   0, 0, 0, 1, 0, 1, 1, 1,
   0, 0, 0, 1, 0, 0
};

void doActions(void)
{
   while (ap != apend)
   {
      unsigned char cmd = *ap++;
      if (cmd > 0 && cmd < 52)
      {
         printAsc(strings+messages[cmd]);
      }
      else if (cmd >= 102)
      {
         printAsc(strings+messages[cmd-50]);
      }
      else
      {
         if (!doAction(cmd))
         {
            // stop processing actions
            ap = apend;
            cont = 0;
            break;
         }
         else
         {
            ap += numParms[cmd-52];
         }
      }
   }
   if (propfont_getx() > 1)
   {
      cr();
   }
}

void readNextAction(void)
{
   counts = *ap++;
   numCond = counts>>4;
   numAct = counts - (numCond<<4);
   v = *ap++;
   n = *ap++;
   apend = ap + 2*numCond + numAct;
}

void runDefaultActions(void)
{
   cont = 0;
   died = 0;
   ap = acts;
   while (*(ap+1) == 0)
   {
      readNextAction();

      if (cont && n != 0)
      {
         cont = 0;
      }
      if (cont || (rand() % 100) < n)
      {
         if (testConditions(numCond))
         {
            doActions();
            if (died)
            {
               break;
            }
         }
      }
      ap = apend;
   }
}

void runActions(void)
{
   // 710
   char f = 1; // command needs response
   char f3 = 0; // command matches action
   char f2 = 0; // command passes action conditions

   if (verb == 1 && noun < 7)
   {
      go();
   }
   else
   {
      unsigned char *actsEnd = acts + head->actionCount;

      cont = 0;
      ap = acts;

      //skip the default actions
      while (*(ap + 1) == 0)
      {
         readNextAction();
         ap = apend;
      }
      while (ap != actsEnd)
      {
         unsigned char pass = 0;
         readNextAction();
         if (cont)
         {
            if (v == 0 && n == 0)
            {
               pass = 1;
            }
            else
            {
               cont = 0;
               return;
            }
         }
         if (!pass)
         {
            if (verb == v)
            {
               if (n == 0 || noun == n)
               {
                  pass = 1;
               }
            }
         }
         if (pass)
         {
            f = 0;
            f3 = 1;

            f2 = testConditions(numCond); 
            if (f2)
            {
               doActions();
               if (!cont)
               {
                  return;
               }
            }
         }
         ap = apend;
      }
      // 1640
      if ((verb == 10 || verb == 18) && !f3)
      {
         char exists = 0;
         char got = 0;
         char obj;
         f = 0;
         if (verb == 10)
         {
            if (noun)
            {
               for (obj = 0; obj < head->objectCount; ++obj)
               {
                  if (objs[obj].noun == noun)
                  {
                     exists = 1;

                     // get
                     if (sd.objLoc[obj] == sd.r)
                     {
                        got = 1;
                        if (tryGet(obj))
                        {
                           printCbm_cr("OK, ");
                        }
                        break;
                     }
                  }
               }
            }
            if (!got)
            {
               if (exists)
               {
                  printCbm_cr("i don't see it here.");
               }
               else
               {
                  printCbm_cr("what?");
               }
            }
         }
         else
         {
            if (noun)
            {
               for (obj = 0; obj < head->objectCount; ++obj)
               {
                  if (objs[obj].noun == noun)
                  {
                     exists = 1;

                     // drop
                     if (sd.objLoc[obj] == -1)
                     {
                        got = 1;
                        sd.objLoc[obj] = sd.r;
                        printCbm_cr("OK, ");
                        break;
                     }
                  }
               }
            }
            if (!got)
            {
               if (exists)
               {
                  printCbm_cr("i'm not carrying it!");
               }
               else
               {
                  printCbm_cr("what?");
               }
            }
         }
      }
      if (f)
      {
         printCbm_cr("i don't understand your command.");
      }
      else if (f3 && !f2)
      {
         printCbm_cr("i can't do that yet.");
      }
   }
}

static char buff[64];

static void getcommand(void)
{
   char c, p, l = 0;
   while (1)
   {
      char letter = 0, number = 0;
      c = propfont_getc();
      if (c >= 'a' && c <= 'z')
      {
         letter = 1;
      }
      else if (c >= '0' && c <= '9')
      {
         number = 1;
      }
      if (propfont_getx() < 150 && (c == ' ' || letter || number))
      {
         p = letter ? c + 32 : c;
         propfont_putc(p);
         buff[l] = c;
         l++;
      }
      else if (l > 0)
      {
         if (c == 20)
         {
            l--;
            p = buff[l];
            p = (p >= 'a' && p <= 'z') ? p + 32 : p;
            propfont_unputc(p);
         }
         else if (c == 13)
         {
            // check for non-space
            char i;
            for (i = 0; i < l; ++i)
            {
               if (buff[i] != ' ')
               {
                  cr();
                  buff[l] = 0;
                  break;
               }
            }
            if (i != l)
            {
               break;
            }
         }
      }
   }
}

char *scanWord(char *p)
{
   char *w;
   while (*p == ' ')
   {
      ++p;
   }
   w = p;
   while (*p != ' ' && *p != 0)
   {
      ++p;
   }
   *p = 0;
   return w;
}

char getInput(void)
{
   char error;
   char i;
   char len;
   char verbLen;

   // 450
   // just needs to set verb and noun from input
   if (tellMe)
   {
      printCbm_cr("tell me what to do");
      tellMe = 0;
   }
   getcommand();

   len = strlen(buff);
   verbStr = scanWord(buff);
   verbLen = strlen(verbStr);
   nounStr = verbStr + verbLen;
   if (nounStr < buff + len)
   {
      nounStr = scanWord(nounStr + 1);
   }

   verb = 0;
   noun = 0;
   error = 0;

   // single character abbreviations
   // note that these only work for games with standard directions
   // it won't break anything, I guess
   if (verbLen == 1)
   {
      static char abbrev[9] = { 'n', 's', 'e', 'w', 'u', 'd', 'i', 'l', 'x' };
      for (i = 0; i < 9; ++i)
      {
         if (verbStr[0] == abbrev[i])
         {
            // point directly at long form
            verbStr = longForm[i];
            break;
         }
      }
   }

   // look up verb
   for (i = 0; i < head->verbCount; ++i)
   {
      if (!strnicmp(verbStr, strings+verbs[i].name, head->wordLength))
      {
         verb = verbs[i].synonymFor;
         break;
      }
   }
   if (verb == 0)
   {
      error = 1;
      // check for direction
      for (i = 1; i <= 6; ++i)
      {
         if (!strnicmp(verbStr, strings+nouns[i].name, head->wordLength))
         {
            verb = 1; // go
            noun = i;
            error = 0;
            break;
         }
      }
   }
   else if (strlen(nounStr) > 0)
   {
      // look up noun
      for (i = 0; i < head->nounCount; ++i)
      {
         if (!strnicmp(nounStr, strings+nouns[i].name, head->wordLength))
         {
            noun = nouns[i].synonymFor;
            break;
         }
      }
      if (noun == 0)
      {
         error = 1;
      }
   }
   if (error == 1)
   {
      printCbm_cr("you use word(s) i don't know!");
   }
   return error;
}

void playGame(void)
{
   finish = 0;

   while (!finish)
   {
      char i;

      // 300
      sd.r = head->startRoom;
      sd.lx = head->timeLimit;
      for (i = 0; i < 32; ++i)
      {
         sd.flags[i] = 0;
      }
      sd.cnt = 0;
      for (i = 0; i < 8; ++i)
      {
         sd.altCounter[i] = 0;
      }

      for (i = 0; i < head->objectCount; ++i)
      {
         sd.objLoc[i] = objs[i].startRoom;
      }

      printCbm_cr("USE SAVED GAME (Y or N)");
      getcommand();
      if (buff[0] == 'y')
      {
         loadGame();
      }
      propfont_cls();

      // 380
      while (1)
      {
         runDefaultActions();
         describeRoom();
         if (finish) break;
         while (getInput()) ;
         runActions();
         if (finish) break;
         if (sd.objLoc[9] != 0 && sd.lx != -1)
         {
            --sd.lx;
            if (sd.lx < 0)
            {
               sd.flags[16] = 1;
               if (sd.objLoc[9] == -1 || sd.objLoc[9] == sd.r)
               {
                  printCbm_cr("light has run out!");
               }
               sd.objLoc[9] = 0;
            }
            else if (sd.lx < 25)
            {
               if (sd.objLoc[9] == -1 || sd.objLoc[9] == sd.r)
               {
                  printCbm("light runs out in ");
                  printInt(sd.lx);
                  printCbm_cr(" turns!");
               }
            }
         }
      }

      finish = 0;
      printCbm_cr("the game is now over.-another game?");
      getcommand();
      if (buff[0] == 'n')
      {
         finish = 1;
      }
   }
}

int main(int argc, char *argv[])
{
   char i, numAdv;
   char *name;

   CaseLock = 128; // lock
   ScreenBorderColor = 8 + 6; // 8 for black BG, + 6 for blue border
   propfont_cls();
   propfont_setup();
   propfont_setcol(2); // white characters (7 for yellow)

   showIntro();

   printCbm_cr("hit RETURN to choose ADVENTURE");
   cgetc();
   propfont_cls();

   load("index.bin", (char *)0x5E00, 8);

   i = 0;
   name = (char *) 0x5E00;
   while (*name != 0)
   {
      char key[4] = "A. ";
      key[0] = 'A' + i;
      printCbm(key);
      printAsc_cr(name);
      name += 48;
      ++i;
   }
   numAdv = i;
   do
   {
      i = cgetc() - 'a';
   }
   while (i >= numAdv);
   // etc
   printCbm_cr("Loading... ");

   name = (char *) 0x5E00 + 48*i + 32;

   // 2100
   if (load(name, (char *)0x5E00, 8))
   {
      head = (SHeader *) 0x5E00;
      acts = (unsigned char *) (head + 1);
      verbs = (SVocab *) (acts + head->actionCount);
      nouns = (SVocab *) (verbs + head->verbCount);
      messages = (short *) (nouns + head->nounCount);
      rooms = (SRoom *) (messages + head->messageCount);
      objs = (SObject *) (rooms + head->roomCount);
      // read what's left
      strings = (char *) (objs + head->objectCount);

      printCbm_cr("hit RETURN to begin ADVENTURE");
      cgetc();
      cr();

      playGame();
   }
   else
   {
      printCbm_cr("Failed to load!");
      cgetc();
   }

   return 0;
}
