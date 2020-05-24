#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <string.h>
#include "message.h"

int main(void)
{
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

	MsrpMessage sTestMessage = {
		.mtype = TYPE1,
	};
	MsrpMessage sRcvMessage;

	char pcInput[MAX_TEXT_LEN + 1] = {0};
	int ret;
	while (1) {
		// todo: msgctl获取消息队列信息
		

		printf("Please input the msrp message: ");
		fgets(pcInput, sizeof(char) * (MAX_TEXT_LEN + 1), stdin);
		pcInput[strlen(pcInput) - 1] = '\0';
		strncpy(sTestMessage.mtext, pcInput, (MAX_TEXT_LEN + 1));
		printf("Message to be sent: %s, strlen: %d\n", sTestMessage.mtext, strlen(sTestMessage.mtext));
		printf("Length of mtext: %d\n", sizeof(sTestMessage.mtext));

		// 阻塞式发送消息
		ret = msgsnd(iMsqid, (void *)&sTestMessage, sizeof(sTestMessage.mtext), 0);
		if (ret != 0) {
			printf("Error: failed to send message\n");
			return 1;
		}

		// 阻塞式接收消息
		/*ret = msgrcv(iMsqid, (void *)&sRcvMessage, MAX_TEXT_LEN + 1, TYPE1, 0);
		if (ret != 0) {
			printf("Error: failed to receive message\n");
			return 1;
		}
		printf("Receive message: %s\n", sRcvMessage.mtext);*/

		// 如果输入QUIT，结束通信
		if (strncmp(pcInput, "QUIT", 4) == 0) {
			printf("QUIT\n");
			break;
		}
	}

	ret = msgctl(iMsqid, IPC_RMID, NULL);
	if (ret != 0) {
		printf("Error: failed to delete message queue\n");
		return 1;
	}

	return 0;
}
