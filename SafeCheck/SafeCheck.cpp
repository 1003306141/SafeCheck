#include "scan.h"
#include "monitor.h"

//禁止多开
int MoreOpen();
//交互菜单页面
void mainface();

int main()
{
	if (MoreOpen() == TRUE)
		return 0;
	while (1)
	{
		char code;
		mainface();
		scanf("\n%c", &code);
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
			case '3':
			{	
				printf("\n现在开始全盘普通扫描......\n");
				Scaner::alldiskscannormal();
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
	}
	return 0;
}

void mainface()
{
	printf("--------------------------------\n");
	printf("           1.快速扫描           \n");
	printf("           2.全盘快速           \n");
	printf("           3.全盘普通           \n");
	printf("           4.测试功能           \n");
	printf("           0.退出程序           \n");
	printf("--------------------------------\n");
	printf("请输入您的操作: ");
}

int MoreOpen()
{
	HANDLE g_hMutex = CreateMutexA(NULL, TRUE, "防止多开");
	DWORD dwRet = GetLastError();
	if (g_hMutex == NULL)
		return TRUE;
	else if (dwRet == ERROR_ALREADY_EXISTS)
		return TRUE;
	return FALSE;
}