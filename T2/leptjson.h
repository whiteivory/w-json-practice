#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
/*
���ڲ���c++,�ܶ�ط�Ϊ������namespaceҪ����ͷ׺
*/

typedef enum {
	LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT
} lept_type;
typedef struct {
	double n;
	lept_type type;
}lept_value;
/*enum����int*/
enum
{
	LEPT_PARSE_OK = 0,
	LEPT_PARSE_EXPECT_VALUE, //��ֻ�пո��ʱ��
	LEPT_PARSE_INVALID_VALUE,//�������������е�һ��
	LEPT_PARSE_ROOT_NOT_SINGULAR, //���Ŀո���滹��ֵ��ʱ��
	LEPT_PARSE_NUMBER_TOO_BIG //number too big
};

/*���ش�����߳ɹ���*/
int lept_parse(lept_value*,const char*);
lept_type lept_get_type(const lept_value*);//mark:notice const
double lept_get_number(const lept_value*);