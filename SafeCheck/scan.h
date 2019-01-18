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


//**********�ؼ��ֽṹ��**************
struct MyKey
{
	int count;
	char* Key[100];
};

//**********��ɨ�蹦�ܵ�����**************
struct MyFile
{
	int type;
	char name[201];			//�ļ���
	char path[3001];		//�ļ�·��
	char key[21];			//�ļ��ؼ���
	char comment[4096];		//�ļ��ؼ�������
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
	static int findstr(MyFile* file);

	//�ж�all2txt�����Ƿ����
	static int CheckProcess();

	//����ɨ��ʱ�����־�ļ�
	static void CreateLog(int type);

	//���¹ؼ���
	static void GetKeyConfig();

};


//������Щ���͵�Ŀ¼�����ļ�ѡ������
/*
��Ŀ¼�ࡿ
FILE_ATTRIBUTE_HIDDEN		����Ŀ¼
FILE_ATTRIBUTE_REPARSE_POINT	��ݷ�ʽĿ¼
FILE_ATTRIBUTE_SYSTEM		ϵͳĿ¼
���ļ��ࡿ
FILE_ATTRIBUTE_TEMPORARY		��ʱ�ļ�
FILE_ATTRIBUTE_SYSTEM		ϵͳ�ļ�
*/

