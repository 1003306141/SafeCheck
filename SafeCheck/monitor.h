#pragma once
#include <stdio.h>
#include <Windows.h>

char* getsyspath();

void start();

int getfiletype(char filename[MAX_PATH]);

void myfindfile(const char* path);