#include "scan.h"

#define TwoAll2txt 1
#define OneAll2txt 0

//ȫ�ֹؼ���
MyKey mykey;

//�������������ڴ洢�洢�ҵ����ļ���Ϣ
vector<MyFile> v;

//ֻ��һ��ɨ���߳�
DWORD _stdcall ThreadScan(LPVOID path)
{
	Scaner::myfindfile((char*)path);
	return 0;
}

//ȫ��ɨ����ú���
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

	printf("******************************ȫ��ɨ�����******************************\n");
	printf("ȫ��ɨ������ʱ�䣺%f\n", (double)(end - start) / CLK_TCK);
	for (int i = 0; i < getdiskcount(); i++)
		int ret = CloseHandle(hThread[i]);
	free(hThread);

	Scaner::CreateLog(1);
}

//����ɨ����ú���
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

	printf("******************************����ɨ�����******************************\n");
	printf("����ɨ������ʱ�䣺%f\n", (double)(end - start) / CLK_TCK);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	free(path1);
	free(path2);

	Scaner::CreateLog(0);
}

//��ȡ�û�������ĵ�·��
void Scaner::getpath(char* path1, char* path2)
{
	//path1Ϊ�ĵ�·����path2Ϊ����·��
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

//��ȡ��������
int Scaner::getdiskcount()
{
	int diskcount = 0;
	DWORD diskinfo = GetLogicalDrives();
	if (diskinfo != 0)
	{
		while (diskinfo)
		{
			//ͨ���ж�ÿһλ�Ƿ�Ϊ1��ȷ���Ƿ���ڸô���
			if (diskinfo & 1)
				++diskcount;
			diskinfo = diskinfo >> 1;
		}
	}
	return diskcount;
}

//��ȡ��������
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
		//DRIVE_FIXED���ع̶����� DRIVE_REMOVABLE���ƶ����̣�U�̣��ƶ�Ӳ�̣� DRIVE_CDROM ����
		if (dtype == DRIVE_FIXED || dtype == DRIVE_REMOVABLE || dtype == DRIVE_CDROM)
		{
			diskname[i * 2] = *dir;
			diskname[i * 2 + 1] = '\0';
		}
	}
	free(buf);
}

//�����ļ��������ҵ��ض������ļ��������޹��ļ���Ŀ¼
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
					//���ǵ�ǰĿ¼��Ҳ������һ��Ŀ¼���ͶԸ�Ŀ¼�����ٴ�ɨ��
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
				//����ϵͳ�ļ�Ҳ������ʱ�ļ�
				//�ж��ļ�����	Ŀǰɨ��8���ļ����ͣ�doc��docx��xls��xlsx��pdf��wps��ppt��pptx��
				int ret = Scaner::getfiletype(findFileData.cFileName);
				if (ret == 1 || ret == 2 || ret == 3 || ret == 4 || ret == 5 || ret == 6 || ret == 7 || ret == 8)
				{
					MyFile* file = (MyFile*)malloc(sizeof(MyFile));
					memset(file, 0, sizeof(MyFile));

					//�ļ����͸�ֵ
					file->type = ret;
					//�ļ�����ֵ
					strcpy(file->name, findFileData.cFileName);
					//�ļ�·����ֵ
					sprintf(file->path, "%s\\%s", path, findFileData.cFileName);

					if (findstr(file) == TRUE)
						v.push_back(*file);
					free(file);
				}
			}
			if (FindNextFile(hFind, &findFileData) == FALSE)
			{
				//�Ҳ�����һ���ļ�������ѭ��
				break;
			}
		}
		FindClose(hFind);
		free(currentpath);
		free(subPath);
	}
}

//�ļ�ת�����������ض������ļ�תΪTXT
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
	CreateProcess(".\\\\all2txt\\a2tcmd.exe", command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (WaitForSingleObject(pi.hProcess, 3000) == WAIT_TIMEOUT)
	{
		TerminateProcess(pi.hProcess, 0);
		Sleep(1);
	}
	CloseHandle(pi.hProcess);
	free(command);
}

//��TXT�ļ��в��ҹؼ���
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
	while (!feof(fp))
	{
		char* buf = (char*)malloc(4096);
		memset(buf, 0, 4096);
		fgets(buf, 4096, fp);
		for (int i = 0; i < mykey.count; i++)
		{
			if (strstr(buf, mykey.Key[i]) != NULL)
			{
				strcpy(file->key, mykey.Key[i]);
				strcpy(file->comment, buf);
				if (file->comment[4095] == '\0')
					file->comment[4094] = '\n';

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

//��ȡ�ļ���׺����
int Scaner::getfiletype(char filename[MAX_PATH])
{
	//����ֵDOC=1��DOCX=2��XLS=3��XLSX=4��PDF=5��WPS=6��PPT=7��PPTX=8
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

//�ж�all2txt�����Ƿ����
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

//����ɨ��ʱ�����־�ļ�
void Scaner::CreateLog(int type)
{
	//type=1Ϊȫ�̡�type=0Ϊ����
	time_t timep;
	struct tm *p;
	time(&timep);
	p = gmtime(&timep);
	char time[1024] = { 0 };
	if (type == 1)
		sprintf(time, "All%d��%d��%d��%dʱ%d��%d��.log", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, 8 + p->tm_hour, p->tm_min, p->tm_sec);
	else
		sprintf(time, "Fast%d��%d��%d��%dʱ%d��%d��.log", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, 8 + p->tm_hour, p->tm_min, p->tm_sec);
	
	//������־�ļ�
	FILE* fp;
	fp = fopen(time, "a+");
	if (fp != NULL)
		for (int i = 0; i < v.size(); i++)
			fprintf(fp, "%s------%s------%s------%s", v[i].name, v[i].path, v[i].key, v[i].comment);
	fclose(fp);
	
	//���Ԫ�ز������ڴ�
	//vector<MyFile>().swap(v);
	v.clear();
}

//���¹ؼ���
void Scaner::GetKeyConfig()
{
	for (int i = 0; i < 100; i++)
		mykey.Key[i] = (char*)malloc(20);
	FILE* fp;
	fp = fopen("key.ini", "r");
	if (fp == NULL)
	{
		mykey.count = 3;
		mykey.Key[0] = "����";
		mykey.Key[1] = "����";
		mykey.Key[2] = "����";

		//�������ļ���Ĭ������һ���ؼ����ļ�
		FILE* fp2 = fopen("key.ini", "w");
		if (fp2 != NULL)
		{
			for (int i = 0; i < mykey.count; i++)
				fprintf(fp2, "%s\n", mykey.Key[i]);
			fclose(fp2);
		}
		return;
	}
	int count = 0;
	while (!feof(fp))
		fscanf(fp, "%s", mykey.Key[count++]);
	mykey.count = count;
	fclose(fp);
}


