#pragma once
#include <Windows.h>
#include <stdio.h>
#include <ShlObj.h>
#include <time.h>
#include "userface.h"


#pragma comment  (lib,"User32.lib")
#pragma comment  (lib,"Gdi32.lib")
#pragma comment	 (lib, "shell32.lib")

//**********��ɨ�蹦�ܵ�����**************
struct MyFile
{
	int type;
	char path[3000];
};
class Scaner
{
public:
	//��ȡ��������
	static int getdiskcount();

	//��ȡ��������
	static void getdiskname(char* diskname);

	//��ȡ�ҵ��ĵ�������·��
	static void getpath(char* path1, char* path2);

	//����ָ�����͵��ļ������ҵ����ļ�·���������ƶ�Ŀ¼���ļ���
	static void myfindfile(const char* path);

	//��ȡ�ļ���׺����
	static int getfiletype(char filename[MAX_PATH]);

	//ȫ�̲���
	static void alldiskscan();

	//ת���ض���ʽ�ļ���txt
	static void all2txt(MyFile* file);

	//���ٲ���
	static void fastscan();

	//�ı��в����ַ���
	static void findstr(MyFile* file);
};

