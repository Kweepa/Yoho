#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "yoho.h"

// code to compress dat file
// http://www.filfre.net/misc/ADVDB.TXT
// http://www.trs-80.com/download/Software/Scott%20Adams/Dats%20-%20Scott%20Adams%20Adventure%20DATs%20v2.2%20(1984)(Scott%20Adams).zip

char loadaddr[2] = { 0, 0x5e };

int read_int(FILE *fp)
{
   int i;
   char buf[128];
   fgets(buf, 128, fp);
   sscanf(buf, "%d", &i);
   return i;
}

void read_str(FILE *fp, char *buf, int *finalNum = NULL)
{
   int i = 0;
   bool stringStarted = false;
   bool stringFinished = false;
   bool firstChar = true;
   while (!feof(fp) && !stringFinished)
   {
      char line[128];
      fgets(line, 128, fp);
      for (int k = 0; k < strlen(line); ++k)
      {
         int c = line[k];
         if (!stringStarted)
         {
            if (c == '\"')
            {
               stringStarted = true;
            }
            continue;
         }
         if (c == '\r' || c == '\n')
         {
            // skip an initial carriage return
            if (firstChar)
            {
               firstChar = false;
               continue;
            }
            // convert any other carriage returns into spaces.
            c = ' ';
         }
         firstChar = false;
         if (c == '\"')
         {
            stringFinished = true;

            if (finalNum)
            {
               if (k+1 < strlen(line))
               {
                  int retVal = sscanf(line+k+1, "%d", finalNum);
                  if (retVal == 0 || retVal == EOF)
                  {
                     *finalNum = 0xffff;
                  }
               }
               else
               {
                  *finalNum = 0xffff;
               }
            }
            break;
         }
         // remove multiple spaces
         if (i == 0 || c != ' ' || buf[i-1] != ' ')
         {
            buf[i++] = c;
         }
      }
   }
   // remove trailing space
   if (i > 0 && buf[i-1] == ' ')
   {
      --i;
   }
   buf[i] = 0;
}

static char *strings;
static int si;

static SVocab *nouns;

static char *ob(SObject *obj)
{
   /*
   if (obj->noun)
   {
      return strings+nouns[obj->noun].name;
   }
   */
   return strings+obj->desc;
}

static unsigned char numParms[38] =
{
   1, 1, 1, 1, 0, 0, 1, 1,
   1, 0, 2, 0, 0, 0, 0, 1,
   1, 0, 0, 0, 2, 0, 1, 2,
   0, 0, 0, 1, 0, 1, 1, 1,
   0, 0, 0, 1, 0, 0
};

void read_vocab(FILE *fp, SVocab *words, int i, int &synonymFor)
{
   char word[128];
   read_str(fp, word);
   if (word[0] != '*')
   {
      synonymFor = i;
      strcpy(strings+si, word);
      words[i].name = si;
      si += 1+strlen(word);
   }
   else
   {
      strcpy(strings+si, word+1);
      words[i].name = si;
      si += strlen(word);
   }
   words[i].synonymFor = synonymFor;
}

unsigned char getCounterLookup(SHeader &head, int &numLookups, short data)
{
	bool found = false;
	for (int i = 0; i < numLookups; ++i)
	{
		if (head.counterLookup[i] == data)
		{
			data = i;
			found = true;
			break;
		}
	}
	if (!found)
	{
		head.counterLookup[numLookups] = data;
		data = numLookups++;
	}

	return data;
}

int convert(char *inputFile, char *outputFile, char *txtFile)
{
	int numLookups = 0;

   FILE *fp = fopen(inputFile, "r");
   if (fp)
   {
      SHeader head;

      strings = (char *) malloc(16384);
      si = 0;

      int stringSize = read_int(fp);
      head.objectCount = 1 + read_int(fp);
      head.actionCount = 1 + read_int(fp);
      head.verbCount = 1 + read_int(fp);
      head.nounCount = head.verbCount;
      head.roomCount = 1 + read_int(fp);
      head.maxCarry = read_int(fp);
      head.startRoom = read_int(fp);
      head.treasureCount = read_int(fp);
      head.wordLength = read_int(fp);
      head.timeLimit = read_int(fp);
      head.messageCount = 1 + read_int(fp);
      head.treasureRoom = read_int(fp);

      unsigned char *acts = (unsigned char *) malloc(head.actionCount*16);
      unsigned char *ap = acts;
      for (int x = 0; x < head.actionCount; ++x)
      {
         unsigned int parms[5];
         int ip = 0;
         int ic = 0;
         int verbNoun = read_int(fp);
         unsigned char *aps = ap;
         unsigned char verb = verbNoun/150;
         unsigned char noun = verbNoun-150*verb;
         int numCond = 0;
         int numAct = 0;
         // leave space for size
         ap++;
         *ap++ = verb;
         *ap++ = noun;
         for (int c = 0; c < 5; ++c)
         {
            int typePar = read_int(fp);
            unsigned int data = typePar/20;
            unsigned char cond = typePar-20*data;

            if (cond)
            {
				if (cond == kCond_CounterEqual || cond == kCond_CounterGreater || cond == kCond_CounterLessOrEqual)
				{
					data = getCounterLookup(head, numLookups, data);
				}
               *ap++ = cond;
               *ap++ = data;
               ++numCond;
            }
            else
            {
               parms[ip++] = data;
            }
         }

         int cmd12 = read_int(fp);
         int cmd34 = read_int(fp);
         int command[4];
         command[0] = cmd12/150;
         command[1] = cmd12-150*command[0];
         command[2] = cmd34/150;
         command[3] = cmd34-150*command[2];

         ip = 0;
         for (int i = 0; i < 4; ++i)
         {
            unsigned char cmd = command[i];
            if (cmd)
            {
               *ap++ = cmd;
               ++numAct;
               switch (cmd)
               {
               case kCmd_GetObject:
               case kCmd_DropObject:
               case kCmd_GotoRoom:
               case kCmd_StoreObject:
               case kCmd_SetBit:
               case kCmd_StoreObject_Dup:
               case kCmd_ClearBit:
               case kCmd_AlwaysGetObject:
               case kCmd_SetCounter:
               case kCmd_ExchangeCounter:
               case kCmd_AddToCounter:
               case kCmd_SubtractFromCounter:
               case kCmd_ExchangeRoomRegister:
				   if (cmd == kCmd_SetCounter || cmd == kCmd_AddToCounter || cmd == kCmd_SubtractFromCounter)
				   {
					   parms[ip] = getCounterLookup(head, numLookups, parms[ip]);
				   }
                  *ap++ = parms[ip++];
                  ++numAct;
                  break;
               case kCmd_MoveObject:
               case kCmd_ExchangeObjects:
               case kCmd_MoveObjectToObject:
                  *ap++ = parms[ip++];
                  *ap++ = parms[ip++];
                  numAct += 2;
                  break;
               }
            }
            else
            {
               break;
            }
         }
         // write size of action
         *aps = (numCond<<4) + numAct;
      }
      int oldCount = head.actionCount;
      head.actionCount = ap - acts;

	  printf("Num counter lookups = %d\n", numLookups);

      printf("Action size reduced from %d to %d\n", oldCount*sizeof(SAction), head.actionCount);

      SVocab *verbs = (SVocab *) malloc(head.verbCount*sizeof(SVocab));
      nouns = (SVocab *) malloc(head.nounCount*sizeof(SVocab));
      int realVerb = 0;
      int realNoun = 0;
      for (int i = 0; i < head.verbCount; ++i)
      {
         read_vocab(fp, verbs, i, realVerb);
         read_vocab(fp, nouns, i, realNoun);
      }

      SRoom *rooms = (SRoom *) malloc(head.roomCount*sizeof(SRoom));
      for (int i = 0; i < head.roomCount; ++i)
      {
         for (int e = 0; e < 6; ++e)
         {
            rooms[i].exits[e] = read_int(fp);
         }
         char desc[1024];
         read_str(fp, desc);
         if (desc[0] == '*')
         {
            rooms[i].useDescPrefix = 0;
            strcpy(strings+si, desc+1);
            rooms[i].desc = si;
            si += strlen(desc);
         }
         else
         {
            rooms[i].useDescPrefix = 1;
            strcpy(strings+si, desc);
            rooms[i].desc = si;
            si += 1+strlen(desc);
         }
      }

      short *messages = (short *) malloc(head.messageCount*sizeof(short));
      for (int i = 0; i < head.messageCount; ++i)
      {
         read_str(fp, strings+si);
         messages[i] = si;
         si += 1+strlen(strings+si);
      }

      SObject *objs = (SObject *) malloc(head.objectCount*sizeof(SObject));
      for (int i = 0; i < head.objectCount; ++i)
      {
         char objStr[128];
         int objStartRoom;
         read_str(fp, objStr, &objStartRoom);
         if (objStartRoom == 0xffff)
         {
            // must be on a separate line
            objStartRoom = read_int(fp);
         }
         objs[i].startRoom = objStartRoom == -1 ? 0xff : objStartRoom;
         int len = strlen(objStr);
         if (objStr[len-1] == '/')
         {
            char *slash = strchr(objStr, '/');
            *slash = 0;
            objStr[len-1] = 0;

            strcpy(strings+si, objStr);
            objs[i].desc = si;
            si += 1+strlen(objStr);

            objs[i].noun = 0;
            for (int k = 0; k < head.nounCount; ++k)
            {
               if (!strcmp(strings+nouns[k].name, slash+1))
               {
                  objs[i].noun = k;
                  break;
               }
            }
         }
         else
         {
            strcpy(strings+si, objStr);
            objs[i].desc = si;
            si += 1+strlen(objStr);
            objs[i].noun = 0;
         }
         objs[i].treasure = (objStr[0] == '*') ? 1 : 0;
      }

      for (int i = 0; i < head.actionCount; ++i)
      {
         char actionName[128];
         fgets(actionName, 128, fp);
      }

      int version = read_int(fp);
      head.versionMajor = version/100;
      head.versionMinor = version-100*head.versionMajor;

      head.adventureNumber = read_int(fp);

      fclose(fp);

      fp = fopen(outputFile, "wb");
      if (fp)
      {
         fwrite(loadaddr, 2, 1, fp);
         fwrite(&head, 1, sizeof(SHeader), fp);
         fwrite(acts, head.actionCount, 1, fp);
         fwrite(verbs, head.verbCount, sizeof(SVocab), fp);
         fwrite(nouns, head.nounCount, sizeof(SVocab), fp);
         fwrite(messages, head.messageCount, sizeof(short), fp);
         fwrite(rooms, head.roomCount, sizeof(SRoom), fp);
         fwrite(objs, head.objectCount, sizeof(SObject), fp);
         fwrite(strings, si, 1, fp);
         fclose(fp);
      }

      // also write out text version
      fp = fopen(txtFile, "w");
      if (fp)
      {
         unsigned char *ap = acts;
         int k = 0;
         while (ap < acts + head.actionCount)
         {
            unsigned char counts = *ap++;
            unsigned char *apend;
            unsigned char v, n;
            unsigned char numCond = counts>>4;
            unsigned char numAct = counts - (numCond<<4);
            v = *ap++;
            n = *ap++;
            fprintf(fp, "%03d: ", k++);
            if (v == 0 && n > 0)
            {
               fprintf(fp, "rnd< %d ", n);
            }
            else if (v == 0 && n == 0)
            {
               fprintf(fp, "cont ");
            }
            else
            {
               fprintf(fp, "%s %s ", strings+verbs[v].name, strings+nouns[n].name);
            }
            apend = ap + 2*numCond + numAct;
            for (int i = 0; i < numCond; ++i)
            {
               unsigned char cond = *ap++;
               unsigned char data = *ap++;

               if (cond == 0)
               {
                  fprintf(fp, "n/a ");
               }
               else
               {
                  switch (cond)
                  {
                  case kCond_Always:
                     break;
                  case kCond_HaveObject:
                     fprintf(fp, "has '%s' ", ob(&objs[data]));
                     break;
                  case kCond_InRoomWithObject:
                     fprintf(fp, "in/w '%s' ", ob(&objs[data]));
                     break;
                  case kCond_ObjectAvailable:
                     fprintf(fp, "avl '%s' ", ob(&objs[data]));
                     break;
                  case kCond_InRoom:
                     fprintf(fp, "in %d ", data);
                     break;
                  case kCond_NotInRoomWithObject:
                     fprintf(fp, "-in/w '%s' ", ob(&objs[data]));
                     break;
                  case kCond_NotHasObject:
                     fprintf(fp, "-has '%s' ", ob(&objs[data]));
                     break;
                  case kCond_NotInRoom:
                     fprintf(fp, "-in %d ", data);
                     break;
                  case kCond_BitSet:
                     fprintf(fp, "bit %d ", data);
                     break;
                  case kCond_BitClear:
                     fprintf(fp, "-bit %d ", data);
                     break;
                  case kCond_HaveAnyObject:
                     fprintf(fp, "any ");
                     break;
                  case kCond_NotHaveAnyObject:
                     fprintf(fp, "-any ");
                     break;
                  case kCond_NotObjectAvailable:
                     fprintf(fp, "-avl '%s' ", ob(&objs[data]));
                     break;
                  case kCond_ObjectNotInRoom0:
                     fprintf(fp, "-stored '%s' ", ob(&objs[data]));
                     break;
                  case kCond_ObjectInRoom0:
                     fprintf(fp, "stored '%s' ", ob(&objs[data]));
                     break;
                  case kCond_CounterLessOrEqual:
                     fprintf(fp, "c<= %d ", head.counterLookup[data]);
                     break;
                  case kCond_CounterGreater:
                     fprintf(fp, "c> %d ", head.counterLookup[data]);
                     break;
                  case kCond_ObjectInOriginalRoom:
                     fprintf(fp, "-moved '%s' ", ob(&objs[data]));
                     break;
                  case kCond_ObjectNotInOriginalRoom:
                     fprintf(fp, "moved '%s' ", ob(&objs[data]));
                     break;
                  case kCond_CounterEqual:
                     fprintf(fp, "c== %d ", head.counterLookup[data]);
                     break;
                  }
               }
            }
            fprintf(fp, "-> ");
            while (ap < apend)
            {
               unsigned char a = *ap++;
               unsigned char p1 = *ap;
               unsigned char p2 = *(ap+1);
               if (a < 52)
               {
                  fprintf(fp, "say \"%s\" ", strings+messages[a]);
               }
               else if (a >= 102)
               {
                  fprintf(fp, "say \"%s\" ", strings+messages[a-50]);
               }
               else
               {
                  ap += numParms[a-52];

                  switch (a)
                  {
                  case kCmd_GetObject:
                     fprintf(fp, "get '%s' ", ob(&objs[p1]));
                     break;
                  case kCmd_DropObject:
                     fprintf(fp, "drop '%s' ", ob(&objs[p1]));
                     break;
                  case kCmd_GotoRoom:
                     fprintf(fp, "goto %d ", p1);
                     break;
                  case kCmd_StoreObject:
                     fprintf(fp, "store '%s' ", ob(&objs[p1]));
                     break;
                  case kCmd_Night:
                     fprintf(fp, "night ");
                     break;
                  case kCmd_Day:
                     fprintf(fp, "day ");
                     break;
                  case kCmd_SetBit:
                     fprintf(fp, "set %d ", p1);
                     break;
                  case kCmd_StoreObject_Dup:
                     fprintf(fp, "store '%s' ", ob(&objs[p1]));
                     break;
                  case kCmd_ClearBit:
                     fprintf(fp, "clear %d ", p1);
                     break;
                  case kCmd_Dead:
                     fprintf(fp, "dead ");
                     break;
                  case kCmd_MoveObject:
                     fprintf(fp, "move '%s',%d ", ob(&objs[p1]), p2);
                     break;
                  case kCmd_Finish:
                     fprintf(fp, "finish ");
                     break;
                  case kCmd_DisplayRoom:
                     fprintf(fp, "look ");
                     break;
                  case kCmd_Score:
                     fprintf(fp, "score ");
                     break;
                  case kCmd_Inventory:
                     fprintf(fp, "inv ");
                     break;
                  case kCmd_SetBit0:
                     fprintf(fp, "set0 ");
                     break;
                  case kCmd_ClearBit0:
                     fprintf(fp, "clear0 ");
                     break;
                  case kCmd_FillLight:
                     fprintf(fp, "fill ");
                     break;
                  case kCmd_ClearScreen:
                     fprintf(fp, "cls ");
                     break;
                  case kCmd_Save:
                     fprintf(fp, "save ");
                     break;
                  case kCmd_ExchangeObjects:
                     fprintf(fp, "swap '%s','%s' ", ob(&objs[p1]), ob(&objs[p2]));
                     break;
                  case kCmd_ContinueToNextAction:
                     fprintf(fp, "cont ");
                     break;
                  case kCmd_AlwaysGetObject:
                     fprintf(fp, "aget '%s' ", ob(&objs[p1]));
                     break;
                  case kCmd_MoveObjectToObject:
                     fprintf(fp, "move '%s','%s' ", ob(&objs[p1]), ob(&objs[p2]));
                     break;
                  case kCmd_DisplayRoom_Dup:
                     fprintf(fp, "look ");
                     break;
                  case kCmd_DecrementCounter:
                     fprintf(fp, "dec ");
                     break;
                  case kCmd_DisplayCounter:
                     fprintf(fp, "dispcnt ");
                     break;
                  case kCmd_SetCounter:
                     fprintf(fp, "setcnt %d ", head.counterLookup[p1]);
                     break;
                  case kCmd_ExchangeRoomRegister0:
                     fprintf(fp, "xrr0 ");
                     break;
                  case kCmd_ExchangeCounter:
                     fprintf(fp, "xcnt %d ", p1);
                     break;
                  case kCmd_AddToCounter:
                     fprintf(fp, "addcnt %d ", head.counterLookup[p1]);
                     break;
                  case kCmd_SubtractFromCounter:
                     fprintf(fp, "subcnt %d ", head.counterLookup[p1]);
                     break;
                  case kCmd_SayNoun:
                     fprintf(fp, "saynoun ");
                     break;
                  case kCmd_SayNounCR:
                     fprintf(fp, "saynouncr ");
                     break;
                  case kCmd_SayCR:
                     fprintf(fp, "saycr ");
                     break;
                  case kCmd_ExchangeRoomRegister:
                     fprintf(fp, "xrr %d ", p1);
                     break;
                  case kCmd_Delay:
                     fprintf(fp, "delay ");
                     break;
                  }
               }
            }
            ap = apend;
            fprintf(fp, "\n");
         }
         fclose(fp);
      }
   }
   return 0;
}

typedef struct
{
   char name[32];
   char file[16];
}
SIndexRecord;

int main_compress(int argc, char *argv[])
{
   FILE *fp = fopen(argv[1], "r");
   if (fp)
   {
      SIndexRecord records[32];
      int i = 0;
      while (!feof(fp))
      {
         char line[64];
         char file[32];
         fgets(records[i].name, 63, fp);
         records[i].name[strlen(records[i].name)-1] = 0;
         fgets(line, 31, fp);
         strcpy(file, line);
         if (file[strlen(line)-1] == 10)
         {
            file[strlen(line)-1] = 0;
         }

         char in[32], out[32], txt[32];
         sprintf(in, "%s.dat", file);
         sprintf(out, "%s.bin", file);
         sprintf(txt, "%s.txt", file);

         strcpy(records[i].file, strupr(out));

         convert(in, out, txt);
         ++i;
      }
      fclose(fp);
      
      fp = fopen(argv[2], "wb");
      if (fp)
      {
         fwrite(loadaddr, 2, 1, fp);
         fwrite(records, sizeof(SIndexRecord), i, fp);
         fwrite(loadaddr, 1, 1, fp);
         fclose(fp);
      }
   }
   return 0;
}
