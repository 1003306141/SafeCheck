#pragma once
#include <stdio.h>
#include <malloc.h>
#include <string.h>
enum MatchMode
{
	OnceMatch,
	AllMatch
};

struct KeywordsInfo
{
	MatchMode match_mode;

	//input
	int need_match_count;
	char* need_match_keyword[100];
	int need_match_keyword_rank[100];

	//output
	int match_count;
	char* match_keyword[100];
	int match_keyword_rank[100];

	int match_keyword_repeat_time[100];

	int match_keyword_position[100][100];

	char *match_keywords_summary[100][100];//每个关键字最多有100个位置 最多100个关键字 存放摘要字符串
};




int Kmp_MainStrstr(KeywordsInfo * information, const char *filecontent);
int Kmp_OnceStrStr(int i, KeywordsInfo * information, const char *filecontent);
int Kmp_AllStrStr(KeywordsInfo * information, const char *filecontent,int i,int j);
void Kmp_getNextArray(const char * match, int *next); 
void Kmp_DeleteAllMem(KeywordsInfo * Information);

int Kmp_getSummary_One(KeywordsInfo * information, int res, const char *filecontent, int j);

int Kmp_getSummart_All(KeywordsInfo * information, const char *filecontent, int j);