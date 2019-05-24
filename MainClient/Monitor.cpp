#pragma once
#include "stdafx.h"
#include "Monitor.h"
#include "Network.h"

SYSTEMTIME g_localtime = { 0 };
extern SSL_Handler hdl;

DWORD _stdcall MonitorStart(LPVOID)
{
	while (TRUE)
	{
		GetLocalTime(&g_localtime);
		Sleep(MonitorTime*60000);
		StartScanAllDisk();
	}
	return 0;
}

void StartScanAllDisk()
{
	char* alldisk_name = (char*)malloc(3 * 32);
	memset(alldisk_name, 0, 3 * 32);
	GetAllDiskName(alldisk_name);

	for (int i = 0; i < GetDiskCount(); i++)
		FindNewFile(&alldisk_name[i * 3]);

	free(alldisk_name);
}

bool GetLocalKeyword(KeyWord* keyword)
{
	for (int i = 0; i < 100; i++)
	{
		keyword->key[i] = (char*)malloc(100);
		memset(keyword->key[i], 0, 100);
	}
	
	FILE* fp;
	fp = fopen("Fullkeywords.txt", "r");
	if (fp == NULL)
		return FALSE;

	int count = 0;
	while (!feof(fp))
	{
		fscanf(fp, "%d-%s", &keyword->rank[count], keyword->key[count]);
		count++;
	}
	//多执行一次，这里减一
	count--;

	for (int i = 0; i < count; i++)
		UTF8ToGBK(keyword->key[i]);
	keyword->count = count;
	fclose(fp);

	return TRUE;
}

void GetAllDiskName(char* alldisk_name)
{
	UINT dtype;
	char* buf = (char*)malloc(32 * 4);
	memset(buf, 0, 32 * 4);
	GetLogicalDriveStrings(32 * 2, (LPSTR)buf);
	for (int i = 0; i < GetDiskCount(); i++)
	{
		char dir[3] = { buf[i * 4],':','\\' };
		dtype = GetDriveType(buf + i * 4);
		if (dtype == DRIVE_FIXED || dtype == DRIVE_REMOVABLE || dtype == DRIVE_CDROM)
		{
			alldisk_name[i * 3] = dir[0];
			alldisk_name[i * 3 + 1] = dir[1];
			alldisk_name[i * 3 + 2] = '\0';
		}
	}
	free(buf);
}

int GetDiskCount()
{
	int disk_count = 0;
	DWORD disk_info = GetLogicalDrives();
	if (disk_info != 0)
	{
		while (disk_info)
		{
			if (disk_info & 1)
				++disk_count;
			disk_info = disk_info >> 1;
		}
	}
	return disk_count;
}

bool IsTheFile(WIN32_FIND_DATA file_data)
{
	if (!IsNewFile(file_data.ftLastAccessTime))
		return FALSE;
	if (!IsTypeFile(file_data.cFileName))
		return FALSE;
	return TRUE;
}

bool IsNewFile(FILETIME findfile_time)
{
	FILETIME findfile_localtime;
	FileTimeToLocalFileTime(&findfile_time, &findfile_localtime);

	FILETIME g_filetime;
	SystemTimeToFileTime(&g_localtime, &g_filetime);

	if (CompareFileTime(&findfile_localtime, &g_filetime) >= 0)
	{
		return TRUE;
	}

	return FALSE;
}

bool IsTypeFile(const char file_name[MAX_PATH])
{
	for (int i = strlen(file_name) - 1; i >= 0; i--)
	{
		if (*(file_name + i) == '.')
		{
			if (strcmp(file_name + i + 1, "doc") == 0)
				return TRUE;
			if (strcmp(file_name + i + 1, "docx") == 0)
				return TRUE;
			if (strcmp(file_name + i + 1, "xls") == 0)
				return TRUE;
			if (strcmp(file_name + i + 1, "xlsx") == 0)
				return TRUE;
			if (strcmp(file_name + i + 1, "pdf") == 0)
				return TRUE;
			if (strcmp(file_name + i + 1, "wps") == 0)
				return TRUE;
			if (strcmp(file_name + i + 1, "ppt") == 0)
				return TRUE;
			if (strcmp(file_name + i + 1, "pptx") == 0)
				return TRUE;
			if (strcmp(file_name + i + 1, "rar") == 0)
				return TRUE;
			if (strcmp(file_name + i + 1, "zip") == 0)
				return TRUE;
			if (strcmp(file_name + i + 1, "7z") == 0)
				return TRUE;
			return FALSE;
		}
	}
	return FALSE;
}

enum FileType GetFileType(const char file_name[MAX_PATH])
{
	for (int i = strlen(file_name) - 1; i >= 0; i--)
	{
		if (*(file_name + i) == '.')
		{
			if (strcmp(file_name + i + 1, "doc") == 0)
				return DOC;
			if (strcmp(file_name + i + 1, "docx") == 0)
				return DOCX;
			if (strcmp(file_name + i + 1, "xls") == 0)
				return XLS;
			if (strcmp(file_name + i + 1, "xlsx") == 0)
				return XLSX;
			if (strcmp(file_name + i + 1, "pdf") == 0)
				return PDF;
			if (strcmp(file_name + i + 1, "wps") == 0)
				return WPS;
			if (strcmp(file_name + i + 1, "ppt") == 0)
				return PPT;
			if (strcmp(file_name + i + 1, "pptx") == 0)
				return PPTX;
			if (strcmp(file_name + i + 1, "rar") == 0)
				return RAR;
			if (strcmp(file_name + i + 1, "zip") == 0)
				return ZIP;
			if (strcmp(file_name + i + 1, "7z") == 0)
				return Z7;
		}
	}
}

bool FindKeyword(FileInfo* file)
{
	KeyWord g_keyword;
	if (!GetLocalKeyword(&g_keyword))
		return FALSE;

	Translate2Txt(file);

	char txt_file[1024];
	sprintf(txt_file, "%s.txt", file->file_path);

	FILE* fp = fopen(txt_file, "r");
	if (fp == NULL)
		return FALSE;

	int txt_file_size = 0;
	fseek(fp, 0, SEEK_END);
	txt_file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	file->file_content = (char*)malloc(txt_file_size);
	fread(file->file_content, txt_file_size, 1, fp);

	file->keywords_info.need_match_count = g_keyword.count;
	for (int i = 0; i < g_keyword.count; i++)
	{
		strcpy(file->keywords_info.need_match_keyword[i], g_keyword.key[i]);
		file->keywords_info.need_match_keyword_rank[i] = g_keyword.rank[i];
	}
	file->keywords_info.match_mode = AllMatch;

	Kmp_MainStrstr(&file->keywords_info, file->file_content);
	
	fclose(fp);
	remove(txt_file);

	if (file->keywords_info.match_count == 0)
		return FALSE;
	return TRUE;
}

void ExtractFile(FileInfo* file)
{
	char command[2000] = { 0 };
	sprintf(command, ".\\7z\\7z.exe x -o%sx %s", file->file_path, file->file_path);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (WaitForSingleObject(pi.hProcess, 3000) == WAIT_TIMEOUT)
	{
		TerminateProcess(pi.hProcess, 0);
		Sleep(1);
	}
	CloseHandle(pi.hProcess);

	char path[1024] = { 0 };
	sprintf(path, "%sx", file->file_path);
	FindNewFile(path);
	
	char remove_command[1024] = { 0 };
	sprintf(remove_command, "rmdir /s /q %s", path);
	system(remove_command);
}

void Translate2Txt(FileInfo* file)
{
	char dest_path[3000] = { 0 };
	sprintf(dest_path, "%s.txt", file->file_path);

	char command[2000] = {0};
	switch (file->file_type)
	{
	case ZIP:
	{
		ExtractFile(file);
		return;
	}break;
	case RAR:
	{
		ExtractFile(file);
		return;
	}break;
	case Z7:
	{
		ExtractFile(file);
		return;
	}break;
	case WPS:
	{
		sprintf(command, ".\\all2txt\\a2tcmd.exe -tdoc \"%s\" \"%s\"\n", file->file_path, dest_path);
	}break;
	default:
	{
		sprintf(command, ".\\all2txt\\a2tcmd.exe \"%s\" \"%s\"\n", file->file_path, dest_path);
	}break;
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (WaitForSingleObject(pi.hProcess, 3000) == WAIT_TIMEOUT)
	{
		TerminateProcess(pi.hProcess, 0);
		Sleep(1);
	}
	CloseHandle(pi.hProcess);
}

void InitFile(FileInfo* file,WIN32_FIND_DATA file_data,const char* path)
{
	//文件类型赋值
	file->file_type = GetFileType(file_data.cFileName);
	//文件名赋值
	strcpy(file->file_name, file_data.cFileName);
	//文件路径赋值
	sprintf(file->file_path, "%s\\%s", path, file_data.cFileName);
	//文件大小赋值
	file->file_size = file_data.nFileSizeLow;
	//文件访问时间赋值
	FILETIME local_filetime;
	FileTimeToLocalFileTime(&file_data.ftLastAccessTime, &local_filetime);
	SYSTEMTIME local_system_time;
	FileTimeToSystemTime(&local_filetime, &local_system_time);
	file->file_access_time = local_system_time;
	//初始化空间
	for (int i = 0; i < 100; i++)
		file->keywords_info.need_match_keyword[i] = (char*)malloc(50);
}

void FindNewFile(const char* path)
{
	char* current_path = (char*)malloc(6000);
	char* sub_path = (char*)malloc(6000);

	HANDLE h_find = 0;
	WIN32_FIND_DATA findfile_data = { 0 };

	sprintf(current_path, "%s\\%s", path, "*.*");

	h_find = FindFirstFile(current_path, &findfile_data);

	if (h_find == INVALID_HANDLE_VALUE)
		return;

	while (TRUE)
	{
		//判断是否是目录文件
		if (findfile_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			//判断是否为当前目录或上一级目录
			if (findfile_data.cFileName[0] != '.')
			{
				//判断文件属性是否为一下几类
				if (!((findfile_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || (findfile_data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) || (findfile_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)))
				{
					sprintf(sub_path, "%s\\%s", path, findfile_data.cFileName);
					FindNewFile(sub_path);
				}
			}
		}
		else
		{
			//判断文件属性是否为一下几类
			if (!(findfile_data.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) && !(findfile_data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) && !(findfile_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
			{
				if (IsTheFile(findfile_data))
				{
					//初始化
					FileInfo* file = (FileInfo*)malloc(sizeof(FileInfo));
					memset(file, 0, sizeof(FileInfo));
					InitFile(file, findfile_data, path);
					//查找关键字
					if (FindKeyword(file))
					{
						//u.push_back(*file);
						char time[100] = { 0 };
						sprintf(time, "%d-%02d-%02d %02d:%02d",
							file->file_access_time.wYear,
							file->file_access_time.wMonth,
							file->file_access_time.wDay,
							file->file_access_time.wHour,
							file->file_access_time.wMinute);

						char file_hash[] = "11111111111111111111111111111111";

						char key_info[200] = { 0 };
						sprintf(key_info, "%d-%s-%d-%d",
							file->keywords_info.match_keyword_rank[0],
							file->keywords_info.match_keyword[0],
							file->keywords_info.match_keyword_repeat_time[0],
							file->keywords_info.match_keyword_position[0][0]);

						char content[200] = { 0 };
						sprintf(content, "0 %s", "机密_秘密_绝密");

						char file_path[200] = { 0 };
						sprintf(file_path, "%s", file->file_path);

						char buffer[2048] = { 0 };
						memset(buffer, 0, 1024);
						sprintf(buffer, "%s\n%s\n%s\n%s\n%s", time, file_hash, key_info, content, file_path);
						string str = GBKToUTF8(buffer);

						SendInfo("LOG", str.c_str());

						char info[50];
						GetReplyInfo(info);
					}
				}
			}
		}
		if (FindNextFile(h_find, &findfile_data) == FALSE)
			break;
	}

	FindClose(h_find);
	free(current_path);
	free(sub_path);
}

/*
void CreateMonitorLog()
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	char logfile_name[1024];
	sprintf(logfile_name, "%d-%02d-%02d %02d-%02d-%02d.txt", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	FILE* fp = fopen(logfile_name, "w");
	if (fp == NULL)
		return;

	for (int i = 0; i < u.size(); i++)
	{
		fprintf(fp, "-------------------------------------------------------\n");

		fprintf(fp, "报警文件名：%s\n",u[i].file_name);
		fprintf(fp, "文件路径：%s\n",u[i].file_path);

		fprintf(fp, "关键字：\n");
		for (int j = 0; j < u[i].keywords_info.match_count; j++)
		{
			fprintf(fp, "级别：%d 关键字：%s 重复次数：%d 位置：",
				u[i].keywords_info.match_keyword_rank[j],
				u[i].keywords_info.match_keyword[j],
				u[i].keywords_info.match_keyword_repeat_time[j]);
			for (int k = 0; k < u[i].keywords_info.match_keyword_repeat_time[j]; k++)
				fprintf(fp, "%d ", u[i].keywords_info.match_keyword_position[j][k]);
			fprintf(fp, "\n");
		}

		fprintf(fp, "关键字前后文：%s\n", u[i].keywords_info.match_keywords_summary[0][0]);

		fprintf(fp, "报警时间：%d-%02d-%02d %02d:%02d:%02d\n",
			u[i].file_access_time.wYear,
			u[i].file_access_time.wMonth,
			u[i].file_access_time.wDay,
			u[i].file_access_time.wHour,
			u[i].file_access_time.wMinute,
			u[i].file_access_time.wSecond);

		fprintf(fp, "-------------------------------------------------------\n\n");
	}
	fclose(fp);

	//清空Vector所使用的空间
	u.clear();
}
*/