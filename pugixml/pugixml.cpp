//111111111111111111111111111111111111111111111111111111111111111111111111
//格式测试:
//邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵
//起始日期:
//完成日期:
//************************************************************************
//修改日志:
//	2019-05-13: 新增'tty 文本标准格式'风格
//, , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , ,, , ,

//编译:
//g++ -o x ./pugixml.cpp -lpugixml

//pugixml 生成xml file 的使用建议:
/*

注意: pugixml 里面, xml_node 变量也是指针,
		 但你操作完之后, 一旦更换了对象, 移动了指针,
		 指向了新的对象, 就找不回来以前的对象了.
		 所以操作要一次完成, 且准确.


注意2: xml 并不适合表示太复杂的数据(二维数组以上的复杂数据都不适合)
			一般只能表示一维数组, 或者简单的数据表.
			简单来说, xml 表示的数据形式一般都不会太复杂.
			但xml 的层次可以是多个的, 可以有很多层, 很多层root-X


注意3: (xml 生成树参考例子)
	//生成节点代码:
	pugi::xml_node nodeBooks = nodeRoot.append_child("books");
	for(int32_t i = 0; i < 10; ++i)
	{
			sprintf_s(szBuf, nBufSize, "book_%02d", i);
			pugi::xml_node nodeBook = nodeBooks.append_child("book");
			// 增加属性
			nodeBook.append_attribute("book").set_value(szBuf);
			nodeBook.append_attribute("price").set_value(50 - i);
	}
	//生成节点数据:
	<books>
			<book book="book_00" price="50" />
			<book book="book_01" price="49" />
			<book book="book_02" price="48" />
			<book book="book_03" price="47" />
			<book book="book_04" price="46" />
			<book book="book_05" price="45" />
			<book book="book_06" price="44" />
			<book book="book_07" price="43" />
			<book book="book_08" price="42" />
			<book book="book_09" price="41" />
	</books>

注意4:
	//典型错误!! xml 里面, 任何一个元素, 都不应该有' '空格分隔,
	//必须使用'_'或者'-'分隔, 否则就是错误的节点!
	//son_root_tmp = root_doc.append_child("student score table");

	son_root_tmp = root_doc.append_child("student_score_table");

注意5:
	遇到宽字符, 可以加_T(""), 来解决问题.

*/



#include <iostream>
#include <string.h>
#include <time.h>
#include <cstring>

#include <pugiconfig.hpp>
#include <pugixml.hpp>

#include <sys/types.h>//for stat(), man 2 stat
#include <sys/stat.h>
#include <unistd.h>

using namespace pugi;//使用名字空间pugi
using namespace std;



#define xml_filename "./test_data"	//1.生成/2.解析的xml filename
#define test_count 10000
#define FILE_MAX_PARSER_SAX_XML 1*1024*1024*1024//最大解析1GB的xml文件
//#define FILE_MAX_PARSER_SAX_XML 10


//1.生成一棵的xml 树, 然后将该xml 树导出到file 中保存
void cre_xml_data_dom(void);

//2.解析一个xml 文件, 将解析到的结果打印出来
void parse_xml_dom(char* xml_file_path);

//3.用stat()检查一下路径是否被占用, 顺便读取文件大小.
bool stat_check(char* pfile_name, int* file_size);


int main(void){
	//1.先-生成一棵的xml 树, 然后将该xml 树导出到file 中保存
	cre_xml_data_dom();

	//2.再-解析一个xml 文件, 将解析到的结果打印出来
	parse_xml_dom(xml_filename);

	return 0;
}



//1.生成一棵的xml 树, 然后将该xml 树导出到file 中保存
void cre_xml_data_dom(void){
	xml_document root_doc;//pugi xml 文档载体(相当与pugixml 的总根, 但无节点名的总根)
	xml_node son_pre;//声明子根
	xml_node son_root_tmp,son_node;//pugi xml 子根tmp, 子节点
	char* tmp_id = "id";
	char* tmp_name = "tmp";
	char id_buf[64],name_buf[64];
	int pos;
	FILE *fp;
	xml_writer_file* pxml_fp;//pugixml 文件流导出类



	//1.填充'pugi 声明子根'到总根里(pugi 习惯上都会使用声明节点, 声明版本和编码格式)
	//	令’son_pre节点‘成为'声明子根'.
	son_pre = root_doc.prepend_child(pugi::node_declaration);
	son_pre.append_attribute("version") = "1.0";		//附加属性: xml 版本
	son_pre.append_attribute("encoding") = "utf-8";	//附加属性: xml 字符编码格式


	//2.创建根节点1到'总根'中, 并命名子根节点为student score table

	//典型错误!! xml 里面, 任何一个元素, 都不应该有' '空格分隔,
	//必须使用'_'或者'-'分隔, 否则就是错误的节点!
	//son_root_tmp = root_doc.append_child("student score table");


	son_root_tmp = root_doc.append_child("student_score_table");
	//<id1 name="adan" score="100" />
	son_node = son_root_tmp.append_child("id1");
	son_node.append_attribute("name").set_value("adan");
	son_node.append_attribute("score").set_value(100);

	//<id2 name="shaw" score="99" />
	son_node = son_root_tmp.append_child("id2");
	son_node.append_attribute("name").set_value("shaw");
	son_node.append_attribute("score").set_value(99);

	//大量生产成绩表数据
	for(pos=0;pos<test_count;pos++){
		snprintf(id_buf,64,"%s%d",tmp_id,pos);
		snprintf(name_buf,64,"%s%d",tmp_name,pos);
		son_node = son_root_tmp.append_child(id_buf);
		son_node.append_attribute("name").set_value(name_buf);
		son_node.append_attribute("score").set_value(pos);
	}



	//3.打开文件, 独占起来.
	fp = fopen(xml_filename, "w");
	if(fp == NULL){
		perror("fopen() failed!");
		return;
	}

	// 独占文件方式2(错误的方式, 这个是装载, 是r !! 不是w !!)
	/*
	if(!root_doc.load_file(xml_filename)){
		perror("root_doc.load_file() failed!");
		return;
	}
	*/

	//ps: 文件独占, 是以进程为单位的
	/*
		这不同内存数据独占, 每个进程的文件句柄列表是共享的,
		而内存数据, 只提供一个地址, 所以更苛刻, 内存数据只能单线程独占.

		所以, 即使一个进程内有个多'对同一文件的独占操作',
		但只要一个操作成功获取独占权, 整个进程都会共享这个独占权.
	*/

	//4.保存pugixml 数据到文件中
	//	(为了防止保存失败, 前面第3步, 已经强占了文件的使用权
	//	 强占文件之后, 你可以直接对文件名输出)
	root_doc.save_file(xml_filename, "\t", 1U, pugi::encoding_utf8);
	fclose(fp);



	//5.保存pugixml 数据到FILE 文件流中.
	/*
			pugixml一般情况下, 不支持保存数据到缓冲区中,
			因为你不知道pugixml 生成的数据, 具体缓冲区大小是多大.

			但是pugixml 提供root_doc.save()函数,将数据推入FILE文件流,
			这样你就不用考虑缓冲区是否会溢出的问题了,
			无论缓冲区长度如何, 都由FILE流自己解决.

			FILE 文件流, 可以是天然的FILE 文件流:stdin, stdout, stderr,
			也可以是socket, FILE* 文件.
			所以如果你生成pugixml 数据用作传输, 根本就不需要保存到缓冲区,
			可以直接写入socket 流.(这个有待测试)

			通过stdin 文件流, 导入数据到缓冲区, 同样会涉及缓冲区溢出问题,
			这种问题passed, 不再讨论, 是个馊主意.
			FILE 缓冲区是不会溢出的, FILE 流会自动冲刷, 缓冲区满了自动冲刷,
			所以FILE 文件流是不需要考虑缓冲区溢出的问题.
	 */
	pxml_fp = new xml_writer_file(stdout);
	root_doc.save(*pxml_fp,"\t", 1U, pugi::encoding_utf8);



	//在eclipse 中, 红色的'类成员函数或者变量', 都是私有变量, 不能调用的.
	//只有绿色的'类成员函数或者变量'才能调用,
	//也就是pugixml 没法访问自身的缓冲区, 也没办法知道自身的缓冲区大小.
	//printf("strlen(root_doc._buffer)=%d\n",strlen(root_doc._buffer));

	//printf("root_doc.hash_value=%d\n",root_doc.hash_value());



	//6.清空root_doc
	//root_doc.~xml_document();//~xml_document() 里面有_destroy() 函数
														 //pugixml 不需要自己进行释放

	//root_doc.reset();//重新初始化. 这不是释放.
										 //(勿用, 重新装载一次数据, 有何用？)

	//判断root_doc 总根是否是empty
	if(root_doc.empty())
		printf("root_doc is empty already!!\n");//测试root_doc 是否已经清空
	else
		printf("root_doc is not empty!!\n");



	//7.打印一个pugixml 树
	//	(失败, pugixml 没有自动打印全树的方案,
	//	 不过最简单的, 可以直接将pugixml 导出到stdout / stderr 文件流,
	//	 这样就自动打印pugixml 了)
	delete pxml_fp;
	pxml_fp = new xml_writer_file(stderr);
	root_doc.save(*pxml_fp,"\t", 1U, pugi::encoding_utf8);
	delete pxml_fp;

	return;
}



//2.解析一个xml 文件, 将解析到的结果打印出来
void parse_xml_dom(char* xml_file_path){
	xml_document root_doc;
	//xml_node son_pre;//声明子根
	struct xml_parse_result root_result;//第一子根结构体(只用来判断load()的结果)
																			//(ps:该结构体带构造函数,会自动f赋值初始化)
	xml_node son_root,son_node;//pugi xml 子根tmp, 子节点
	string node_name,node_val0,node_val1;//节点名,节点值0,节点值1
	int node_val2;
	xml_node_iterator it;//节点-遍历迭代器
	xml_attribute_iterator ait;//属性-遍历迭代器
	string val3,val4;
	int filesize;



	//0.检查文件是否超过额定装载的max 限制
	if(!stat_check(xml_file_path,&filesize)){
		perror("stat_check() failed");
		return;
	}
	if(filesize > FILE_MAX_PARSER_SAX_XML){
		printf("filesize=%d > FILE_MAX_PARSER_SAX_XML=%d !! too big file\n",\
				filesize, FILE_MAX_PARSER_SAX_XML);
		return;
	}


	//1.装载xml 文件(有4 中方式, 返回一个'带构造函数的结构体'实体)
	//	1.1 从可变内存加载root_doc.load_buffer_inplace()
	//	1.2 从固定内存加载root_doc.load_buffer()
	//	1.3 从streams io 流加载root_doc.load()
	//	1.4 从文件中加载root_doc.load_file()
	//	ps: 不需要考虑缓冲区装载溢出的问题,pugixml实现过程会自己考虑这个问题.
	//			但是你可以防止xml 文件过大, 导致程序装载内存亏空, 导致死机
	root_result = root_doc.load_file(xml_file_path);
	if(root_result.status != status_ok){
		printf("root_doc.load_file() failed, son_root->status=%d",\
				root_result.status);
		return;
	}


	//2.读取声明子根(错误, 不可读取)
	//son_pre = root_doc.child(pugi::node_declaration);

	//2.读取普通子根(错误,xml节点命名,不能用空格间隔,任何一个节点都不能带空格命名)
	//son_root = root_doc.child("student score table");

	//2.读取普通子根(正确,xml节点命名,不能用空格间隔,任何一个节点都不能带空格命名)
	//	ps: 该子根是最大的子根"student_score_table"
	son_root = root_doc.child("student_score_table");
	node_name = son_root.name();
	printf("%s\n",node_name.c_str());


	//3.遍历普通子根
	//<id1 name="adan" score="100" />
	//Get node value, or "" if node is empty or it has no value
	son_node = son_root.child("id1");//由于错乱问题, id1 有两个
	node_name = son_node.name();//获取id1 的名
	//node_val0 = son_node.value();//如果子根只有一个值,不需要展开,即可直接提取值;
																 //如果子根有很多个值,需要展开,不可直接提取值！!
	node_val1 = son_node.attribute("name").value();
	node_val2 = son_node.attribute("score").as_int();
	//node_val2 = son_node.attribute("score").as_double();//double类型例子
	//node_val2 = son_node.attribute("score").as_bool();//bool类型例子
	printf("ZZZ%s: name=%s,score=%d\n",\
			node_name.c_str(),node_val1.c_str(),node_val2);

	son_node = son_root.child("id2");//由于错乱问题, id2 有两个
	node_name = son_node.name();//获取id2 的名
	node_val1 = son_node.attribute("name").value();
	node_val2 = son_node.attribute("score").as_int();
	printf("ZZZ%s: name=%s,score=%d\n",\
			node_name.c_str(),node_val1.c_str(),node_val2);

	//***重点***
	//重名节点,可以通过这个函数,向后移动;
	//next_sibling()表示移动到下一个叫"id2"的节点,
	//因为正常情况下, 上一个id2=shaw, 的下一个节点, 是id0=tmp0, 而不是id2=tmp2
	//所以, next_sibling() 是重名专属函数！！
	son_node = son_node.next_sibling("id2");
	node_name = son_node.name();//获取id2 的名
	node_val1 = son_node.attribute("name").value();
	node_val2 = son_node.attribute("score").as_int();
	printf("ZZZ%s: name=%s,score=%d\n",\
			node_name.c_str(),node_val1.c_str(),node_val2);

	son_node = son_root.child("id3");//id3以上,或其它id的,只有唯一一个
	node_name = son_node.name();//获取id3 的名
	node_val1 = son_node.attribute("name").value();
	node_val2 = son_node.attribute("score").as_int();
	printf("ZZZ%s: name=%s,score=%d\n",\
			node_name.c_str(),node_val1.c_str(),node_val2);


	//4.迭代遍历-子根(不能对总根"student_score_table"进行遍历)
	//	要屏蔽遍历结果, 可以用./x | grep ZZZ, 查看上面的测试!
	for(it=son_root.begin(); it!=son_root.end(); ++it){
		node_name = it->name();//打印节点名


		//4.1 自动遍历属性--不好用, 无法确定属性类型
		for(ait=it->attributes_begin(); ait!=it->attributes_end(); ++ait){
			//ait->value()获取属性值, 不知道类型, 默认是字符串!但是数字不能这么做
			//相反, ait->name()就肯定是string !! 名称肯定是string !
			printf("AAA%s=%s, ",ait->name(),ait->value());
		}
		printf("\n");


		//4.2 手动遍历属性
		ait=it->attributes_begin();//指向it 字节点的第一个属性

		val3 = ait->name();//"name" 属性名
		node_val1 = ait->as_string();//"adan" 属性值

		ait->next_attribute();//移动到下一个属性(迭代器移动, 跟指针移动不一样!)
													//迭代器移动, 不能ait = ait->next_attribute();

		val4 = ait->name();//"score" 属性名
		node_val2= ait->as_int();//99 属性值

		//打印
		printf("===%s: %s=%s,%s=%d\n",\
				node_name.c_str(),val3.c_str(),node_val1.c_str(),\
				val4.c_str(),node_val2);

	}
	return;
}



//用stat()检查一下路径是否被占用, 顺便读取文件大小.
bool stat_check(char* pfile_name, int* file_size){
	struct stat stat_buf;
	int ret;
	char ptimebuf[64];//打印修改时间的buf



	ret = stat(pfile_name, &stat_buf);
	if(ret == -1){
		//如果stat() 出错, 分析出错原因
		perror("Problem getting information, stat() failed");
		switch(errno){
			case ENOENT:
				printf("File %s not found.\n", pfile_name);
				break;
			case EINVAL:
				printf("Invalid parameter to stat().\n");
				break;
		 default:
			 printf("Unexpected error in stat().\n");
		}
		return false;
	}
	else{
		//如果stat() 没有出错, 打印stat() 结果
		//Output some of the statistics(输出一些统计数据):
		printf("File size 文件大小: %ld\n", stat_buf.st_size);
		printf("Drive 驱动标记: %c:\n", stat_buf.st_dev + 'A');
		printf("st_dev: %lx, st_ino: %lx\n", \
					 (u_long) stat_buf.st_dev, (u_long) stat_buf.st_ino);

		//打印-最近修改时间
		memset(ptimebuf,'\0',sizeof(ptimebuf));
		//ctime_s() 是c11 特有, 线程安全 + 缓冲区安全
		//ctime_s(timebuf, 26, &stat_buf.st_mtime);
		ctime_r(&stat_buf.st_mtime, ptimebuf);//自己保障缓冲区不会溢出
		printf("Time modified 修改时间: %s\n", ptimebuf);

		*file_size = stat_buf.st_size;//取值文件大小
		return true;
	}

	/* NOTREACHED */
}
