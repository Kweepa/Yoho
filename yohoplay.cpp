#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>

#include "yoho.h"

// Scott Adams classic adventure interpreter
// generic c code

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
int lx; // time
int cnt; // counter
unsigned char r; // current room
unsigned char flags[32]; // sf
char altRoom[6];
int altCounter[8];
char objLoc[256]; // ia()
// end stuff stored in save game

unsigned char verb; // nv(0)
unsigned char noun; // nv(1)
char finish;

void showIntro()
{
   // 180
   printf("WELCOME TO ADVENTURE\n");
   printf("the object of your adventure is to find treasures and return them to their proper place\n");
   printf(" i'm your clone. give me commands that consist of a verb and a noun.\n");
   printf("I.E., GO EAST, TAKE KEY, CLIMB TREE, SAVE GAME, TAKE INVENTORY.\n");
   printf(" you'll need special items to do some things, but I'm sure you'll be a good adventurer and figure these things out.\n");
   printf("HAPPY ADVENTURING!!\n");
}

void loadGame()
{
   // 320
   FILE *fp = fopen("save.bin", "rb");
   if (fp)
   {
      fread(&lx, sizeof(int), 1, fp);
      fread(&cnt, sizeof(int), 1, fp);
      fread(&r , sizeof(unsigned char), 1, fp);
      fread(flags, sizeof(unsigned char), 32, fp);
      fread(objLoc, sizeof(char), head->objectCount, fp);
      fclose(fp);
   }
}

void saveGame()
{
   // 2020
   FILE *fp = fopen("save.bin", "wb");
   if (fp)
   {
      fwrite(&lx, sizeof(int), 1, fp);
      fwrite(&cnt, sizeof(int), 1, fp);
      fwrite(&r , sizeof(unsigned char), 1, fp);
      fwrite(flags, sizeof(unsigned char), 32, fp);
      fwrite(objLoc, sizeof(char), head->objectCount, fp);
      fclose(fp);
   }
}

char lastDescriptionRoom = 0;
char lastDescriptionDark;
int lastDescriptionObjHash;
int objHash;

void addToHash(char x)
{
   objHash ^= x;
   objHash *= 11;
}

void describeRoom()
{
   char k = 0;
   char z;

   // 550
   char dark = flags[15] && objLoc[9] != -1 && objLoc[9] != r;
   objHash = 0;
   for (z = 0; z < head->objectCount; ++z)
   {
      if (objLoc[z] == r)
      {
         addToHash(z);
         ++k;
      }
   }
   printf("ObjHash = %d\n", objHash);

   // has anything visibly changed?
   if (lastDescriptionRoom != r
      || lastDescriptionDark != dark
      || (!dark && lastDescriptionObjHash != objHash))
   {
      // 550
      if (dark)
      {
         printf("i can't see, its too dark!\n");
      }
      else
      {
         if (rooms[r].useDescPrefix)
         {
            printf("i'm in a ");
         }
         printf(strings + rooms[r].desc);
         printf("\n");
         if (k > 0)
         {
            printf("visible items here: ");
            for (z = 0; z < head->objectCount; ++z)
            {
               if (objLoc[z] == r)
               {
                  // 600
                  printf(strings + objs[z].desc);
                  if (--k)
                  {
                     printf("; ");
                  }
                  else
                  {
                     printf(".\n");
                  }
               }
            }
         }
         // 680
         k = 1;
         for (z = 0; z < 6; ++z)
         {
            if (rooms[r].exits[z] != 0)
            {
               if (k == 1)
               {
                  printf("obvious exits: ");
                  k = 0;
               }
               printf(strings + nouns[z+1].name);
               printf(" ");
            }
         }
         if (k == 0)
         {
            printf("\n");
         }
      }
   }
   lastDescriptionRoom = r;
   lastDescriptionDark = dark;
   lastDescriptionObjHash = objHash;
}

void go()
{
   // 'go'
   // 1190
   char dark = flags[15] && objLoc[9] != r && objLoc[9] != -1;
   if (dark)
   {
      printf("dangerous in the dark!\n");
   }
   if (noun < 1)
   {
      printf("give me a direction too.\n");
   }
   else
   {
      char newRoom = rooms[r].exits[noun-1];
      if (newRoom == 0)
      {
         if (dark)
         {
            printf("i fell down and broke my neck.\n");
            newRoom = head->roomCount-1;
            flags[15] = 0;
         }
         else
         {
            printf("i can't go in that direction!!\n");
         }
      }
      else
      {
         printf("\n");
      }
      if (newRoom != 0)
      {
         r = newRoom;
         describeRoom();
      }
   }
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
   char i;
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
         f1 = objLoc[d] == -1;
         break;
      case kCond_InRoomWithObject:
         f1 = objLoc[d] == r;
         break;
      case kCond_ObjectAvailable:
         f1 = objLoc[d] == -1 || objLoc[d] == r;
         break;
      case kCond_InRoom:
         f1 = d == r;
         break;
      case kCond_NotInRoomWithObject:
         f1 = objLoc[d] != r;
         break;
      case kCond_NotHasObject:
         f1 = objLoc[d] != -1;
         break;
      case kCond_NotInRoom:
         f1 = d != r;
         break;
      case kCond_BitSet:
         f1 = flags[d] != 0;
         break;
      case kCond_BitClear:
         f1 = flags[d] == 0;
         break;
      case kCond_HaveAnyObject:
         f1 = 0;
         for (i = 0; i < head->objectCount; ++i)
         {
            if (objLoc[i] == -1)
            {
               f1 = 1;
               break;
            }
         }
         break;
      case kCond_NotHaveAnyObject:
         f1 = 1;
         for (i = 0; i < head->objectCount; ++i)
         {
            if (objLoc[i] == -1)
            {
               f1 = 0;
               break;
            }
         }
         break;
      case kCond_NotObjectAvailable:
         f1 = objLoc[d] != -1 && objLoc[d] != r;
         break;
      case kCond_ObjectNotInRoom0:
         f1 = objLoc[d] != 0;
         break;
      case kCond_ObjectInRoom0:
         f1 = objLoc[d] == 0;
         break;
      case kCond_CounterLessOrEqual:
         f1 = cnt <= d;
         break;
      case kCond_CounterGreater:
         f1 = cnt > d;
         break;
      case kCond_ObjectInOriginalRoom:
         f1 = objLoc[d] == objs[d].startRoom;
         break;
      case kCond_ObjectNotInOriginalRoom:
         f1 = objLoc[d] != objs[d].startRoom;
         break;
      case kCond_CounterEqual:
         f1 = cnt == d;
         break;
      }
      f2 = f2 && f1;
   }
   return f2;
}

char ip;

char countCarried()
{
   char carry = 0;
   char i;
   for (i = 0; i < head->objectCount; ++i)
   {
      if (objLoc[i] == -1)
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
      printf("i've too much to carry!\n");
      return 0;
   }
   objLoc[o] = -1;
   return 1;
}

static unsigned char numParms[38] =
{
   1, 1, 1, 1, 0, 0, 1, 1,
   1, 0, 2, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 2, 0, 1, 1,
   0, 0, 0, 1, 0, 1, 1, 1,
   0, 0, 0, 1, 0, 0
};

char inputStr[64];
char *verbStr;
char *nounStr;

char cont = 0;

int doAction(unsigned char cmd)
{
   unsigned char p1 = *ap;
   unsigned char p2 = *(ap+1);
   switch (cmd)
   {
   case kCmd_GetObject:
      if (!tryGet(p1))
      {
         return 0;
      }
      break;
   case kCmd_DropObject:
      objLoc[p1] = r;
      break;
   case kCmd_GotoRoom:
      r = p1;
      break;
   case kCmd_StoreObject:
      objLoc[p1] = 0;
      break;
   case kCmd_Night:
      flags[15] = 1;
      break;
   case kCmd_Day:
      flags[15] = 0;
      break;
   case kCmd_SetBit:
      flags[p1] = 1;
      break;
   case kCmd_StoreObject_Dup:
      objLoc[p1] = 0;
      break;
   case kCmd_ClearBit:
      flags[p1] = 0;
      break;
   case kCmd_Dead:
      printf("i'm dead...\n");
      r = head->roomCount-1;
      flags[15] = 0;
      describeRoom();
      break;
   case kCmd_MoveObject:
      objLoc[p1] = p2;
      break;
   case kCmd_Finish:
      // set a flag to restart
      finish = 1;
      break;
   case kCmd_DisplayRoom:
      if (numAct == 1)
      {
         lastDescriptionRoom = 0;
      }
      describeRoom();
      break;
   case kCmd_Score:
      {
         int collected = 0;
         char i;
         for (i = 0; i < head->objectCount; ++i)
         {
            if (objs[i].treasure && objLoc[i] == head->treasureRoom)
            {
               ++collected;
            }
         }
         printf("i've stored %d treasures. on a scale\n", collected);
         printf("of 0 TO 100 that rates a %d\n", (100*collected)/head->treasureCount);
         if (collected == head->treasureCount)
         {
            printf("well done.\n");
            finish = 1;
         }
      }
      break;
   case kCmd_Inventory:
      {
         unsigned char nothing = 1;
         char i;
         printf("i'm carrying:\n");
         for (i = 0; i < head->objectCount; ++i)
         {
            if (objLoc[i] == -1)
            {
               printf(strings+objs[i].desc);
               printf("\n");
               nothing = 0;
            }
         }
         if (nothing)
         {
            printf("nothing\n");
         }
      }
      break;
   case kCmd_SetBit0:
      flags[0] = 1;
      break;
   case kCmd_ClearBit0:
      flags[0] = 0;
      break;
   case kCmd_FillLight:
      lx = head->timeLimit;
      objLoc[9] = -1;
      flags[16] = 0;
      break;
   case kCmd_ClearScreen:
      // does nothing
      break;
   case kCmd_Save:
      saveGame();
      break;
   case kCmd_ExchangeObjects:
      {
         char z = objLoc[p1];
         objLoc[p1] = objLoc[p2];
         objLoc[p2] = z;
      }
      break;
   case kCmd_ContinueToNextAction:
      cont = 1;
      break;
   case kCmd_AlwaysGetObject:
      objLoc[p1] = -1;
      break;
   case kCmd_MoveObjectToObject:
      objLoc[p1] = objLoc[p2];
      break;
   case kCmd_DisplayRoom_Dup:
      if (numAct == 1)
      {
         lastDescriptionRoom = 0;
      }
      describeRoom();
      break;
   case kCmd_DecrementCounter:
      --cnt;
      if (cnt < -1)
      {
         cnt = -1;
      }
      break;
   case kCmd_DisplayCounter:
      printf("%d ", cnt);
      break;
   case kCmd_SetCounter:
      cnt = p1;
      break;
   case kCmd_ExchangeRoomRegister0:
      {
         unsigned char z = r;
         r = altRoom[0];
         altRoom[0] = z;
      }
      break;
   case kCmd_ExchangeCounter:
      {
         int z = cnt;
         cnt = altCounter[p1];
         altCounter[p1] = z;
      }
      break;
   case kCmd_AddToCounter:
      cnt += p1;
      break;
   case kCmd_SubtractFromCounter:
      cnt -= p1;
      if (cnt < -1)
      {
         cnt = -1;
      }
      break;
   case kCmd_SayNoun:
      printf(nounStr);
      break;
   case kCmd_SayNounCR:
      printf(nounStr);
      printf("\n");
      break;
   case kCmd_SayCR:
      printf("\n");
      break;
   case kCmd_ExchangeRoomRegister:
      {
         unsigned char z = r;
         r = altRoom[p1];
         altRoom[p1] = z;
      }
      break;
   case kCmd_Delay:
      break;
   }
   return 1;
}

void doActions()
{
   ip = 0;
   while (ap != apend)
   {
      unsigned char cmd = *ap++;
      if (cmd < 52)
      {
         printf(strings+messages[cmd]);
         printf("\n");
      }
      else if (cmd >= 102)
      {
         printf(strings+messages[cmd-50]);
         printf("\n");
      }
      else
      {
         if (!doAction(cmd))
         {
            ap = apend;
            break;
         }
         else
         {
            ap += numParms[cmd-52];
         }
      }
   }
}

void readNextAction()
{
   counts = *ap++;
   numCond = counts>>4;
   numAct = counts - (numCond<<4);
   v = *ap++;
   n = *ap++;
   apend = ap + 2*numCond + numAct;
}

void runDefaultActions()
{
   cont = 0;
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
         }
      }
      ap = apend;
   }
}

void runActions()
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
      cont = 0;
      unsigned char *actsEnd = acts + head->actionCount;

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
         f = 0;
         if (verb == 10)
         {
            char obj;
            char exists = 0;
            char got = 0;
            if (noun)
            {
               for (obj = 0; obj < head->objectCount; ++obj)
               {
                  if (objs[obj].noun == noun)
                  {
                     exists = 1;

                     // get
                     if (objLoc[obj] == r)
                     {
                        got = 1;
                        if (tryGet(obj))
                        {
                           printf("OK, \n");
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
                  printf("i don't see it here.\n");
               }
               else
               {
                  printf("what?\n");
               }
            }
         }
         else
         {
            char obj;
            char exists = 0;
            char got = 0;
            if (noun)
            {
               for (obj = 0; obj < head->objectCount; ++obj)
               {
                  if (objs[obj].noun == noun)
                  {
                     exists = 1;

                     // drop
                     if (objLoc[obj] == -1)
                     {
                        got = 1;
                        objLoc[obj] = r;
                        printf("OK, \n");
                        break;
                     }
                  }
               }
            }
            if (!got)
            {
               if (exists)
               {
                  printf("i'm not carrying it!\n");
               }
               else
               {
                  printf("what?\n");
               }
            }
         }
      }
      if (f)
      {
         printf("i don't understand your command.\n");
      }
      else if (f3 && !f2)
      {
         printf("i can't do that yet.\n");
      }
   }
}

char *scanWord(char *p)
{
   char *w;
   while (*p == ' ' && *p != 0)
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

char getInput()
{
   char error;
   unsigned char i;
   char len;
   char q = 0;

   // 450
   // just needs to set verb and noun from input
   printf("tell me what to do\n");
   gets(inputStr);
   len = strlen(inputStr);

   verbStr = scanWord(inputStr);
   nounStr = verbStr + strlen(verbStr);
   if (nounStr < inputStr + len)
   {
      nounStr = scanWord(nounStr + 1);
   }

   verb = 0;
   noun = 0;
   error = 0;

   // look up verb
   for (i = 0; i < head->verbCount; ++i)
   {
      if (!strnicmp(verbStr, strings+verbs[i].name,3))
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
         if (!strnicmp(verbStr, strings+nouns[i].name, 3))
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
         if (!strnicmp(nounStr, strings+nouns[i].name, head->wordLength))//strlen(strings+nouns[i].name)))
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
      printf("you use word(s) i don't know!\n");
   }
   return error;
}

void playGame()
{
   finish = 0;

   while (!finish)
   {
      char i;

      // 300
      r = head->startRoom;
      lx = head->timeLimit;
      for (i = 0; i < 32; ++i)
      {
         flags[i] = 0;
      }

      for (i = 0; i < head->objectCount; ++i)
      {
         objLoc[i] = objs[i].startRoom;
      }

      cnt = 0;
      for (i = 0; i < 8; ++i)
      {
         altCounter[i] = lx;
      }

      for (i = 0; i < 6; ++i)
      {
         altRoom[i] = r;
      }

      printf("USE SAVED GAME (Y or N)\n");
      if (getch() == 'y')
      {
         loadGame();
      }

      describeRoom();

      // 380
      while (!finish)
      {
         runDefaultActions();
         while (getInput()) ;
         if (objLoc[9] != 0 && lx != -1)
         {
            --lx;
            if (lx < 0)
            {
               flags[16] = 1;
               if (objLoc[9] == -1 || objLoc[9] == r)
               {
                  printf("light has run out!\n");
               }
               objLoc[9] = 0;
            }
            else if (lx < 25)
            {
               if (objLoc[9] == -1 || objLoc[9] == r)
               {
                  printf("light runs out in %d turns!\n", lx);
               }
            }
         }
         runActions();
      }

      finish = 0;
      printf("the game is now over.-another game?\n");
      if (getch() == 'n')
      {
         finish = 1;
      }
   }
}

int main_play()
{
   char binName[] = "quest1.bin";
   FILE *fp;
   
   showIntro();

   // 2100
   fp = fopen(binName, "rb");
   if (fp)
   {
      int binSize;
      char *bin;

      fseek(fp, 0, SEEK_END);
      binSize = ftell(fp);
      bin = (char *) malloc(binSize);

      fseek(fp, 2, SEEK_SET);
      fread(bin, binSize-2, 1, fp);
      fclose(fp);

      head     = (SHeader *) bin;
      acts     = (unsigned char *) (head + 1);
      verbs    = (SVocab *) (acts + head->actionCount);
      nouns    = (SVocab *) (verbs + head->verbCount);
      messages = (short *) (nouns + head->nounCount);
      rooms    = (SRoom *) (messages + head->messageCount);
      objs     = (SObject *) (rooms + head->roomCount);
      strings  = (char *) (objs + head->objectCount);

      printf("hit RETURN to begin ADVENTURE\n");
      while (getch() != '\r') ;

      playGame();
   }
   return 0;
}

int main_compress(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    // swap these over to compress dat files
    //return main_compress(argc, argv);
    return main_play();
}
