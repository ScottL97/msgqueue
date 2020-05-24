#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <string.h>
#include "message.h"

// todo: 消息较长时截断而不是收到多个消息

#define THREAD_NUM 20
typedef struct ThreadParam {
	int iMsqid;
	MsrpMessage *psMessage;
} ThreadParam;

int g_number = 0;
void *ReceiveMessage(void *psThreadParam)
{
	int iMsqid = ((ThreadParam *)psThreadParam)->iMsqid;
	MsrpMessage *psRcvMessage = ((ThreadParam *)psThreadParam)->psMessage;
	while (1) {
		// 阻塞式接收消息
		int ret = msgrcv(iMsqid, psRcvMessage, MAX_TEXT_LEN + 1, TYPE1, 0);
		if (ret == -1) {
			perror("Failed to receive message\n");
			return (void *)1;
		}
		printf("Receive message: %s\n", psRcvMessage->mtext);
	}
	return (void *)0;
}

void *SendMessage(void *psThreadParam)
{
	int iMsqid = ((ThreadParam *)psThreadParam)->iMsqid;
	MsrpMessage *psSndMessage = ((ThreadParam *)psThreadParam)->psMessage;
	int ret = 0;
	char pcInput[MAX_TEXT_LEN + 1] = "hello";
	// todo: msgctl获取消息队列信息

	printf("-------%d-------\n", g_number++);
	strncpy(psSndMessage->mtext, pcInput, (MAX_TEXT_LEN + 1));
	//printf("Message to be sent: %s, strlen: %d\n", psSndMessage->mtext, strlen(psSndMessage->mtext));
	//printf("Length of mtext: %d\n", sizeof(psSndMessage->mtext));

	// 阻塞式发送消息
	ret = msgsnd(iMsqid, psSndMessage, sizeof(psSndMessage->mtext), 0);
	if (ret != 0) {
		perror("Failed to send message\n");
		return (void *)1;
	}
	return (void *)0;
}

void *InputAndSendMessage(void *psThreadParam)
{
	int iMsqid = ((ThreadParam *)psThreadParam)->iMsqid;
	MsrpMessage *psSndMessage = ((ThreadParam *)psThreadParam)->psMessage;
	int ret = 0;
	char pcInput[MAX_TEXT_LEN + 1] = {0};
	while (1) {
		// todo: msgctl获取消息队列信息

		printf("-------%d-------\n", g_number++);
		printf("Please input the msrp message: ");
		fgets(pcInput, sizeof(char) * (MAX_TEXT_LEN + 1), stdin);
		pcInput[strlen(pcInput) - 1] = '\0';
		// 如果输入QUIT，结束通信
		if (strncmp(pcInput, "QUIT", 4) == 0) {
			printf("QUIT\n");
			break;
		}
		strncpy(psSndMessage->mtext, pcInput, (MAX_TEXT_LEN + 1));
		//printf("Message to be sent: %s, strlen: %d\n", psSndMessage->mtext, strlen(psSndMessage->mtext));
		//printf("Length of mtext: %d\n", sizeof(psSndMessage->mtext));

		// 阻塞式发送消息
		ret = msgsnd(iMsqid, psSndMessage, sizeof(psSndMessage->mtext), 0);
		if (ret != 0) {
			perror("Failed to send message\n");
			return (void *)1;
		}
	}
	return (void *)0;
}


int main(void)
{
	key_t kMsrpQueue = ftok("/data/msrpqueue/", PRODUCTID);
	if (kMsrpQueue == -1) {
		perror("Failed to create message queue key");
		exit(EXIT_FAILURE);
	}
	printf("key ID: 0x%x\n", kMsrpQueue);

	// 创建/获取消息队列
	int iMsqid = msgget(kMsrpQueue, IPC_CREAT | 0666);
	if (iMsqid == -1) {
		printf("Failed to create or get message queue");
		exit(EXIT_FAILURE);
	}
	printf("message queue ID: %d\n", iMsqid);

	// 发送和接收MSRP消息的结构
	MsrpMessage sTestMessage = {
		.mtype = TYPE1,
	};
	MsrpMessage sRcvMessage;

	// 线程id、参数定义
	int ret;
	pthread_t pthSendId[THREAD_NUM] = {0};
	pthread_t pthReadId, pthInputId;
	void *pthRet;
	ThreadParam sThreadParam = {
		.iMsqid = iMsqid,
		.psMessage = &sTestMessage,
	};

	// 读取消息线程
	ThreadParam sReadThreadParam = {
		.iMsqid = iMsqid,
		.psMessage = &sRcvMessage,
	};
	ret = pthread_create(&pthReadId, NULL, ReceiveMessage, (void *)&sReadThreadParam);
	if ((int)ret != 0) {
		perror("Read message queue thread creation failed");
		exit(EXIT_FAILURE);
	}

	// 创建THREAD_NUM个线程发送消息
	int i;
	for (i = 0; i < THREAD_NUM; i++) {
		ret = pthread_create(&pthSendId[i], NULL, SendMessage, (void *)&sThreadParam);
		if ((int)ret != 0) {
			perror("Send to message queue thread creation failed");
			exit(EXIT_FAILURE);
		}
	}
	
	// 手动发送消息的线程
	ret = pthread_create(&pthInputId, NULL, InputAndSendMessage, (void *)&sThreadParam);
	if ((int)ret != 0) {
		perror("Input and send to message queue thread creation failed");
		exit(EXIT_FAILURE);
	}

	ret = pthread_join(pthInputId, pthRet);
	if ((int)ret != 0) {
		perror("Thread join failed");
		exit(EXIT_FAILURE);
	}
	
	// todo: 将删除消息队列写到信号处理函数中
	/*ret = msgctl(iMsqid, IPC_RMID, NULL);
	if (ret != 0) {
		perror("Failed to delete message queue");
		exit(EXIT_FAILURE);
	}*/

	exit(EXIT_SUCCESS);
}
