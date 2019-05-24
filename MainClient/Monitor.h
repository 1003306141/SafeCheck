#pragma once
#include <Windows.h>
#include <stdio.h>

//扫描间隔时间，单位分钟
#define MonitorTime 1

//需要转换的文件类型
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

//关键字匹配模式
enum MatchMode
{
	OnceMatch,
	AllMatch
};

//上传到服务器的动态报警信息格式
struct MonitorLog
{
	char time[100];//2019-05-02 10:10
	char file_hash[32];//11111111111111111111111111111111
	char key_info[20];//3-机密-1-16:3-秘密-1-24:3-绝密-1-32
	char content[200];//机密_秘密_绝密
	char file_path[200];//C:\\USER\\Hello\\Wordl\\A.doc
};

//文件关键字信息
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

//存储文件信息结构体
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

//用于存储从本地文件读取到的关键字信息
struct KeyWord
{
	int count;
	char* key[100];
	int rank[100];
};


//定时器回调函数
VOID CALLBACK TimerProc(
	UINT      uTimerID,
	UINT      uMsg,
	DWORD_PTR dwUser,
	DWORD_PTR dw1,
	DWORD_PTR dw2);

//扫描线程函数
DWORD _stdcall MonitorStart(LPVOID);

//在磁盘中搜索新文件并处理
void FindNewFile(const char* path);

//判断是否为新且特定类型文件
bool IsTheFile(WIN32_FIND_DATA file_data);

//判断是否为最新更改过的文件
bool IsNewFile(FILETIME findfile_time);

//判断是否为特定文件类型
bool IsTypeFile(const char file_name[MAX_PATH]);

//获取文件类型
enum FileType GetFileType(const char file_name[MAX_PATH]);

//在特定文件中查找关键字
bool FindKeyword(FileInfo* file);

//从压缩包提取文件
void ExtractFile(FileInfo* file);

//文件格式转换函数，将特定类型转换为TXT
void Translate2Txt(FileInfo* file);

//生成日志文件
void CreateMonitorLog();

//开始全盘扫描
void StartScanAllDisk();

//获取全部磁盘名称
void GetAllDiskName(char* alldisk_name);

//获取磁盘数量
int GetDiskCount();

//获取本地关键字
bool GetLocalKeyword(KeyWord* keyword);

//UTF8转GBK
void UTF8ToGBK(char* strUTF8);



//------------------------------------------------------------------------------------
int Kmp_MainStrstr(KeywordsInfo * information, const char *filecontent);
int Kmp_OnceStrStr(int i, KeywordsInfo * information, const char *filecontent);
int Kmp_AllStrStr(KeywordsInfo * information, const char *filecontent, int i, int j);
void Kmp_getNextArray(const char * match, int *next);
void Kmp_DeleteAllMem(KeywordsInfo * Information);

int Kmp_getSummary_One(KeywordsInfo * information, int res, const char *filecontent, int j);

int Kmp_getSummart_All(KeywordsInfo * information, const char *filecontent, int j);