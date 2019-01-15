#include "scan.h"

#define NAME1 "机密"
#define NAME2 "秘密"
#define NAME3 "绝密"

#define TwoAll2txt 1
#define OneAll2txt 0

//快速扫描线程
DWORD _stdcall ThreadFastScan(LPVOID path)
{
	char path1[MAX_PATH] = { 0 };
	char path2[MAX_PATH] = { 0 };
	Scaner::getpath(path1, path2);
	Scaner::myfindfile(path1);
	Scaner::myfindfile(path2);
	return 0;
}

//全盘扫描线程
DWORD _stdcall ThreadAllScan(LPVOID path)
{
	Scaner::myfindfile((char*)path);
	return 0;
}

//全盘扫描调用函数
void Scaner::alldiskscan()
{
	clock_t start, end;
	start = clock();
	char* diskname = (char*)malloc(2 * 32);
	memset(diskname, 0, 32 * 2);
	getdiskname(diskname);

	HANDLE* hThread = (HANDLE*)malloc(sizeof(HANDLE)*getdiskcount());
	for (int i = 0; i < getdiskcount(); i++)
	{
		char* path = (char*)malloc(3);
		memset(path, 0, 3);
		*(path + 0) = diskname[i * 2];
		*(path + 1) = ':';
		*(path + 2) = '\0';
		hThread[i] = CreateThread(NULL, 0, ThreadAllScan, (LPVOID)path, 0, NULL);
	}
	WaitForMultipleObjects(getdiskcount(), hThread, TRUE, INFINITE);
	end = clock();
	MessageBoxA(NULL,"全盘扫描成功","成功",0);
	printf("全盘扫描所用时间：%f\n", (double)(end - start) / CLK_TCK);
	for (int i = 0; i < getdiskcount(); i++)
		int ret = CloseHandle(hThread[i]);
	free(hThread);
}

//快速扫描调用函数
void Scaner::fastscan()
{
	HANDLE hThread = CreateThread(NULL, 0, ThreadFastScan, NULL, 0, NULL);
	WaitForSingleObject(hThread,INFINITE);
	MessageBoxA(NULL, "常用路径扫描完成", "成功", NULL);
}

//获取用户桌面和文档路径
void Scaner::getpath(char* path1, char* path2)
{
	char m_lpszDefaultDir1[MAX_PATH] = { 0 };
	char szDocument1[MAX_PATH] = { 0 };
	memset(m_lpszDefaultDir1, 0, MAX_PATH);

	char m_lpszDefaultDir2[MAX_PATH] = { 0 };
	char szDocument2[MAX_PATH] = { 0 };
	memset(m_lpszDefaultDir2, 0, MAX_PATH);

	LPITEMIDLIST pidl = NULL;
	LPITEMIDLIST pid2 = NULL;

	SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl);
	if (pidl && SHGetPathFromIDList(pidl, szDocument1))
	{
		GetShortPathName(szDocument1, m_lpszDefaultDir1, _MAX_PATH);
	}

	SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pid2);
	if (pid2 && SHGetPathFromIDList(pid2, szDocument2))
	{
		GetShortPathName(szDocument2, m_lpszDefaultDir2, _MAX_PATH);
	}
	strcpy(path1, szDocument1);
	strcpy(path2, szDocument2);
}

//获取磁盘数量
int Scaner::getdiskcount()
{
	int diskcount = 0;
	DWORD diskinfo = GetLogicalDrives();
	if (diskinfo != 0)
	{
		while (diskinfo)
		{
			//通过判断每一位是否为1来确认是否存在该磁盘
			if (diskinfo & 1)
				++diskcount;
			diskinfo = diskinfo >> 1;
		}
	}
	return diskcount;
}

//获取磁盘名称
void Scaner::getdiskname(char* diskname)
{
	UINT dtype;
	char* buf = (char*)malloc(32 * 4);
	memset(buf, 0, 32 * 4);
	GetLogicalDriveStrings(32 * 2, (LPSTR)buf);
	for (int i = 0; i < getdiskcount(); i++)
	{
		char dir[3] = { buf[i * 4],':','\\' };
		dtype = GetDriveType(buf + i * 4);
		//DRIVE_FIXED本地固定磁盘 DRIVE_REMOVABLE可移动磁盘（U盘，移动硬盘） DRIVE_CDROM 光盘
		if (dtype == DRIVE_FIXED || dtype == DRIVE_REMOVABLE || dtype == DRIVE_CDROM)
		{
			diskname[i * 2] = *dir;
			diskname[i * 2 + 1] = '\0';
		}
	}
	free(buf);
}

//核心算法函数，用于遍历文件
void Scaner::myfindfile(const char* path)
{
	char* currentpath = (char*)malloc(3000);
	char* subPath = (char*)malloc(3000);

	WIN32_FIND_DATA findFileData = *(WIN32_FIND_DATA*)malloc(sizeof(WIN32_FIND_DATA));
	HANDLE hFind = *(HANDLE*)malloc(sizeof(HANDLE));

	sprintf(currentpath, "%s\\%s", path, "*.*");

	hFind = FindFirstFile(currentpath, &findFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		//printf("INVALID HANDLE\n");
		return;
	}
	else
	{
		while (1)
		{
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				//find a directory
				if (findFileData.cFileName[0] != '.')
				{
					//不是当前目录，也不是上一级目录，就对该目录进行再次扫描
					sprintf(subPath, "%s\\%s", path, findFileData.cFileName);
					myfindfile(subPath);
				}
			}
			else
			{
				//判断文件类型	目前扫描7种文件类型（doc，docx，xls，xlsx，pdf，wps，ppt，pptx）
				int ret = Scaner::getfiletype(findFileData.cFileName);
				if (ret == 1 || ret == 2 || ret == 3 || ret == 4 || ret == 5 || ret == 6 || ret == 7 || ret == 8)
				{
					MyFile* file = (MyFile*)malloc(sizeof(MyFile));
					memset(file, 0, sizeof(MyFile));
					sprintf(file->path, "%s\\%s", path, findFileData.cFileName);
					file->type = ret;
					findstr(file);
					free(file);
				}
			}
			if (FindNextFile(hFind, &findFileData) == FALSE)
			{
				//找不到下一个文件，跳出循环
				break;
			}
		}
		FindClose(hFind);
		free(currentpath);
		free(subPath);
	}
}

//文件转换函数，将特定类型文件转为TXT
void Scaner::all2txt(MyFile* file)
{
	char destpath[3000] = { 0 };
	sprintf(destpath, "%s.txt", file->path);

	char* command = (char*)malloc(6000);
	if (file->type == 6)
		sprintf(command, ".\\all2txt\\a2tcmd.exe -tdoc \"%s\" \"%s\"\r\n", file->path, destpath);
	else sprintf(command, ".\\all2txt\\a2tcmd.exe \"%s\" \"%s\"\r\n", file->path, destpath);

	
	while (TRUE)
	{
		if (CheckProcess() <= OneAll2txt)
		{
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			ZeroMemory(&si, sizeof(si));
			ZeroMemory(&pi, sizeof(pi));
			CreateProcess(".\\\\all2txt\\a2tcmd.exe", command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
			WaitForSingleObject(pi.hProcess, INFINITE);
			//通过WaitForSingleObject达到阻塞的目的
			CloseHandle(pi.hProcess);
			break;
		}
		else Sleep(1);
	}
	
	

	/*
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	CreateProcess(".\\\\all2txt\\a2tcmd.exe", command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	WaitForSingleObject(pi.hProcess, 5000);
	//通过WaitForSingleObject达到阻塞的目的
	CloseHandle(pi.hProcess);
	*/

	free(command);
}

//在TXT文件中查找关键字
void Scaner::findstr(MyFile* file)
{
	all2txt(file);
	char destpath[3000] = { 0 };
	sprintf(destpath, "%s.txt", file->path);

	FILE* fp = fopen(destpath, "r");
	if (fp != NULL)
	{
		while (!feof(fp))
		{
			char* buf = (char*)malloc(1024);
			fgets(buf, 1024, fp);
			if (strstr(buf, NAME1) != NULL || strstr(buf, NAME2) != NULL || strstr(buf, NAME3) != NULL)
			{
				char findpath[MAX_PATH] = { 0 };
				time_t timep;
				struct tm *p;
				time(&timep);
				p = gmtime(&timep);
				sprintf(findpath, ".\\find.log");

				FILE* fp1 = fopen(findpath, "a+");
				if (fp1 != NULL)
				{
					fprintf(fp1, "%s----------%s", file->path, buf);
					printf("---------------------------------------------------------------------------------------------------------------------------------找到啦！\n");
					fclose(fp1);
				}
				free(buf);
				break;
			}
			free(buf);
		}
		fclose(fp);
	}
	remove(destpath);
}

//获取文件后缀类型
int Scaner::getfiletype(char filename[MAX_PATH])
{
	//返回值DOC=1、DOCX=2、XLS=3、XLSX=4、PDF=5、WPS=6、PPT=7、PPTX=8
	for (int i = strlen(filename) - 1;i >= 0; i--)
	{
		if (*(filename + i) == '.')
		{
			if (strcmp(filename + i + 1, "doc") == 0)
				return 1;
			if (strcmp(filename + i + 1, "docx") == 0)
				return 2;
			if (strcmp(filename + i + 1, "xls") == 0)
				return 3;
			if (strcmp(filename + i + 1, "xlsx") == 0)
				return 4;
			if (strcmp(filename + i + 1, "pdf") == 0)
				return 5;
			if (strcmp(filename + i + 1, "wps") == 0)
				return 6;
			if (strcmp(filename + i + 1, "ppt") == 0)
				return 7;
			if (strcmp(filename + i + 1, "pptx") == 0)
				return 8;
			return 0;
		}
	}
}

//判断all2txt进程是否存在
int Scaner::CheckProcess()
{
	HANDLE procSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (procSnap == INVALID_HANDLE_VALUE)
		return 0;

	PROCESSENTRY32 procEntry = { 0 };
	procEntry.dwSize = sizeof(PROCESSENTRY32);
	BOOL bRet = Process32First(procSnap, &procEntry);
	int count = 0;
	while (bRet)
	{
		if (strcmp("a2tcmd.exe", procEntry.szExeFile) == 0)
			count++;
		bRet = Process32Next(procSnap, &procEntry);
	}
	CloseHandle(procSnap);
	return count;
}