// TestReverse.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <assert.h>
#include <string.h>

char* HH_strcpy(char *dst, const char* src)
{
	assert(dst);
	assert(src);
	char* cp = dst;// cp是临时的指针变量，随着循环的进行，地址一直在变，直到循环结束。cp的地址从0x006c6338变化到0x006c633c。
	while (*cp++ = *src++)
		;
	return dst; // 必须传回首地址才行



	//Error: 错误方式的拷贝
	//assert(dst);
	//assert(src);
	//while (*dst++ = *src++)
		;
	//return dst; 随着循环的进行，dst地址一直在发生变化，return dst就不是返回首地址了。
}
int _tmain(int argc, _TCHAR* argv[])
{
	char data[5];
	memset(data, 0, 5);
	char* sz =  HH_strcpy(data, "333");

	return 0;
}