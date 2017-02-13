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
typedef struct lept_value lept_value;
struct lept_value{
	union {
		struct { lept_value* e; size_t len; }a;
		struct { char* s; size_t len; }s;
		double n;
	}u;
	lept_type type;
};
/*enum就是int*/
enum
{
	LEPT_PARSE_OK = 0,
	LEPT_PARSE_EXPECT_VALUE, //当只有空格的时候
	LEPT_PARSE_INVALID_VALUE,//不是所给类型中的一种
	LEPT_PARSE_ROOT_NOT_SINGULAR, //最后的空格后面还有值的时候
	LEPT_PARSE_NUMBER_TOO_BIG, //number too big
	LEPT_PARSE_MISS_QUOTATION_MARK,
	LEPT_PARSE_INVALID_STRING_ESCAPE, //
	LEPT_PARSE_INVALID_STRING_CHAR,
	LEPT_PARSE_INVALID_UNICODE_HEX,
	LEPT_PARSE_INVALID_UNICODE_SURROGATE,
	LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET
};

/*返回错误或者成功码*/
int lept_parse(lept_value*,const char*);

//init
//mistake: v没有加括号
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