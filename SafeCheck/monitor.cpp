#include "monitor.h"


char* Monitor::getpath()
{
	char buffer[1024];
	memset(buffer, 0, 1024);
	GetEnvironmentVariable("USERPROFILE", buffer, MAX_PATH);
	sprintf(buffer, "%s\\AppData\\Roaming\\Microsoft\\Windows\\Recent",buffer);
	return buffer;
}

void Monitor::start()
{
	char buffer[1024];
	memset(buffer, 0, 1024);
	strcpy(buffer, getpath());
	while (1)
	{
		myfindfile(buffer);
		Sleep(1000);
	}
}

void Monitor::myfindfile(const char* path)
{
	char* currentpath = (char*)malloc(3000);

	WIN32_FIND_DATA findFileData = { 0 };
	HANDLE hFind = { 0 };

	sprintf(currentpath, "%s\\%s", path, "*.*");

	hFind = FindFirstFile(currentpath, &findFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		free(currentpath);
		return;
	}
	
	while (1)
	{
		if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			//�ж��ļ�����	Ŀǰɨ��8���ļ����ͣ�doc��docx��xls��xlsx��pdf��wps��ppt��pptx��
			int ret = getfiletype(findFileData.cFileName);
			if (ret == 1 || ret == 2 || ret == 3 || ret == 4 || ret == 5 || ret == 6 || ret == 7 || ret == 8)
			{
				FILETIME ft = { 0 };
				SYSTEMTIME st1 = { 0 };
				FileTimeToLocalFileTime(&findFileData.ftLastWriteTime, &ft);
				FileTimeToSystemTime(&ft, &st1);

				if (compare(st1) == 0)
				{
					printf("%s---------%4d-%02d-%02d %02d:%02d:%02d\n", findFileData.cFileName, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
					updatetime();
				}
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
}

int Monitor::compare(SYSTEMTIME st1)
{
	///��ֵԽ�����ʱ��Խ��

	//�Ƚ���
	if (st1.wYear < st.wYear)
		return -1;
	if (st1.wYear > st.wYear)
		return 0;

	//�Ƚ���
	if (st1.wMonth < st.wMonth)
		return -1;
	if (st1.wMonth > st.wMonth)
		return 0;

	//�Ƚ���
	if (st1.wDay < st.wDay)
		return -1;
	if (st1.wDay > st.wDay)
		return 0;

	//�Ƚ�ʱ
	if (st1.wHour < st.wHour)
		return -1;
	if (st1.wHour > st.wHour)
		return 0;

	//�ȽϷ�
	if (st1.wMinute < st.wMinute)
		return -1;
	if (st1.wMinute > st.wMinute)
		return 0;

	//�Ƚ���
	if (st1.wSecond < st.wSecond)
		return -1;
	if (st1.wSecond > st.wSecond)
		return 0;
	
	//ʱ����ͬ
	return 0;
}

int Monitor::getfiletype(char filename[MAX_PATH])
{
	//����ֵDOC=1��DOCX=2��XLS=3��XLSX=4��PDF=5��WPS=6��PPT=7��PPTX=8
	//��ݷ�ʽ��׺��Ϊ.lnk������������ٶ��4
	for (int i = strlen(filename) - 1 - 4; i >= 0; i--)
	{
		if (*(filename + i) == '.')
		{
			if (strcmp(filename + i + 1, "doc.lnk") == 0)
				return 1;
			if (strcmp(filename + i + 1, "docx.lnk") == 0)
				return 2;
			if (strcmp(filename + i + 1, "xls.lnk") == 0)
				return 3;
			if (strcmp(filename + i + 1, "xlsx.lnk") == 0)
				return 4;
			if (strcmp(filename + i + 1, "pdf.lnk") == 0)
				return 5;
			if (strcmp(filename + i + 1, "wps.lnk") == 0)
				return 6;
			if (strcmp(filename + i + 1, "ppt.lnk") == 0)
				return 7;
			if (strcmp(filename + i + 1, "pptx.lnk") == 0)
				return 8;
			return 0;
		}
	}
}