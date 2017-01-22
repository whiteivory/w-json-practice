#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
/*
由于不是c++,很多地方为了区分namespace要加上头缀
*/

typedef enum {
	LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT
} lept_type;
typedef struct {
	lept_type type;
}lept_value;
/*enum就是int*/
enum
{
	LEPT_PARSE_OK = 0,
	LEPT_PARSE_EXCEPT_VALUE, //当只有空格的时候
	LEPT_PARSE_INVALID_VALUE,//不是所给类型中的一种
	LEPT_PARSE_NOTSIGULAR_VALUE, //最后的空格后面还有值的时候
};

/*返回错误或者成功码*/
int lept_parse(lept_value*,const char*);
lept_type lept_get_type(const lept_value*);//mark:notice const