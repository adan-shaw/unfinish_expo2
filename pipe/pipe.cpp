//111111111111111111111111111111111111111111111111111111111111111111111111
//格式测试:
//邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵
//起始日期:
//完成日期:
//************************************************************************
//修改日志:

//, , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , ,, , ,

//编译:
//g++ -o x ./pipe.cpp -g3


//ps: 管道是流式ipc 通信工具, 并没有消息边界, 一般只能用作传输字符串.
//		如果需要传输变量, 可能需要json,xml 等做"变量2字符串"的转换.


/*
1.管道概述:
	pipe作为linux中最基础的进程间通信机制, 经常在shell中使用,
	例如:
		ps aux | grep aaa # 即建立了一个管道;
	而linux下C程序同样可以通过系统调用pipe()在'父子进程间'使用管道功能.


2.管道pipefd[2] 引用参数:
	pipe()函数通过引用参数, 返回两个描述符fd,
	pipefd[0]用来读,pipefd[1]用来写;(反过来不行,管道不能反向!)
	写入到pipefd[1]的数据将可以从pipefd[0]读出,
	管道作为半双工通道, 数据只能沿一个方向前进;

	pipe函数返回0表示调用成功, 返回-1表示调用失败;


3.管道阻塞问题:
	读取一个空的pipe将导致read()操作阻塞, 直到有数据被写入到pipe中;

	向一个已经满的pipe写数据将导致write()操作阻塞,
	直到pipe中的数据被读取出去;

	如果想避免read()和write()阻塞,
	可通过fcntl()将pipefd设置成O_NONBLOCK,
	read()和write()无论是否成功, 都将直接返回,
	这时需要判断read()和write()返回错误码, 判断操作是否成功.


4.管道数据粒度的重要性:
	(缓冲区额定长度为: PIPE_BUF, 是limit限制.)
	如果有多个进程同时向一个pipe写入时,
	只有在每个进程写入的数据长度都小于PIPE_BUF时,
	才可以保证pipe写入的原子性, 不然可能会出现数据错乱的情况;

	如果写入数据长度, 超出了PIPE_BUF,
	内核会对写入的数据分片传输, write调用保持阻塞,
	直到管道的读取端从该管道中移除一部分数据.
	(这样就不能保证操作的原子性!!)
	而且, 如果管道数据被内核分片, 你不会知道具体分片的长度的.
	这个分片长度由内核决定, 有可能是动态分片的.


5.管道的双向通信的实现:
	管道作为半双工通道,如果想实现双向通信,
	则需要打开两个管道,一个从父进程->子进程,另一个从子进程->父进程.


6.我对shell 管道的实现, 与c 语言管道的实现的'关联':
	管道|前后, 可能是子父进程的关系, 管道前面的进程, 应该是父进程;
	前面的父进程会调用wait()等待子进程结束才会结束自身.

	shell 有多少个管道, 就会挟持创建多少个子进程, 孙进程.
	以ls | grep png 为例:
		ls 是一个回显命令, 在ls 返回'回显字符buf'之后,
		下一步'ls命令所在的父进程'快要结束的时候,
		如果判断'后面还有 | 管道命令'的时候, 则选择不结束父进程.
		而是fork() && wait() 等待子进程结束, 才返回.
		这样丝毫不影响'最终父进程退出', 只是耽搁了一点时间而已.

	附: 管道越多,子进程越多,孙进程越多.
			如果不是父子进程关系,但是仍然想用管道.可以考虑有名管道.
*/



#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>



//管道测试
void pipe_test(void);



int main(void){
	//管道测试
	pipe_test();

	return 0;
}



//管道测试
void pipe_test(void){
	pid_t main_pid = getpid();
	int ppfd[2];
	int tmp;
	char readbuf[PIPE_BUF],writebuf[PIPE_BUF];//PIPE_BUF=4096



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

		memset(writebuf,'\0',PIPE_BUF);
		strncpy(writebuf,"hello pipe!!",PIPE_BUF);//截断copy

		//向管道写入数据
		//如果当前管道不可用, 则阻塞等待到可用为止.
		tmp = write(ppfd[1],writebuf,strlen(writebuf)+1);
		if(tmp >= 0)
			printf("son written size = %d\n",tmp);
		if(tmp == -1)
			perror("write() failed");

		close(ppfd[1]);//关闭ppfd[1]写端
	}
	else{
		usleep(1000);
		//父进程读
		close(ppfd[1]);//关闭管道ppfd[1]写端


		//等待读取'管道'的数据, 如果当前没有数据可读, 则阻塞.
		//所以这也不需要担心会错过.
		memset(readbuf,'\0',PIPE_BUF);
		tmp = read(ppfd[0],readbuf,PIPE_BUF);
		if(tmp >= 0)
			printf("main process read size = %d\nmsg: %s\n",tmp,readbuf);
		if(tmp == -1)
			perror("read() failed");

		close(ppfd[0]);//关闭ppfd[0]读端
	}

	return ;
}
