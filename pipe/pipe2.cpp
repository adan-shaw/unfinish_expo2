//111111111111111111111111111111111111111111111111111111111111111111111111
//格式测试:
//邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵
//起始日期:
//完成日期:
//************************************************************************
//修改日志:

//, , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , ,, , ,

//编译:
//g++ -o x ./pipe2.cpp -g3



#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>




#define MSG_CHAR 1
#define MSG_STRUCT 2
#define MSG_INT 3
#define MSG_DOUBLE 4
#define MSG_VOID_POINT 5



typedef struct pipe_msg_head{
	int type;//消息类型
	int buf_len;//消息总长度
}pipe_msg_h;

//PIPE_BUF=4096
//#define PIPE_BUF2 ( PIPE_BUF-sizeof(pipe_msg_h)*2 )//PIPE_BUF - 16

typedef struct pipe_msg_body{
	pipe_msg_h head;
	char buf[PIPE_BUF - 16];
}pipe_msg_t;


//for test(用作传输的结构体)
typedef struct test_body{
	char x1[64];
	char x2[128];
	int x3;
}test_t;



//1.管道测试
void pipe_test(void);

//2.
//pipe write - 需要填写正确"pipe_msg_h"
//向管道写入数据
//如果当前管道不可用, 则阻塞等待到可用为止.
int my_pipe_write(int pipe_fd, pipe_msg_t *pdata);

//3.
//pipe read - 需要传入pipe_msg_t实体, 进行引用调用
//等待读取'管道'的数据, 如果当前没有数据可读, 则阻塞.
//所以这也不需要担心会错过.
int my_pipe_read(int pipe_fd, pipe_msg_t *pdata);

//4.管道测试2-传输结构体测试
void pipe_test2(void);



int main(void){
	printf("管道测试1\n");
	pipe_test();

	sleep(1);//暂停2 秒, 防止粘合

	printf("\n\n\n管道测试2-传输结构体测试\n");
	pipe_test2();

	return 0;
}



//1.管道测试
void pipe_test(void){
	pid_t main_pid = getpid();
	int ppfd[2];
	int tmp;
	pipe_msg_t rbuf,wbuf;



	//1.创建管道fd 组
	tmp = pipe(ppfd);
	if(tmp == -1){
		perror("pipe() failed");
		return ;
	}


	//2.测试管道的读写能力
	tmp = fork();
	if(tmp == -1){
		perror("fork() failed");
		return ;
	}
	if(tmp == 0){
		//子进程写
		close(ppfd[0]);//关闭管道ppfd[0]读端

		memset(&wbuf,'\0',sizeof(wbuf));
		wbuf.head.type = 1;//缓冲区类型赋值
		strncpy(wbuf.buf,"hello pipe!!",PIPE_BUF);//截断copy, 缓冲区赋值
		wbuf.head.buf_len = strlen(wbuf.buf);//缓冲区长度赋值


		//向管道写入数据
		//如果当前管道不可用, 则阻塞等待到可用为止.
		tmp = my_pipe_write(ppfd[1],&wbuf);
		if(tmp >= 0)
			printf("my_pipe_write() written size = %d\n",tmp);
		if(tmp == -1)
			perror("my_pipe_write() failed");

		close(ppfd[1]);//关闭ppfd[1]写端
		exit(0);//子进程安全推出程序
		// NOTREACHED //
	}
	else{
		usleep(3000);
		//父进程读
		close(ppfd[1]);//关闭管道ppfd[1]写端


		//等待读取'管道'的数据, 如果当前没有数据可读, 则阻塞.
		//所以这也不需要担心会错过.
		memset(&rbuf,'\0',sizeof(rbuf));
		tmp = my_pipe_read(ppfd[0],&rbuf);
		if(tmp >= 0)
			printf("my_pipe_read() read size = %d\n",tmp);
		if(tmp == -1)
			perror("read() failed");

		//打印成功读取的结果(以字符串为例)
		printf("rbuf type:%d, rbuf buf_len:%d\n",\
				rbuf.head.type,rbuf.head.buf_len);
		printf("rbuf result: %s\n",rbuf.buf);

		close(ppfd[0]);//关闭ppfd[0]读端
	}

	return ;
}



//2.
//pipe write - 需要填写正确"pipe_msg_h"
//向管道写入数据
//如果当前管道不可用, 则阻塞等待到可用为止.
int my_pipe_write(int pipe_fd, pipe_msg_t *pdata){
	int tmp,count = 0;



	//检查长度是否出错, 免得写入出问题
	if(pdata->head.buf_len > PIPE_BUF-16 || pdata->head.buf_len <= 0){
		printf("my_pipe_write(): head.buf_len error = %d\n",pdata->head.buf_len);
		return -1;
	}

	//先写入头
	tmp = write(pipe_fd,(void*)&pdata->head,sizeof(pipe_msg_h));
	if(tmp >= 0)
		printf("my_pipe_write(): written size = %d\n",tmp);
	if(tmp == -1)
		perror("write() failed");

	count += tmp;

	//再写入buf, 这样做可以减少字节流写入
	tmp = write(pipe_fd,(void*)pdata->buf,pdata->head.buf_len);
	if(tmp >= 0)
		printf("my_pipe_write(): written size = %d\n",tmp);
	if(tmp == -1)
		perror("write() failed");

	return count += tmp;
}



//3.
//pipe read - 需要传入pipe_msg_t实体, 进行引用调用
//等待读取'管道'的数据, 如果当前没有数据可读, 则阻塞.
//所以这也不需要担心会错过.
int my_pipe_read(int pipe_fd, pipe_msg_t *pdata){
	int tmp,count = 0;



	//先读取头
	tmp = read(pipe_fd,(void*)&pdata->head,sizeof(pipe_msg_h));
	if(tmp >= 0)
		printf("my_pipe_read(): read size = %d\n",tmp);
	if(tmp == -1)
		perror("read() failed");

	//检查长度是否出错, 免得写入出问题
	if(pdata->head.buf_len > PIPE_BUF-16 || pdata->head.buf_len <= 0){
		printf("my_pipe_read(): head.buf_len error = %d\n",pdata->head.buf_len);
		return -1;
	}

	count += tmp;

	//再读取数据体
	tmp = read(pipe_fd,(void*)pdata->buf,pdata->head.buf_len);
	if(tmp >= 0)
		printf("my_pipe_read(): read size = %d\n",tmp);
	if(tmp == -1)
		perror("read() failed");

	return count += tmp;
}









//4.管道测试2-传输结构体测试
void pipe_test2(void){
	pid_t main_pid = getpid();
	int ppfd[2];
	int tmp;
	pipe_msg_t rbuf,wbuf;
	test_t rtest,wtest;



	//1.创建管道fd 组
	tmp = pipe(ppfd);
	if(tmp == -1){
		perror("pipe() failed");
		return ;
	}


	//2.测试管道的读写能力
	tmp = fork();
	if(tmp == -1){
		perror("fork() failed");
		return ;
	}
	if(tmp == 0){
		//子进程写
		close(ppfd[0]);//关闭管道ppfd[0]读端


		wtest.x3 = 999;//填充测试结构体
		strncpy(wtest.x2,"22222222222222!!",sizeof(wtest.x2));
		strncpy(wtest.x1,"11111111111111!!",sizeof(wtest.x1));

		memset(&wbuf,'\0',sizeof(wbuf));
		wbuf.head.type = 1;//缓冲区类型赋值
		wbuf.head.buf_len = sizeof(wtest);//缓冲区长度赋值
		memcpy(&wbuf.buf,&wtest,sizeof(wtest));


		//向管道写入数据
		//如果当前管道不可用, 则阻塞等待到可用为止.
		tmp = my_pipe_write(ppfd[1],&wbuf);
		if(tmp >= 0)
			printf("my_pipe_write() written size = %d\n",tmp);
		if(tmp == -1)
			perror("my_pipe_write() failed");

		close(ppfd[1]);//关闭ppfd[1]写端
		exit(0);//子进程安全推出程序
		// NOTREACHED //
	}
	else{
		usleep(5000);
		//父进程读
		close(ppfd[1]);//关闭管道ppfd[1]写端


		//等待读取'管道'的数据, 如果当前没有数据可读, 则阻塞.
		//所以这也不需要担心会错过.
		memset(&rbuf,'\0',sizeof(rbuf));
		tmp = my_pipe_read(ppfd[0],&rbuf);
		if(tmp >= 0)
			printf("my_pipe_read() read size = %d\n",tmp);
		if(tmp == -1)
			perror("read() failed");

		//打印成功读取的结果(以字符串为例)
		printf("rbuf type:%d, rbuf buf_len:%d\n",\
				rbuf.head.type,rbuf.head.buf_len);
		memcpy(&rtest,&rbuf.buf,rbuf.head.buf_len);
		printf("rbuf result:\nx1=%s\nx2=%s\nx3=%d\n",\
				rtest.x1,rtest.x2,rtest.x3);

		close(ppfd[0]);//关闭ppfd[0]读端
	}

	return ;
}
