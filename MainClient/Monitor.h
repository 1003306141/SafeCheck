#pragma once
#include <Windows.h>
#include <stdio.h>

//ɨ����ʱ�䣬��λ����
#define MonitorTime 1

//��Ҫת�����ļ�����
enum FileType
{
	DOC,
	DOCX,
	XLS,
	XLSX,
	PPT,
	PPTX,
	PDF,
	WPS,
	ZIP,
	RAR,
	Z7//7Z
};

//�ϴ����������Ķ�̬������Ϣ��ʽ
struct MonitorLog
{
	char time[100];//2019-05-02 10:10
	char file_hash[50];//11111111111111111111111111111111
	//char key_info[20];//3-����-1-16:3-����-1-24:3-����-1-32

	int rank;
	char key[20];
	int position;

	char content[200];//����_����_����
	char file_path[200];//C:\\USER\\Hello\\Wordl\\A.doc

	char buffer[2000];
};

//�洢�ļ���Ϣ�ṹ��
struct FileInfo
{
	FileType	file_type;
	char		file_name[MAX_PATH];
	char		file_path[3000];
	int			file_size;
	SYSTEMTIME	file_access_time;

	MonitorLog log_message;
};

//���ڴ洢�ӱ����ļ���ȡ���Ĺؼ�����Ϣ
struct KeyWord
{
	int count;
	char* key[100];
	int rank[100];
};


//��ʱ���ص�����
VOID CALLBACK TimerProc(
	UINT      uTimerID,
	UINT      uMsg,
	DWORD_PTR dwUser,
	DWORD_PTR dw1,
	DWORD_PTR dw2);

//ɨ���̺߳���
DWORD _stdcall MonitorStart(LPVOID);

//�ڴ������������ļ�������
void FindNewFile(const char* path);

//�ж��Ƿ�Ϊ�����ض������ļ�
bool IsTheFile(WIN32_FIND_DATA file_data);

//�ж��Ƿ�Ϊ���¸��Ĺ����ļ�
bool IsNewFile(FILETIME findfile_time);

//�ж��Ƿ�Ϊ�ض��ļ�����
bool IsTypeFile(const char file_name[MAX_PATH]);

//��ȡ�ļ�����
enum FileType GetFileType(const char file_name[MAX_PATH]);

//���ض��ļ��в��ҹؼ���
bool FindKeyword(FileInfo* file);

//��ѹ������ȡ�ļ�
void ExtractFile(FileInfo* file);

//�ļ���ʽת�����������ض�����ת��ΪTXT
void Translate2Txt(FileInfo* file);

//������־�ļ�
void CreateMonitorLog();

//��ʼȫ��ɨ��
void StartScanAllDisk();

//��ȡȫ����������
void GetAllDiskName(char* alldisk_name);

//��ȡ��������
int GetDiskCount();

//��ȡ���عؼ���
bool GetLocalKeyword(KeyWord* keyword);

//UTF8תGBK
void UTF8ToGBK(char* strUTF8);



