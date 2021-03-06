#pragma once
#include <Windows.h>
#include <tlhelp32.h>
#include <ShlObj.h>
#include <stdio.h>
#include <time.h>
#include <vector>
using namespace std;

#pragma comment  (lib,"User32.lib")
#pragma comment  (lib,"Gdi32.lib")
#pragma comment	 (lib, "shell32.lib")

//**********关键字结构体**************
struct MyKey
{
	int count;
	char* Key[100];
	int rank[100];
};

//**********文件信息结构体**************
struct MyFile
{
	int type;
	char name[201];			//文件名
	char path[3001];		//文件路径
	char key[21];			//文件关键字
	char comment[4096];		//文件关键字内容
	int size;				//文件大小
	int rank;				//关键字级别
	int position;			//关键字位置（GB2312编码下关键字位置）

	//日志文件格式
	//路径|关键字级别-关键字-重复次数-关键字位置:文件大小|关键字上下文
};
class Scaner
{

	//**********对扫描功能的声明**************
public:
	//获取磁盘数量
	static int getdiskcount();

	//获取磁盘名称
	static void getdiskname(char* diskname);

	//获取我的文档，桌面路径
	static void getpath(char* path1, char* path2);

	//遍历文件函数，找到特定类型文件并屏蔽无关文件和目录
	static void myfindfile(const char* path);

	//获取文件后缀类型
	static int getfiletype(char filename[MAX_PATH]);

	//全盘快速AAAAAAAAAAAAAAA
	static void alldiskscan();

	//全盘普通
	static void alldiskscannormal();

	//转换特定格式文件到txt
	static void all2txt(MyFile* file);

	//快速查找
	static void fastscan();

	//文本中查找字符串
	static int findstr(MyFile* file);

	//生成扫描时间的日志文件
	static void CreateLog(int type);

	//更新关键字
	static void GetKeyConfig();
};


//遇到这些类型的目录或者文件选择跳过
/*
【目录类】
FILE_ATTRIBUTE_HIDDEN		隐藏目录
FILE_ATTRIBUTE_REPARSE_POINT	快捷方式目录
FILE_ATTRIBUTE_SYSTEM		系统目录
【文件类】
FILE_ATTRIBUTE_TEMPORARY		临时文件
FILE_ATTRIBUTE_SYSTEM		系统文件
FILE_ATTRIBUTE_HIDDEN		隐藏文件
*/

string GBKToUTF8(const char* strGBK);