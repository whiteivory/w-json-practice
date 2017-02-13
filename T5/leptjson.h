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
typedef struct lept_value lept_value;
struct lept_value{
	union {
		struct { lept_value* e; size_t len; }a;
		struct { char* s; size_t len; }s;
		double n;
	}u;
	lept_type type;
};
/*enum����int*/
enum
{
	LEPT_PARSE_OK = 0,
	LEPT_PARSE_EXPECT_VALUE, //��ֻ�пո��ʱ��
	LEPT_PARSE_INVALID_VALUE,//�������������е�һ��
	LEPT_PARSE_ROOT_NOT_SINGULAR, //���Ŀո���滹��ֵ��ʱ��
	LEPT_PARSE_NUMBER_TOO_BIG, //number too big
	LEPT_PARSE_MISS_QUOTATION_MARK,
	LEPT_PARSE_INVALID_STRING_ESCAPE, //
	LEPT_PARSE_INVALID_STRING_CHAR,
	LEPT_PARSE_INVALID_UNICODE_HEX,
	LEPT_PARSE_INVALID_UNICODE_SURROGATE,
	LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET
};

/*���ش�����߳ɹ���*/
int lept_parse(lept_value*,const char*);

//init
//mistake: vû�м�����
#define lept_init(v) do{(v)->type = LEPT_NULL;}while(0);

//get set method
int lept_get_boolean(const lept_value* v);
void lept_set_boolean(lept_value* v, int b);

double lept_get_number(const lept_value* v);
void lept_set_number(lept_value* v, double n);

const char* lept_get_string(const lept_value* v);
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len);
void lept_free(lept_value* v);

size_t lept_get_array_size(const lept_value* v);
lept_value* lept_get_array_element(const lept_value* v, size_t index);



#define lept_set_null(v) lept_free(v)