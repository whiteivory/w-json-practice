#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
/*
	tdd 测试驱动开发。
	先写测试，然后运行，新的测试会失败，然后编写代码实现，若再失败重新编写修改代码
	这样的好处是不会写多余的代码，且保证了测试的覆盖率，但会由于做了测试的工作，拖慢进程
*/

/*
标记有
notice 需要注意的地方
mistake 我做的时候和标准不一样的地方
trick 技巧
todo 待做或者待完善的地方
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
#define EXPECT_EQ_BASE(equality,expect,actual,format) \
	do{ \
		if(equality == 1)\
			test_pass ++;\
		else{ \
			fprintf(stderr,"%s:%d expect value: "format"actual value: "format"\n",__FILE__,__LINE__,expect,actual);\
			main_ret = 1; \
			}\
	}while(0) \
/*对于int变量,这里指lept_type*/
#define EXPECT_EQ_INT(expect,actual) \
	EXPECT_EQ_BASE((expect == actual),expect,actual,"%d");
#define EXPECT_EQ_DOUBLE(expect,actual) \
	EXPECT_EQ_BASE((expect == actual),expect,actual,"%l");
/*notice: 判断字符串相等的方法*/
#define EXPECT_EQ_STRING(expect,actual,len) \
	EXPECT_EQ_BASE(sizeof(expect) - 1 == len && memcmp(expect, actual, len) == 0, expect, actual, "%s")
#define EXPECT_FALSE(actual) \
	EXPECT_EQ_BASE((actual == 0),"false","true","%s")
#define EXPECT_TRUE(actual) \
	EXPECT_EQ_BASE((actual == 1),"true","false","%s")

/*这里 do while语法又很好得解决了变量作用域的问题，防止重复定义v*/
/*检测parse失败的错误，这种情况下lept_value type成员会设置成LEPT_NULL*/
/*todo: 这里v.type=LEPT_FALSE 不太好，应该放在构造函数里初始化*/
#define TEST_ERROR(expect_ret,json) \
	do{ \
	lept_value v;\
	lept_init(&v); \
	v.type = LEPT_FALSE;\
	EXPECT_EQ_INT(expect_ret, lept_parse(&v, json));\
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));\
	lept_free(&v);\
	}while(0)

//mistake 这里需要传两个参数才能达到目的
#define TEST_NUMBER(expect,test_value) \
	do{ \
		lept_value v; \
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, test_value));\
		EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));\
		EXPECT_EQ_DOUBLE(expect,lept_get_number(&v));\
	}while(0)

//mistake: 加入字符串动态分配后，必须记得释放
#define TEST_STRING(expect,json) \
	do{ \
		lept_value v; \
		lept_init(&v); \
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, test_value));\
		EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&v));\
		EXPECT_EQ_STRING(expect,lept_get_string(&v),lept_get_string_length(&v));\
		lept_free(&v);\
	} while (0);

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
void test_parse_true() {
	lept_value v;
	v.type = LEPT_FALSE;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "true"));
	EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));
}
void test_parse_false() {
	lept_value v;
	v.type = LEPT_TRUE;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "false"));
	EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
}


static void test_parse_number() {
	TEST_NUMBER(0.0, "0");
	TEST_NUMBER(0.0, "-0");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(1.0, "1");
	TEST_NUMBER(-1.0, "-1");
	TEST_NUMBER(1.5, "1.5");
	TEST_NUMBER(-1.5, "-1.5");
	TEST_NUMBER(3.1416, "3.1416");
	TEST_NUMBER(1E10, "1E10");
	TEST_NUMBER(1e10, "1e10");
	TEST_NUMBER(1E+10, "1E+10");
	TEST_NUMBER(1E-10, "1E-10");
	TEST_NUMBER(-1E10, "-1E10");
	TEST_NUMBER(-1e10, "-1e10");
	TEST_NUMBER(-1E+10, "-1E+10");
	TEST_NUMBER(-1E-10, "-1E-10");
	TEST_NUMBER(1.234E+10, "1.234E+10");
	TEST_NUMBER(1.234E-10, "1.234E-10");
	TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

	//notice: denormal和subnormal 参考https://en.wikipedia.org/wiki/Denormal_number
	TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
	TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
	TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
	TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
	TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
	TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}
static void test_parse_number_too_big() {
#if 1
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e309");
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e309");
#endif
}


void test_except_value() {

	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, " ");
}

void test_invalid_value() {
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nul");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "?");

	/*notice:注意这种注释的方式，一般的注释不能有嵌套，而这种方式可以嵌套
	而且想要取消注释只需要改成if 1*/
#if 1
	/* invalid number */
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+0");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+1");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "INF");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "inf");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "NAN");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nan");
#endif
}

void test_root_not_single() {
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null x");
}
static void test_parse_missing_quotation_mark() {
	TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"");
	TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
#if 1
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
#endif
}

static void test_parse_invalid_string_char() {
#if 1
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
#endif
}

/*notice: 所有函数都要经过测试*/
static void test_access_null() {
	lept_value v;
	lept_init(&v);
	lept_set_string(&v, "a", 1);
	lept_set_null(&v);
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
	lept_free(&v);
}

//主要测试用lept_set的时候有没有进行lept_free
static void test_access_boolean() {
	lept_value v;
	lept_init(&v);
	lept_set_string(&v, "a", 1);
	lept_set_boolean(&v, 1);
	EXPECT_TRUE(lept_get_boolean(&v));
	lept_set_boolean(&v, 0);
	EXPECT_FALSE(lept_get_boolean(&v));
	lept_free(&v);
}

static void test_access_number() {
	lept_value v;
	lept_init(&v);
	lept_set_string(&v, "a", 1);
	lept_set_number(&v, 1234.5);
	EXPECT_EQ_DOUBLE(1234.5, lept_get_number(&v));
	lept_free(&v);
}

static void test_access_string() {
	lept_value v;
	lept_init(&v);
	lept_set_string(&v, "", 0);
	EXPECT_EQ_STRING("", lept_get_string(&v), lept_get_string_length(&v));
	lept_set_string(&v, "Hello", 5);
	EXPECT_EQ_STRING("Hello", lept_get_string(&v), lept_get_string_length(&v));
	lept_free(&v);
}
void test_parse() {
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_invalid_value();
	test_except_value();
	test_root_not_single();
	test_parse_number();
	test_parse_number_too_big();

	test_parse_missing_quotation_mark();
	test_parse_invalid_string_escape();
	test_parse_invalid_string_char();

	test_access_null();
	test_access_boolean();
	test_access_number();
	test_access_string();
}

int main() {
	//question: 不知道为什么按照说的没有检测数内存泄漏
#ifdef _WINDOWS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	test_parse();
	return main_ret;
}