#ifndef _MYHEAD_H_
#define _MYHEAD_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <pthread.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
extern 	struct tasklist *list_init(void);
extern	void handler(void *arg);
extern	void *gettask(void *arg);////取出任务链表中的任务
extern	struct threadpool *pool_init(int n);//初始化线程池
extern	struct tasklist *find_node(struct tasklist *head);
extern	int insert_node(struct tasklist *p,struct tasklist *new);
extern	int add_task(void *(*sometask)(void *),void *taskarg,struct threadpool *somepool);//尾插法添加新任务
extern	int pool_destroy(struct threadpool *somepool,int n);//回收线程
#endif