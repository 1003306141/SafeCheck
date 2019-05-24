#include "stdafx.h"
#include "kmp_main.h"

int Kmp_MainStrstr(KeywordsInfo * information,const char *filecontent)
{
	if (information == NULL || information->need_match_count <= 0 
		|| information->need_match_keyword == NULL || 
		information->need_match_keyword_rank == NULL)
	{
		return -1;
	}
	memset(information->match_keyword_repeat_time, 0, 100 * 40);
	information->match_count = 0;
	int j = 0;//可能有没有匹配到的关键字，故结果数据的下标和i小标可能不一致
	if (information->match_mode == OnceMatch)
	{
		for (int i = 0; i < information->need_match_count; i++)
		{
			int res = Kmp_OnceStrStr(i, information, filecontent);
			if (res == -1)
				continue;
			
			information->match_count++;
			information->match_keyword[j] =
				(char *)malloc(sizeof(char)*strlen(information->need_match_keyword[i]) + 1);
			strcpy(information->match_keyword[j], information->need_match_keyword[i]);
			information->match_keyword_rank[j] = information->need_match_keyword_rank[i];
			information->match_keyword_position[0][j] = res;
			Kmp_getSummary_One(information, res, filecontent, j);
			j++;
		}
	}
	else if (information->match_mode == AllMatch)
	{
		for (int i = 0; i < information->need_match_count; i++)
		{
			int res = Kmp_AllStrStr(information, filecontent,i,j);
			if(res == -1)
				continue;
			information->match_keyword[j] =
				(char *)malloc(sizeof(char)*strlen(information->need_match_keyword[i]) + 1);
			strcpy(information->match_keyword[j], information->need_match_keyword[i]);
			information->match_keyword_rank[j] = information->need_match_keyword_rank[i];
			information->match_count++;
			Kmp_getSummart_All(information, filecontent, j);
			j++;
		}
	}
	return 0;
	
}

int Kmp_getSummary_One(KeywordsInfo * information, int res , const char *filecontent , int j)//j是代表第k行
{
	char tmpstr[1024] = { 0 };
	int headposition = res - 25;
	if (headposition < 0)
		headposition = 0;
	int tailposition = res + 25;
	if (tailposition > strlen(filecontent))
		tailposition = strlen(filecontent);
	int i = 0;
	while (headposition != tailposition)
	{
		tmpstr[i++] = filecontent[headposition++];
	}
	tmpstr[i] = '\0';
	information->match_keywords_summary[0][j] = (char *)malloc(i + 1);
	strcpy(information->match_keywords_summary[0][j], tmpstr);

	return 0;
}

int Kmp_getSummart_All(KeywordsInfo * information, const char *filecontent, int j)
{
	//当前关键字重复的次数
	int all_repeat_count = information->match_keyword_repeat_time[j];
	for (int i = 0; i < all_repeat_count; i++)
	{
		int current_position = information->match_keyword_position[j][i];//第i次出现的位置大小。

		char tmpstr[1024] = { 0 };
		int headposition = current_position - 25;
		if (headposition < 0)
			headposition = 0;
		int tailposition = current_position + 25;
		if (tailposition > strlen(filecontent))
			tailposition = strlen(filecontent);
		int num = 0;
		while (headposition != tailposition)
		{
			tmpstr[num++] = filecontent[headposition++];
		}
		tmpstr[num] = '\0';
		information->match_keywords_summary[j][i] = (char *)malloc(num + 1);
		strcpy(information->match_keywords_summary[j][i], tmpstr);
	}
	return 0;
}

int Kmp_OnceStrStr(int i, KeywordsInfo * information, const char *filecontent)
{
	int sp = 0;
	int mp = 0;
	int *next = (int *)malloc(sizeof(int) * strlen(information->need_match_keyword[i]));
	if (next == nullptr)
	{
		printf("malloc error\n");
		return -1;
	}

	Kmp_getNextArray(information->need_match_keyword[i], next);
	while ((sp < (int)strlen(filecontent)) && (mp < (int)strlen(information->need_match_keyword[i])))
	{
		if (filecontent[sp] == information->need_match_keyword[i][mp])
		{
			sp++;
			mp++;
		}
		else if (next[mp] == -1)
		{
			sp++;
		}
		else
		{
			mp = next[mp];
		}
	}
	free(next);
	if (next != nullptr)
	{
		next = nullptr;
	}
	if (mp == strlen(information->need_match_keyword[i]))
	{
		return sp - mp;//返回这次找到的位置
	}
	else
	{
		//Information->KeyAppearCount = -1;
		return -1;
	}
}

int Kmp_AllStrStr(KeywordsInfo * information,const char *filecontent,int i,int j)
{
	int SaveKeyAppearCount = 0;
	int OffsetNumber = 0;
	int offsetAddress = 0;
	const char *SaveStrContent = filecontent;

	int lastlastNumber = 1;

	int last_find_position = 0;
	//memset(Information->Position, 0, ALLSTRMAXFOUNDCOUNT * sizeof(int));
	while ((last_find_position = Kmp_OnceStrStr(i,information,filecontent)) != -1)
	{
		//给当前本次查询的Position赋值为上次的offsetNumber + 本次Infomation->positions
		information->match_keyword_position[j][SaveKeyAppearCount] = OffsetNumber + last_find_position;

		//下次偏移的地址数 等于本次找到的位置加上关键字的长度
		offsetAddress = strlen(information->need_match_keyword[i]) + last_find_position;

		//下次偏移的位置必须的加上上次的offsetNumber
		OffsetNumber = information->match_keyword_position[j][lastlastNumber++ - 1]
			+ strlen(information->need_match_keyword[i]);

		filecontent += offsetAddress;
		if (information->match_keyword_repeat_time[j] > 100)break;//超过100个关键字个数不做处理

		//出现的次数加1
		SaveKeyAppearCount++;
		information->match_keyword_repeat_time[j]++;
	}
	if (SaveKeyAppearCount != 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

void Kmp_getNextArray(const char * match, int *next)
{
	if (match == nullptr || next == nullptr)
		return;
	int length = strlen(match);
	if (length == 1)
	{
		next[0] = -1;
		return;
	}

	next[0] = -1;
	next[1] = 0;
	int pos = 2;//从下标2开始
	int cn = 0;//
	while (pos < length)
	{
		if (match[pos - 1] == match[cn])
		{
			next[pos++] = ++cn;
		}
		else if (cn > 0)
		{
			cn = next[cn];
		}
		else
		{
			next[pos++] = 0;
		}
	}
}

void Kmp_DeleteAllMem(KeywordsInfo * information)
{
	if (information->match_mode == OnceMatch)
	{
		for (int i = 0; i < information->match_count; i++)
		{
			free(information->match_keyword[i]);
			information->match_keyword[i] = NULL;

			free(information->match_keywords_summary[0][i]);
			information->match_keywords_summary[0][i] = NULL;
		}
	}
	else if (information->match_mode == AllMatch)
	{
		for (int i = 0; i < information->match_count; i++)
		{
			free(information->match_keyword[i]);
			information->match_keyword[i] = NULL;

			int all_repeat_count = information->match_keyword_repeat_time[i];
			for (int j = 0; j < all_repeat_count; j++)
			{
				free(information->match_keywords_summary[i][j]);
				information->match_keywords_summary[i][j] = NULL;
			}
		}
	}

}
