typedef struct
{
   unsigned char objectCount;
   short actionCount;
   unsigned char verbCount;
   unsigned char nounCount;
   unsigned char roomCount;
   unsigned char maxCarry;
   unsigned char startRoom;
   unsigned char treasureCount;
   unsigned char wordLength;
   short timeLimit;
   unsigned char messageCount;
   unsigned char treasureRoom;
   unsigned char versionMajor;
   unsigned char versionMinor;
   unsigned char adventureNumber;
} SHeader;

enum EActionConditionType
{
   kCond_Always,
   kCond_HaveObject,
   kCond_InRoomWithObject,
   kCond_ObjectAvailable,
   kCond_InRoom,
   kCond_NotInRoomWithObject,
   kCond_NotHasObject,
   kCond_NotInRoom,
   kCond_BitSet,
   kCond_BitClear,
   kCond_HaveAnyObject,
   kCond_NotHaveAnyObject,
   kCond_NotObjectAvailable,
   kCond_ObjectNotInRoom0,
   kCond_ObjectInRoom0,
   kCond_CounterLessOrEqual,
   kCond_CounterGreater,
   kCond_ObjectInOriginalRoom,
   kCond_ObjectNotInOriginalRoom,
   kCond_CounterEqual
};

enum EActionCommand
{
   kCmd_DisplayMessage0,
   kCmd_GetObject = 52,
   kCmd_DropObject,
   kCmd_GotoRoom,
   kCmd_StoreObject,
   kCmd_Night,
   kCmd_Day,
   kCmd_SetBit,
   kCmd_StoreObject_Dup,
   kCmd_ClearBit,
   kCmd_Dead,
   kCmd_MoveObject,
   kCmd_Finish,
   kCmd_DisplayRoom,
   kCmd_Score,
   kCmd_Inventory,
   kCmd_SetBit0,
   kCmd_ClearBit0,
   kCmd_FillLight,
   kCmd_ClearScreen,
   kCmd_Save,
   kCmd_ExchangeObjects,
   kCmd_ContinueToNextAction,
   kCmd_AlwaysGetObject,
   kCmd_MoveObjectToObject,
   kCmd_DisplayRoom_Dup,
   kCmd_DecrementCounter,
   kCmd_DisplayCounter,
   kCmd_SetCounter,
   kCmd_ExchangeRoomRegister0,
   kCmd_ExchangeCounter,
   kCmd_AddToCounter,
   kCmd_SubtractFromCounter,
   kCmd_SayNoun,
   kCmd_SayNounCR,
   kCmd_SayCR,
   kCmd_ExchangeRoomRegister,
   kCmd_Delay,
   kCmd_Unused,
   kCmd_DisplayMessage52 = 102
};

typedef struct
{
   unsigned char verb;
   unsigned char noun; // or probability
   unsigned char conditionType[5];
   unsigned char conditionData[5];
   unsigned char command[4];
} SAction;

typedef struct
{
   short name;
   unsigned char synonymFor;
} SVocab;

typedef struct
{
   unsigned char exits[6];
   unsigned char useDescPrefix;
   short desc;
} SRoom;

typedef struct
{
   short desc;
   unsigned char noun;
   unsigned char startRoom;
   unsigned char treasure;
} SObject;
