#include <stdio.h>
#include <Windows.h>
int main()
{
	/*
	FILE* fp;
	fp = fopen("C:\\Users\\Mrtan\\Desktop\\SafeCheck\\SafeCheck\\all2txt\\B.txt","a+");
	if (fp != NULL)
	{
		fprintf(fp, "hello,wolrd!\n");
	}
	*/
	HANDLE hFile = CreateFileA("first.cpp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		printf("创建文件失败！\n");



	return 0;
}