#pragma once
#include <stdio.h>
#include <Windows.h>

class Monitor
{
	SYSTEMTIME st;

	char* getpath();

	int getfiletype(char filename[MAX_PATH]);

	void myfindfile(const char* path);

	int compare(SYSTEMTIME st1);

	void updatetime()
	{
		GetLocalTime(&st);
	}
public:
	Monitor()
	{
		updatetime();
	}
	void start();
};
