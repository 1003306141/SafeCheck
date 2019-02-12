#include "monitor.h"


char* getsyspath()
{
	char buffer[1024];
	memset(buffer, 0, 1024);
	GetEnvironmentVariable("USERPROFILE", buffer, MAX_PATH);
	sprintf(buffer, "%s\\AppData\\Roaming\\Microsoft\\Windows\\Recent",buffer);
	return buffer;
}

void start()
{
	char buffer[1024];
	memset(buffer, 0, 1024);
	strcpy(buffer, getsyspath());
	myfindfile(buffer);
}

void myfindfile(const char* path)
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
			//判断文件类型	目前扫描8种文件类型（doc，docx，xls，xlsx，pdf，wps，ppt，pptx）
			int ret = getfiletype(findFileData.cFileName);
			if (ret == 1 || ret == 2 || ret == 3 || ret == 4 || ret == 5 || ret == 6 || ret == 7 || ret == 8)
			{
				printf("%s\n", findFileData.cFileName);
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
}

int getfiletype(char filename[MAX_PATH])
{
	//返回值DOC=1、DOCX=2、XLS=3、XLSX=4、PDF=5、WPS=6、PPT=7、PPTX=8
	//快捷方式后缀都为.lnk，所以这里得再多减4
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