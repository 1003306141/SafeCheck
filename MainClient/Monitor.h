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

//�ؼ���ƥ��ģʽ
enum MatchMode
{
	OnceMatch,
	AllMatch
};

//�ϴ����������Ķ�̬������Ϣ��ʽ
struct MonitorLog
{
	char time[100];//2019-05-02 10:10
	char file_hash[32];//11111111111111111111111111111111
	char key_info[20];//3-����-1-16:3-����-1-24:3-����-1-32
	char content[200];//����_����_����
	char file_path[200];//C:\\USER\\Hello\\Wordl\\A.doc
};

//�ļ��ؼ�����Ϣ
struct KeywordsInfo
{
	MatchMode	match_mode;

	int			need_match_count;
	char*		need_match_keyword[100];
	int			need_match_keyword_rank[100];

	int			match_count;
	char*		match_keyword[100];
	int			match_keyword_rank[100];

	int			match_keyword_repeat_time[100];

	int			match_keyword_position[100][100];

	char*		match_keywords_summary[100][100];
};

//�洢�ļ���Ϣ�ṹ��
struct FileInfo
{
	FileType	file_type;
	char		file_name[MAX_PATH];
	char		file_path[3000];
	int			file_size;
	SYSTEMTIME	file_access_time;

	char*		file_content;

	KeywordsInfo keywords_info;
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



//------------------------------------------------------------------------------------
int Kmp_MainStrstr(KeywordsInfo * information, const char *filecontent);
int Kmp_OnceStrStr(int i, KeywordsInfo * information, const char *filecontent);
int Kmp_AllStrStr(KeywordsInfo * information, const char *filecontent, int i, int j);
void Kmp_getNextArray(const char * match, int *next);
void Kmp_DeleteAllMem(KeywordsInfo * Information);

int Kmp_getSummary_One(KeywordsInfo * information, int res, const char *filecontent, int j);

int Kmp_getSummart_All(KeywordsInfo * information, const char *filecontent, int j);