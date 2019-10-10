//111111111111111111111111111111111111111111111111111111111111111111111111
//格式测试:
//邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵
//起始日期:
//完成日期:
//************************************************************************
//修改日志:

//, , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , ,, , ,


//编译:
//g++ -g3 -o x ./usock_pair.cpp


//socketpair()创建了一对'无名的'套接字描述符(只能在AF_UNIX域中使用)
/*
	Linux实现了一个源自BSD的socketpair调用,
	在Linux中,socketpair() SOCK_STREAM模式时, 类似于pipe,
	唯一的区别就是这一对文件描述符中的任何一个都可读和可写.

函数介绍：
	socketpair()函数建立一对匿名的已经连接的套接字,
							其特性由协议族d、类型type、协议protocol决定,
							建立的两个套接字描述符会放在sv[0]和sv[1]中.
	成功返回0, 失败返回-1.

参数介绍：
	第1个参数d, 表示协议族, 只能为AF_LOCAL或者AF_UNIX;
	第3个参数protocol, 表示协议, 只能为0.
	第2个参数type, 表示类型, 可以是SOCK_STREAM或者SOCK_DGRAM.
		SOCK_STREAM = 无数据边界的数据流, 类似于pipe
		SOCK_DGRAM = 有数据边界的数据报(一般不会用)

	用SOCK_STREAM建立的'套接字对'是: 管道流;
	与'普通管道'相区别的是:
		套接字对建立的通道是双向的, 即每一端都可以进行读写.
		参数sv, 是一个引用返回参数, 用于保存建立的套接字对.
		而'普通管道'是单相工, 单向的.


fd描述符存储于一个二元一维数组,例如sv[2].
这对套接字可以进行双工通信, 每一个描述符既可以读也可以写.
这个在同一个进程中也可以进行通信,
向sv[0]中写入, 就可以从sv[1]中读取(只能从sv[1]中读取),
也可以在sv[1]中写入, 然后从sv[0]中读取;

但是, 若没有在0端写入, 而从1端读取,
则1端的读取操作会阻塞, 即使在1端写入, 也不能从1读取, 仍然阻塞;


socketpair 只用做传输数据, 并不能进行数据同步,
socketpair 数据同步也一般不用信号量, 信号量一般面向'共享内存'.
socketpair 数据同步,一般使用信号,因为不是构建临界区,
					 而是通知对方去取数据, 用信号函数比较好.
socketpair 一般只用作fork() 之间通信



*/



#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>



/*
	由于socket默认接收缓冲区是8196, 一般不要超出8196比较恰当;
	而且需要修改socket默认缓冲区大小,
	否则udp socket读报可能会出现缓冲区溢出丢失,
	但tcp socket读流, 是不会丢失, 但也性能不好.
	udp和tcp的不同, socket缓冲区机制, 详情请看tcp/ip 协议详解
	(但一般需要修改缓冲区比较常见)

	unix socket udp, 在单次接受数据的情况下, 造成缓冲区溢出, 丢失数据.
	设计比较麻烦, 一般不用

	unix socket tcp, 如果真的发生溢出, 内核也会尽可能地自动分配内存buf,
	先把数据接收了再说, 反正只要一旦read() 之后就会释放,
	所以内核会尽量维护tcp 传输协议的完整性
*/

#define USOCK_UDP_BUF_MAX 4096
#define USOCK_UDP_PACKAGE_MAX 8192



//usock_udp 消息结构体(一般不支持void* 内嵌, 别浪费心血了.)
struct usock_udp_t{
	short int type;//信息类型编号
	short int buf_len;//信息长读buf_len
	char buf[USOCK_UDP_BUF_MAX];
};



//s1.SO_RCVBUF 设置接收缓冲区(默认上限8kb)
inline bool set_sockopt_revbuf(int *sfd, unsigned int sock_buf_max);
//s2.SO_SNDBUF 设置发送缓冲区(默认上限8kb)
inline bool set_sockopt_sndbuf(int *sfd, unsigned int sock_buf_max);

//1.socketpair() 简单双向通信测试
void socketpair_test(void);

//2.socketpair() 简单双向通信测试 -- 传递数据结构体
void socketpair_test_tcp(void);

//3.socketpair() 简单双向通信测试 -- 一般不会这样用
void socketpair_test_udp(void);



int main(void){
	printf("1.socketpair() 简单双向通信测试\n");
	socketpair_test();
	sleep(1);
	printf("\n\n\n");

	printf("2.socketpair() 简单双向通信测试 -- 传递数据结构体\n");
	socketpair_test_tcp();
	sleep(3);
	printf("\n\n\n");

	printf("3.socketpair() 简单双向通信测试 -- 一般不会这样用\n");
	socketpair_test_udp();

	return 0;
}



//1.socketpair() 简单双向通信测试 -- 只传递字符
void socketpair_test(void){
	int sv[2];//一对无名的套接字描述符
	int tmp;
	pid_t id;
	const char* msg2son = "我是父亲\n";
	const char* msg2father = "我是孩子\n";
	char buf[1024];
	ssize_t size;//接收/发送的数据大小tmp 值



	//1.创建socketpair() socket 对, sv[0],sv[1] socket 关联sfd组
	tmp = socketpair(PF_LOCAL,SOCK_STREAM,0,sv);
	if(tmp < 0){
		perror("socketpair() failed");
		return ;
	}

	//2.执行fork() 创建子进程
	id = fork();
	if(id == -1){
		perror("fork() failed");
		close(sv[0]);
		close(sv[1]);
		return;
	}
	if(id == 0){//子进程
		//close(sv[0]);//关闭一个端, 是sv[0],sv[1]都是可写可读的
		close(sv[1]);//子进程-关闭sv[1]

		while(1){
			//write(sv[1],msg,strlen(msg));
			write(sv[0],msg2father,strlen(msg2father));//阻塞write()
			sleep(1);

			//size = read(sv[1],buf,sizeof(buf)-1);
			size = read(sv[0],buf,sizeof(buf)-1);//阻塞read()
			if(size > 0){
				buf[size] = '\0';
				printf("socketpair_test: read() from 父亲 : %s\n",buf);

				//只接收一个回覆便退出
				close(sv[0]);
				//close(sv[1]);//子进程-关闭sv[1]
				break;
			}
		}//while end
		exit(0);
	}
	else{//父进程
		//close(sv[1]);
		close(sv[0]);//父进程-关闭sv[0]

		while(1){
			//size = read(sv[0],buf,sizeof(buf)-1);
			size = read(sv[1],buf,sizeof(buf)-1);//阻塞read()
			if(size > 0){
				buf[size] = '\0';
				printf("socketpair_test: read() from 孩子 : %s\n",buf);
				sleep(1);
			}

			//write(sv[0],msg,strlen(msg));
			write(sv[1],msg2son,strlen(msg2son));//阻塞write()

			//只接收一个回覆便退出
			close(sv[1]);
			//close(sv[0]);//父进程-关闭sv[0]
			break;
		}//while end
	}

	return;
}





//2.socketpair() 简单双向通信测试 -- 传递数据结构体
void socketpair_test_tcp(void){
	int sv[2];//一对无名的套接字描述符
	int tmp;
	pid_t id;
	char msg2son[] = "我是父亲\n";
	char msg2father[] = "我是孩子\n";
	usock_udp_t mbuf,mbuf2,re_mbuf,re_mbuf2;
	ssize_t size;//接收/发送的数据大小tmp 值



	//0.填充usock_udp_t 数据
	mbuf.type = 1;
	mbuf.buf_len = 1;
	//必须填充'\0', 因为发送的时候, 是将整个struct 发出去的.
	//虽然消耗有点高, 但是控制的好, 可以传递结构体其实也不错.
	memset(mbuf.buf,'\0',sizeof(mbuf.buf));
	strncpy(mbuf.buf,msg2father,sizeof(msg2father));

	mbuf2.type = 1;
	mbuf2.buf_len = 999;
	memset(mbuf2.buf,'\0',sizeof(mbuf2.buf));
	strncpy(mbuf2.buf,msg2son,sizeof(msg2son));


	//1.创建socketpair() socket 对, sv[0],sv[1] socket 关联sfd组
	tmp = socketpair(PF_LOCAL,SOCK_STREAM,0,sv);//SOCK_STREAM 流式
	if(tmp < 0){
		perror("socketpair() failed");
		return ;
	}


	//2.修改接收/读写缓冲区大小
	tmp = 1;
	if(!set_sockopt_sndbuf(&sv[0],tmp) && \
			set_sockopt_sndbuf(&sv[1],tmp) && \
			set_sockopt_revbuf(&sv[0],tmp) && \
			set_sockopt_revbuf(&sv[1],tmp)){
		close(sv[0]);
		close(sv[1]);
		return;
	}



	//3.执行fork() 创建子进程
	id = fork();
	if(id == -1){
		perror("fork() failed");
		close(sv[0]);
		close(sv[1]);
		return;
	}
	if(id == 0){//子进程
		//close(sv[0]);//关闭一个端, 是sv[0],sv[1]都是可写可读的
		close(sv[1]);//子进程-关闭sv[1]

		while(1){
			write(sv[0],&mbuf2,sizeof(mbuf2));//阻塞write()
			write(sv[0],&mbuf2,sizeof(mbuf2));//阻塞write()
			write(sv[0],&mbuf2,sizeof(mbuf2));//阻塞write()
			sleep(1);

			memset(&re_mbuf2,0,sizeof(re_mbuf2));
			size = read(sv[0],&re_mbuf2,sizeof(re_mbuf2)*3);//阻塞read()
			if(size > 0){
				printf("tcp: read() from 父亲 : %s\n",re_mbuf2.buf);
				printf("tcp: type = %d, buf_len = %d\n",re_mbuf2.type,re_mbuf2.buf_len);

				//只接收一个回覆便退出
				close(sv[0]);
				//close(sv[1]);//子进程-关闭sv[1]
				break;
			}
		}//while end
		exit(0);
	}
	else{//父进程
		//close(sv[1]);
		close(sv[0]);//父进程-关闭sv[0]

		while(1){
			memset(&re_mbuf,0,sizeof(re_mbuf));
			size = read(sv[1],&re_mbuf,sizeof(re_mbuf)*2);//阻塞read()
			if(size > 0){
				printf("tcp: read() from 孩子 : %s\n",re_mbuf.buf);
				printf("tcp: type = %d, buf_len = %d\n",re_mbuf.type,re_mbuf.buf_len);
				sleep(1);
			}

			write(sv[1],&mbuf,sizeof(mbuf));//阻塞write()
			write(sv[1],&mbuf,sizeof(mbuf));//阻塞write()
			write(sv[1],&mbuf,sizeof(mbuf));//阻塞write()

			//只接收一个回覆便退出
			close(sv[1]);
			//close(sv[0]);//父进程-关闭sv[0]
			break;
		}//while end
	}

	return;
}





//3.socketpair() 简单双向通信测试 -- 一般不会这样用
void socketpair_test_udp(void){
	int sv[2];//一对无名的套接字描述符
	int tmp;
	pid_t id;
	char msg2son[] = "我是父亲\n";
	char msg2father[] = "我是孩子\n";
	usock_udp_t mbuf,mbuf2,re_mbuf,re_mbuf2;
	ssize_t size;//接收/发送的数据大小tmp 值



	//0.填充usock_udp_t 数据
	mbuf.type = 1;
	mbuf.buf_len = 1;
	//必须填充'\0', 因为发送的时候, 是将整个struct 发出去的.
	//虽然消耗有点高, 但是控制的好, 可以传递结构体其实也不错.
	memset(mbuf.buf,'\0',sizeof(mbuf.buf));
	strncpy(mbuf.buf,msg2father,sizeof(msg2father));

	mbuf2.type = 1;
	mbuf2.buf_len = 999;
	memset(mbuf2.buf,'\0',sizeof(mbuf2.buf));
	strncpy(mbuf2.buf,msg2son,sizeof(msg2son));


	//1.创建socketpair() socket 对, sv[0],sv[1] socket 关联sfd组
	tmp = socketpair(PF_LOCAL,SOCK_DGRAM,0,sv);//SOCK_DGRAM 报式
	if(tmp < 0){
		perror("socketpair() failed");
		return ;
	}


	//2.执行fork() 创建子进程
	id = fork();
	if(id == -1){
		perror("fork() failed");
		close(sv[0]);
		close(sv[1]);
		return;
	}
	if(id == 0){//子进程
		//close(sv[0]);//关闭一个端, 是sv[0],sv[1]都是可写可读的
		close(sv[1]);//子进程-关闭sv[1]

		while(1){
			write(sv[0],&mbuf2,sizeof(mbuf2));//阻塞write()
			sleep(1);

			memset(&re_mbuf2,0,sizeof(re_mbuf2));
			size = read(sv[0],&re_mbuf2,sizeof(re_mbuf2));//阻塞read()
			if(size > 0){
				printf("udp: read() from 父亲 : %s\n",re_mbuf2.buf);
				printf("udp: type = %d, buf_len = %d\n",re_mbuf2.type,re_mbuf2.buf_len);

				//只接收一个回覆便退出
				close(sv[0]);
				//close(sv[1]);//子进程-关闭sv[1]
				break;
			}
		}//while end
		exit(0);
	}
	else{//父进程
		//close(sv[1]);
		close(sv[0]);//父进程-关闭sv[0]

		while(1){
			memset(&re_mbuf,0,sizeof(re_mbuf));
			size = read(sv[1],&re_mbuf,sizeof(re_mbuf));//阻塞read()
			if(size > 0){
				printf("udp: read() from 孩子 : %s\n",re_mbuf.buf);
				printf("udp: type = %d, buf_len = %d\n",re_mbuf.type,re_mbuf.buf_len);
				sleep(1);
			}

			write(sv[1],&mbuf,sizeof(mbuf));//阻塞write()

			//只接收一个回覆便退出
			close(sv[1]);
			//close(sv[0]);//父进程-关闭sv[0]
			break;
		}//while end
	}

	return;
}



//s1.SO_RCVBUF 设置接收缓冲区(默认上限8kb)
inline bool set_sockopt_revbuf(int *sfd, unsigned int sock_buf_max){
	if(setsockopt(*sfd, SOL_SOCKET, SO_RCVBUF, &sock_buf_max,\
			sizeof(unsigned int)) == -1){
		perror("set_sockopt_revbuf() failed");
		return false;
	}
	else
		return true;
}
//s2.SO_SNDBUF 设置发送缓冲区(默认上限8kb)
inline bool set_sockopt_sndbuf(int *sfd, unsigned int sock_buf_max){
	if(setsockopt(*sfd, SOL_SOCKET, SO_SNDBUF, &sock_buf_max,\
			sizeof(unsigned int)) == -1){
		perror("set_sockopt_sndbuf() failed");
		return false;
	}
	else
		return true;
}
