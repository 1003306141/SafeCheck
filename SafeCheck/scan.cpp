#include "scan.h"

#define NAME1 "����"
#define NAME2 "����"
#define NAME3 "����"

//����ɨ���߳�
DWORD _stdcall ThreadFastScan(LPVOID path)
{
	char path1[MAX_PATH] = { 0 };
	char path2[MAX_PATH] = { 0 };
	Scaner::getpath(path1, path2);
	Scaner::myfindfile(path1);
	Scaner::myfindfile(path2);
	MessageBox(NULL, "����·��ɨ��ɨ��ɹ�!", "�ɹ�", NULL);
	return 0;
}

//ȫ��ɨ���߳�
DWORD _stdcall ThreadAllScan(LPVOID path)
{
	Scaner::myfindfile((char*)path);

	//���ɨ��ɹ���ʾ
	char* buf = (char*)malloc(20);
	*((char*)path + 1) = '\0';
	sprintf(buf, "%sɨ��ɹ�", path);
	MessageBox(NULL, buf, "�ɹ�", NULL);
	free(buf);
	return 0;
}

//ȫ��ɨ����ú���
void Scaner::alldiskscan()
{
	char* diskname = (char*)malloc(2 * 32);
	memset(diskname, 0, 32 * 2);
	getdiskname(diskname);
	for (int i = 0; i < getdiskcount(); i++)
	{
		char* path = (char*)malloc(3);
		memset(path, 0, 3);
		*(path + 0) = diskname[i * 2];
		*(path + 1) = ':';
		*(path + 2) = '\0';
		CreateThread(NULL, 0, ThreadAllScan, (LPVOID)path, 0, NULL);
	}
}

//����ɨ����ú���
void Scaner::fastscan()
{
	CreateThread(NULL, 0, ThreadFastScan, NULL, 0, NULL);
}

//��ȡ�û�������ĵ�·��
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

//�����㷨���������ڱ����ļ�
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
					sprintf(subPath, "%s\\%s", path, findFileData.cFileName);
					myfindfile(subPath);
				}
			}
			else
			{
				//�ж��ļ�����	Ŀǰɨ��6���ļ����ͣ�doc��docx��xls��xlsx��pdf��wps��
				int ret = Scaner::getfiletype(findFileData.cFileName);
				if (ret == 1 || ret == 2 || ret == 3 || ret == 4 || ret == 5 || ret == 6)
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
		sprintf(command, ".\\all2txt\\a2tcmd.exe -tdoc \"%s\" \"%s\"\r\n", file->path, destpath);
	else sprintf(command, ".\\all2txt\\a2tcmd.exe \"%s\" \"%s\"\r\n", file->path, destpath);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	CreateProcess(".\\\\all2txt\\a2tcmd.exe", command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	WaitForSingleObject(pi.hProcess, 5000);
	//ͨ��WaitForSingleObject�ﵽ������Ŀ��

	free(command);
}

//��TXT�ļ��в��ҹؼ���
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
					printf("---------------------------------------------------------------------------------------------------------------------------------�ҵ�����\n");
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

//��ȡ�ļ���׺����
int Scaner::getfiletype(char filename[MAX_PATH])
{
	//����ֵDOC=1��DOCX=2��XLS=3��XLSX=4��PDF=5��WPS=6
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
			return 0;
		}
	}
}

