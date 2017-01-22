/*
	tdd 测试驱动开发。
	先写测试，然后运行，新的测试会失败，然后编写代码实现，若再失败重新编写修改代码
	这样的好处是不会写多余的代码，且保证了测试的覆盖率，但会由于做了测试的工作，拖慢进程
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"leptjson.h"

static main_ret = 0;
static test_count = 0;
static test_pass = 0;

/*base和int之所以要分开是因为base试用于任何情况，而int只适用于判断int
相等的情况*/
#define EXPECT_EQ_BASE(equality,except,actual,format) \
	do{ \
		if(equality == 1)\
			test_pass ++;\
		else{ \
			fprintf(stderr,"%s:%d except value: "format"actual value: "format"\n",__FILE__,__LINE__,except,actual);\
			main_ret = 1; \
			}\
	}while(0) \
/*对于int变量,这里指lept_type*/
#define EXPECT_EQ_INT(except,actual) \
	EXPECT_EQ_BASE((except == actual),except,actual,"%d");

/*测试lept_value是否能正常解析null变量，有两个值需要测试，一是返回的状态码
而是变量的lept_type是否正确*/
void test_parse_null() {
	lept_value v;
	v.type = LEPT_FALSE;
	int status_code = lept_parse(&v, " null ");
	EXPECT_EQ_INT(LEPT_PARSE_OK, status_code);
	EXPECT_EQ_INT(LEPT_NULL, v.type);//第一个测试结束

	v.type = LEPT_FALSE;
	status_code = lept_parse(&v, "null");
	EXPECT_EQ_INT(LEPT_PARSE_OK, status_code);
	EXPECT_EQ_INT(LEPT_NULL, v.type);//第二个测试结束

}

//todo
//test
void test_parse_true() {
	lept_value v;
	v.type = LEPT_FALSE;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "true"));
	EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));
}
//todo
//test
void test_parse_false() {
	lept_value v;
	v.type = LEPT_TRUE;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "false"));
	EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
}

void test_except_value() {
	//这种清空v的类型没有特殊的，而是用NULL代替
	lept_value v;

	v.type = LEPT_FALSE;
	EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, ""));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));

	v.type = LEPT_FALSE;
	EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, " "));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

void test_invalid_value() {
	lept_value v;
	v.type = LEPT_FALSE;
	EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "nul"));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));

	v.type = LEPT_FALSE;
	EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "?"));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

void test_root_not_single() {
	lept_value v;
	v.type = LEPT_FALSE;
	EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "null x"));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

void test_parse() {
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_invalid_value();
	test_except_value();
	test_root_not_single();
}

int main() {
	test_parse();
	return main_ret;
}