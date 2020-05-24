#ifndef __MESSAGE_H
#define __MESSAGE_H

#define MAX_TEXT_LEN 10

#define TYPE1 1
#define TYPE2 2
#define TYPE3 3

#define PRODUCTID 1

typedef struct MsrpMessage {
	long mtype;
	char mtext[MAX_TEXT_LEN + 1];
} MsrpMessage;

#endif

