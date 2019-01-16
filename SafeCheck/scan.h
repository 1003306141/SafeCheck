#pragma once
#include <Windows.h>
#include <tlhelp32.h>
#include <ShlObj.h>
#include <stdio.h>
#include <time.h>
#include "userface.h"


#pragma comment  (lib,"User32.lib")
#pragma comment  (lib,"Gdi32.lib")
#pragma comment	 (lib, "shell32.lib")

//**********对扫描功能的声明**************
struct MyFile
{
	int type;
	char name[201];			//文件名
	char path[3001];		//文件路径
	char key[21];			//文件关键字
	char comment[1025];		//文件关键字内容
};
class Scaner
{
public:
	//获取磁盘数量
	static int getdiskcount();

	//获取磁盘名称
	static void getdiskname(char* diskname);

	//获取我的文档，桌面路径
	static void getpath(char* path1, char* path2);

	//查找指定类型的文件并将找到的文件路径保存在制定目录的文件中
	static void myfindfile(const char* path);

	//获取文件后缀类型
	static int getfiletype(char filename[MAX_PATH]);

	//全盘查找
	static void alldiskscan();

	//转换特定格式文件到txt
	static void all2txt(MyFile* file);

	//快速查找
	static void fastscan();

	//文本中查找字符串
	static int findstr(MyFile* file);

	//判断all2txt进程是否存在
	static int CheckProcess();

	static void ChangeFileName(int type);
};


