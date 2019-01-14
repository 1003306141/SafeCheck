﻿#include "scan.h"

void mainface();

int main()
{
	while (1)
	{
		char code;
		mainface();
		scanf(" %c", &code);
		if (code >= '0' && code <= '9')
		{
			switch (code)
			{
			case '1':
			{
				printf("\n现在开始快速扫描......\n");
				Scaner::fastscan();
			}; break;
			case '2':
			{
				printf("\n现在开始全盘扫描......\n");
				Scaner::alldiskscan();
			}; break;
			case '0':
			{
				return 0;
			}; break;
			default:
			{
				printf("\n输入错误，重新输入\n");
			}; break;
			}
		}
		else printf("\n输入错误，重新输入\n");
	}
	return 0;
}