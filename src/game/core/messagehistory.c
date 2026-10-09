#include "3dc.h"
#include "module.h"
#include "inline.h"
#include "stratdef.h"
#include "gamedef.h"
#include "language.h"
#include "messagehistory.h"

#define MAX_NO_OF_MESSAGES_IN_HISTORY 64

extern void NewOnScreenMessage(unsigned char *messagePtr);

struct MessageHistory
{
	enum TEXTSTRING_ID StringID;
	int Hours;
	int Minutes;
	int Seconds;
};

static struct MessageHistory MessageHistoryStore[MAX_NO_OF_MESSAGES_IN_HISTORY];
static int NumberOfEntriesInMessageHistory;
static int EntryToNextShow;
static int MessageHistoryAccessedTimer;

void MessageHistory_Initialise(void)
{
	NumberOfEntriesInMessageHistory=0;
	EntryToNextShow=0;
	MessageHistoryAccessedTimer=0;
}

void MessageHistory_Add(enum TEXTSTRING_ID stringID)
{
	if (NumberOfEntriesInMessageHistory<MAX_NO_OF_MESSAGES_IN_HISTORY)
	{
		MessageHistoryStore[NumberOfEntriesInMessageHistory].StringID = stringID;
		MessageHistoryStore[NumberOfEntriesInMessageHistory].Hours = AvP.ElapsedHours;
		MessageHistoryStore[NumberOfEntriesInMessageHistory].Minutes = AvP.ElapsedMinutes;
		MessageHistoryStore[NumberOfEntriesInMessageHistory].Seconds = AvP.ElapsedSeconds/65536;
		NumberOfEntriesInMessageHistory++;
	}
}

void MessageHistory_DisplayPrevious(void)
{
	if (EntryToNextShow) 
	{
		unsigned char buffer[1024];

		EntryToNextShow--;
		sprintf
		(
			buffer,
			"%s %d (%02dh%02dm%02ds) \n \n%s",
			GetTextString(TEXTSTRING_INGAME_MESSAGENUMBER),
			EntryToNextShow+1,
			MessageHistoryStore[EntryToNextShow].Hours,
			MessageHistoryStore[EntryToNextShow].Minutes,
			MessageHistoryStore[EntryToNextShow].Seconds,
			GetTextString(MessageHistoryStore[EntryToNextShow].StringID)
		);
		NewOnScreenMessage(buffer);
		MessageHistoryAccessedTimer = 65536*4;

		if (!EntryToNextShow && NumberOfEntriesInMessageHistory) EntryToNextShow = NumberOfEntriesInMessageHistory;
	}
}

void MessageHistory_Maintain(void)
{
	if (MessageHistoryAccessedTimer)
	{
		extern int NormalFrameTime;
		MessageHistoryAccessedTimer -= NormalFrameTime;

		if (MessageHistoryAccessedTimer<0)
		{
			MessageHistoryAccessedTimer=0;
		}
	}
	else
	{
		EntryToNextShow = NumberOfEntriesInMessageHistory;
	}
}


/*---------------------------**
** Load/Save message history **
**---------------------------*/
#include "savegame.h"

typedef struct message_history_save_block
{
	SAVE_BLOCK_HEADER header;

	int NumberOfEntriesInMessageHistory;
	int EntryToNextShow;
	int MessageHistoryAccessedTimer;
	//follow by message history array
}MESSAGE_HISTORY_SAVE_BLOCK;


void Load_MessageHistory(SAVE_BLOCK_HEADER* header)
{
	int i;
	MESSAGE_HISTORY_SAVE_BLOCK* block = (MESSAGE_HISTORY_SAVE_BLOCK*) header;
	struct MessageHistory* saved_message = (struct MessageHistory*) (block+1);

	int expected_size;

	//make sure the block is the correct size
	expected_size = sizeof(*block);
	expected_size += sizeof(struct MessageHistory) * block->NumberOfEntriesInMessageHistory;
	if(header->size != expected_size) return;
	
	//load the stuff then
	NumberOfEntriesInMessageHistory = block->NumberOfEntriesInMessageHistory;
	EntryToNextShow = block->EntryToNextShow;
	MessageHistoryAccessedTimer = block->MessageHistoryAccessedTimer;

	//load the message history
	for(i=0;i<block->NumberOfEntriesInMessageHistory;i++)
	{
		MessageHistoryStore[i] = *saved_message++;
	}

	
}

void Save_MessageHistory()
{
	MESSAGE_HISTORY_SAVE_BLOCK* block;
	int i;

	//get memory for header
	GET_SAVE_BLOCK_POINTER(block);

	//fill in header
	block->header.type = SaveBlock_MessageHistory;
	block->header.size = sizeof(*block) + NumberOfEntriesInMessageHistory * sizeof(struct MessageHistory);

	block->NumberOfEntriesInMessageHistory = NumberOfEntriesInMessageHistory;
	block->EntryToNextShow = EntryToNextShow;
	block->MessageHistoryAccessedTimer = MessageHistoryAccessedTimer;

	for(i=0;i<NumberOfEntriesInMessageHistory;i++)
	{
		struct MessageHistory* message = GET_SAVE_BLOCK_POINTER(message);
		*message = MessageHistoryStore[i];
	}

}


