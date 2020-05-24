#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "message.h"

int main(void) {
	key_t kMsrpQueue = ftok("/data/msrpqueue/", PRODUCTID);
	if (kMsrpQueue == -1) {
		printf("Error: failed to create message queue key\n");
		return 1;
	}
	printf("key ID: 0x%x\n", kMsrpQueue);

	// 创建/获取消息队列
	int iMsqid = msgget(kMsrpQueue, IPC_CREAT | 0666);
	if (iMsqid == -1) {
		printf("Error: failed to create or get message queue\n");
		return 1;
	}
	printf("message queue ID: %d\n", iMsqid);

	MsrpMessage sRcvMessage;
	int ret = 0;
	while (1) {
		// 阻塞式接收消息
		ret = msgrcv(iMsqid, (void *)&sRcvMessage, MAX_TEXT_LEN + 1, TYPE1, 0);
		if (ret == -1) {
			printf("Error: failed to receive message\n");
			return 1;
		}
/*		if (strncmp(sRcvMessage.mtext, "QUIT", 4) == 0) {
			printf("QUIT\n");
			break;
		}*/
		printf("Receive message: %s\n", sRcvMessage.mtext);
	}

	return 0;
}
