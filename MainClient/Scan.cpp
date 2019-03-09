#include "stdafx.h"
#include "scan.h"

string GBKToUTF8(const char* strGBK)
{
	int len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, strGBK, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	string strTemp = str;
	if (wstr) delete[] wstr;
	if (str) delete[] str;
	return strTemp;
}

void UTF8ToGBK(char* strUTF8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char* szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	strcpy(strUTF8, szGBK);
	if (wszGBK) delete[] wszGBK;
	if (szGBK) delete[] szGBK;
}

void FileGBKToUTF8(const char* filename)
{
	FILE* fp = fopen(filename, "rb");
	if (fp == NULL)
		return;
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buf = (char*)malloc(size);
	fread(buf, size, 1, fp);
	string str = GBKToUTF8(buf);
	fclose(fp);
	remove(filename);
	FILE* fp2 = fopen(filename, "wb");
	if (fp2 == NULL)
		return;
	fwrite(str.c_str(), str.size(), 1, fp2);
	fclose(fp2);
}

//全局关键字
MyKey mykey;

//定义容器，用于存储存储找到的文件信息
vector<MyFile> v;

//扫描线程函数
DWORD _stdcall ThreadScan(LPVOID path)
{
	Scaner::myfindfile((char*)path);
	return 0;
}

//全盘扫描调用函数
void Scaner::alldiskscan()
{
	Scaner::GetKeyConfig();

	clock_t start, end;
	char* diskname = (char*)malloc(2 * 32);
	memset(diskname, 0, 32 * 2);
	getdiskname(diskname);

	start = clock();
	HANDLE* hThread = (HANDLE*)malloc(sizeof(HANDLE)*getdiskcount());
	for (int i = 0; i < getdiskcount(); i++)
	{
		char* path = (char*)malloc(3);
		memset(path, 0, 3);
		*(path + 0) = diskname[i * 2];
		*(path + 1) = ':';
		*(path + 2) = '\0';
		hThread[i] = CreateThread(NULL, 0, ThreadScan, (LPVOID)path, 0, NULL);
	}
	WaitForMultipleObjects(getdiskcount(), hThread, TRUE, INFINITE);
	end = clock();

	printf("******************************全盘扫描完成******************************\n");
	printf("全盘扫描所用时间：%f\n", (double)(end - start) / CLK_TCK);
	for (int i = 0; i < getdiskcount(); i++)
		int ret = CloseHandle(hThread[i]);
	free(hThread);

	Scaner::CreateLog(1);
}

//快速扫描调用函数
void Scaner::fastscan()
{
	Scaner::GetKeyConfig();

	clock_t start, end;
	HANDLE hThread[2];
	char* path1 = (char*)malloc(MAX_PATH);
	char* path2 = (char*)malloc(MAX_PATH);
	Scaner::getpath(path1, path2);

	start = clock();
	hThread[0] = CreateThread(NULL, 0, ThreadScan, (LPVOID)path1, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, ThreadScan, (LPVOID)path2, 0, NULL);
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
	end = clock();

	printf("******************************快速扫描完成******************************\n");
	printf("快速扫描所用时间：%f\n", (double)(end - start) / CLK_TCK);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	free(path1);
	free(path2);

	Scaner::CreateLog(0);
}

//全盘普通扫描调用函数
void Scaner::alldiskscannormal()
{
	Scaner::GetKeyConfig();

	clock_t start, end;
	char* diskname = (char*)malloc(2 * 32);
	memset(diskname, 0, 32 * 2);
	getdiskname(diskname);

	start = clock();
	char* path = (char*)malloc(3);
	for (int i = 0; i < getdiskcount(); i++)
	{
		memset(path, 0, 3);
		*(path + 0) = diskname[i * 2];
		*(path + 1) = ':';
		*(path + 2) = '\0';
		Scaner::myfindfile(path);
	}
	free(path);
	end = clock();

	printf("******************************全盘扫描完成******************************\n");
	printf("全盘扫描所用时间：%f\n", (double)(end - start) / CLK_TCK);

	Scaner::CreateLog(1);
}

//获取用户桌面和文档路径
void Scaner::getpath(char* path1, char* path2)
{
	//path1为文档路径，path2为桌面路径
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

//遍历文件函数，找到特定类型文件并屏蔽无关文件和目录
void Scaner::myfindfile(const char* path)
{
	char* currentpath = (char*)malloc(6000);
	char* subPath = (char*)malloc(6000);

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
					if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || (findFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) || (findFileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
					{
						//do nothing
					}
					else
					{
						sprintf(subPath, "%s\\%s", path, findFileData.cFileName);
						myfindfile(subPath);
					}
				}
			}	
			else if( !(findFileData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) && !(findFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))
			{
				//不是系统文件也不是临时文件
				//判断文件类型	目前扫描8种文件类型（doc，docx，xls，xlsx，pdf，wps，ppt，pptx）
				int ret = Scaner::getfiletype(findFileData.cFileName);
				if (ret == 1 || ret == 2 || ret == 3 || ret == 4 || ret == 5 || ret == 6 || ret == 7 || ret == 8)
				{
					MyFile* file = (MyFile*)malloc(sizeof(MyFile));
					memset(file, 0, sizeof(MyFile));

					//文件类型赋值
					file->type = ret;
					//文件名赋值
					strcpy(file->name, findFileData.cFileName);
					//文件路径赋值
					sprintf(file->path, "%s\\%s", path, findFileData.cFileName);

					if (findstr(file) == TRUE)
						v.push_back(*file);
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
		sprintf(command, ".\\all2txt\\a2tcmd.exe -tdoc \"%s\" \"%s\"\n", file->path, destpath);
	else sprintf(command, ".\\all2txt\\a2tcmd.exe \"%s\" \"%s\"\n", file->path, destpath);

	/*
	while (TRUE)
	{
		if (CheckProcess() <= INFINITE)
		{
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			ZeroMemory(&si, sizeof(si));
			ZeroMemory(&pi, sizeof(pi));
			CreateProcess(".\\\\all2txt\\a2tcmd.exe", command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
			WaitForSingleObject(pi.hProcess, 3000);
			TerminateProcess(pi.hProcess,0);
			CloseHandle(pi.hProcess);
			break;
		}
		else Sleep(1);
	}
	*/
	
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(STARTUPINFO);
	si.lpReserved = NULL;
	si.lpDesktop = NULL;
	si.lpTitle = NULL;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.cbReserved2 = NULL;
	si.lpReserved2 = NULL;
	CreateProcess(".\\\\all2txt\\a2tcmd.exe", command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (WaitForSingleObject(pi.hProcess, 3000) == WAIT_TIMEOUT)
	{
		TerminateProcess(pi.hProcess, 0);
		Sleep(1);
	}
	CloseHandle(pi.hProcess);
	free(command);
}

//在TXT文件中查找关键字
int Scaner::findstr(MyFile* file)
{
	all2txt(file);
	char destpath[3000] = { 0 };
	sprintf(destpath, "%s.txt", file->path);

	FILE* fp = fopen(destpath, "r");
	if (fp == NULL)
	{
		remove(destpath);
		return FALSE;
	}
	//获取文件字节数
	fseek(fp, 0, SEEK_END);
	file->size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	while (!feof(fp))
	{
		char* buf = (char*)malloc(1024);
		memset(buf, 0, 1024);
		fscanf_s(fp, "%s", buf, 1024);//为了防止一个文件中的一行超过1024个字节，最后释放的时候导致HEAP CORRUPTION DETECTED错误
		for (int i = 0; i < mykey.count; i++)
		{
			if (strstr(buf, mykey.Key[i]) != NULL)
			{
				//关键字级别
				file->rank = mykey.rank[i];
				//关键字
				strcpy(file->key, mykey.Key[i]);
				//关键字前后文
				strcpy(file->comment, buf);
				if (file->comment[1023] != '\0')
					file->comment[1023] = '\0';

				//查找关键字位置
				fseek(fp, 0, SEEK_SET);
				char* buf2 = (char*)malloc(file->size);
				fread(buf2, 1, file->size, fp);//读取进来后将\r\n变成\n
				char* find = strstr(buf2, mykey.Key[i]);
				file->position = find - buf2;

				free(buf2);
				free(buf);
				fclose(fp);
				remove(destpath);
				return TRUE;
			}
		}
		free(buf);
	}

	fclose(fp);
	remove(destpath);
	return FALSE;
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

//生成扫描时间的日志文件
void Scaner::CreateLog(int type)
{
	//type=1为全盘、type=0为快速
	char filename[30];
	if (type == 1)
		sprintf(filename, "first.rlog");
	else
		sprintf(filename, "second.rlog");

	//生成日志文件
	FILE* fp;
	fp = fopen(filename, "ab+");
	if (fp != NULL)
		for (int i = 0; i < v.size(); i++)
			fprintf(fp, "%s|%d-%s-1-%d:%d|%s\n", v[i].path, v[i].rank, v[i].key, v[i].position, v[i].size, v[i].comment);
	fclose(fp);

	FileGBKToUTF8(filename);

	//清除元素并回收内存
	//vector<MyFile>().swap(v);
	v.clear();
}

//更新关键字
void Scaner::GetKeyConfig()
{
	for (int i = 0; i < 100; i++)
	{
		mykey.Key[i] = (char*)malloc(60);
		memset(mykey.Key[i], 0, 60);
	}
	FILE* fp;
	fp = fopen("Fullkeywords.txt", "r");
	if (fp == NULL)
		return;

	int count = 0;
	while (!feof(fp))
	{
		fscanf(fp, "%d-%s", &mykey.rank[count], mykey.Key[count]);
		count++;
	}
	//多执行一次，这里减一
	count--;
	for (int i = 0; i < count; i++)
		UTF8ToGBK(mykey.Key[i]);
	mykey.count = count;
	fclose(fp);
}

//数据库操作

/*
void Scaner::query_sql(char* sql)
{
	MYSQL* con;
	MYSQL_RES* res_ptr;
	MYSQL_ROW result_row;
	con = mysql_init(NULL);
	con = mysql_real_connect(con, HOST, USERNAME, PASSWORD, DATABASE, 0, 0, CLIENT_FOUND_ROWS);
	if (!con)
	{
		printf("mysql连接失败\n");
		return;
	}
	mysql_query(con, "set names \'GBK\'");
	int res = mysql_query(con, sql);//返回NULL代表成功
	if (res)
	{
		printf("sql语句执行失败%d\n",res);
		mysql_close(con);
		return;
	}
	
	遍历整个表
	res_ptr = mysql_store_result(con);
	my_ulonglong row = mysql_num_rows(res_ptr);
	my_ulonglong col = mysql_num_fields(res_ptr);
	for (int i = 0; i < row; i++)
	{
		result_row = mysql_fetch_row(res_ptr);
		for (int j = 0; j < col; j++)
			printf("%10s ", result_row[j]);
		printf("\n");
	}
	
	mysql_close(con);
	return;
}
*/