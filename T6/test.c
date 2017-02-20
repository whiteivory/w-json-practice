#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
/*
	tdd ��������������
	��д���ԣ�Ȼ�����У��µĲ��Ի�ʧ�ܣ�Ȼ���д����ʵ�֣�����ʧ�����±�д�޸Ĵ���
	�����ĺô��ǲ���д����Ĵ��룬�ұ�֤�˲��Եĸ����ʣ������������˲��ԵĹ�������������
*/

/*
�����
notice ��Ҫע��ĵط�
mistake ������ʱ��ͱ�׼��һ���ĵط�
trick ����
todo �������ߴ����Ƶĵط�
*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"leptjson.h"

static main_ret = 0;
static test_count = 0;
static test_pass = 0;

/*base��int֮����Ҫ�ֿ�����Ϊbase�������κ��������intֻ�������ж�int
��ȵ����*/
#define EXPECT_EQ_BASE(equality,expect,actual,format) \
	do{ \
		if(equality == 1)\
			test_pass ++;\
		else{ \
			fprintf(stderr,"%s:%d expect value: "format"actual value: "format"\n",__FILE__,__LINE__,expect,actual);\
			main_ret = 1; \
			}\
	}while(0) \
/*����int����,����ָlept_type*/
#define EXPECT_EQ_INT(expect,actual) \
	EXPECT_EQ_BASE((expect == actual),expect,actual,"%d");
#define EXPECT_EQ_DOUBLE(expect,actual) \
	EXPECT_EQ_BASE((expect == actual),expect,actual,"%l");
/*notice: �ж��ַ�����ȵķ���*/
#define EXPECT_EQ_STRING(expect,actual,len) \
	EXPECT_EQ_BASE(sizeof(expect) - 1 == len && memcmp(expect, actual, len) == 0, expect, actual, "%s")
#define EXPECT_FALSE(actual) \
	EXPECT_EQ_BASE((actual == 0),"false","true","%s")
#define EXPECT_TRUE(actual) \
	EXPECT_EQ_BASE((actual == 1),"true","false","%s")
#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif

/*���� do while�﷨�ֺܺõý���˱�������������⣬��ֹ�ظ�����v*/
/*���parseʧ�ܵĴ������������lept_value type��Ա�����ó�LEPT_NULL*/
/*todo: ����v.type=LEPT_FALSE ��̫�ã�Ӧ�÷��ڹ��캯�����ʼ��*/
#define TEST_ERROR(expect_ret,json) \
	do{ \
	lept_value v;\
	lept_init(&v); \
	v.type = LEPT_FALSE;\
	EXPECT_EQ_INT(expect_ret, lept_parse(&v, json));\
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));\
	lept_free(&v);\
	}while(0)

//mistake ������Ҫ�������������ܴﵽĿ��
#define TEST_NUMBER(expect,test_value) \
	do{ \
		lept_value v; \
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, test_value));\
		EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));\
		EXPECT_EQ_DOUBLE(expect,lept_get_number(&v));\
	}while(0)

//mistake: �����ַ�����̬����󣬱���ǵ��ͷ�
#define TEST_STRING(expect,json) \
	do{ \
		lept_value v; \
		lept_init(&v); \
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, test_value));\
		EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&v));\
		EXPECT_EQ_STRING(expect,lept_get_string(&v),lept_get_string_length(&v));\
		lept_free(&v);\
	} while (0);

/*����lept_value�Ƿ�����������null������������ֵ��Ҫ���ԣ�һ�Ƿ��ص�״̬��
���Ǳ�����lept_type�Ƿ���ȷ*/
void test_parse_null() {
	lept_value v;
	v.type = LEPT_FALSE;
	int status_code = lept_parse(&v, " null ");
	EXPECT_EQ_INT(LEPT_PARSE_OK, status_code);
	EXPECT_EQ_INT(LEPT_NULL, v.type);//��һ�����Խ���

	v.type = LEPT_FALSE;
	status_code = lept_parse(&v, "null");
	EXPECT_EQ_INT(LEPT_PARSE_OK, status_code);
	EXPECT_EQ_INT(LEPT_NULL, v.type);//�ڶ������Խ���

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

	//notice: denormal��subnormal �ο�https://en.wikipedia.org/wiki/Denormal_number
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

static void test_parse_array() {
	size_t i, j;
	lept_value v;

	lept_init(&v);
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "[ ]"));
	EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
	EXPECT_EQ_SIZE_T(0, lept_get_array_size(&v));
	lept_free(&v);

	lept_init(&v);
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "[ null , false , true , 123 , \"abc\" ]"));
	EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
	EXPECT_EQ_SIZE_T(5, lept_get_array_size(&v));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(lept_get_array_element(&v, 0)));
	EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(lept_get_array_element(&v, 1)));
	EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(lept_get_array_element(&v, 2)));
	EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(lept_get_array_element(&v, 3)));
	EXPECT_EQ_INT(LEPT_STRING, lept_get_type(lept_get_array_element(&v, 4)));
	EXPECT_EQ_DOUBLE(123.0, lept_get_number(lept_get_array_element(&v, 3)));
	EXPECT_EQ_STRING("abc", lept_get_string(lept_get_array_element(&v, 4)), lept_get_string_length(lept_get_array_element(&v, 4)));
	lept_free(&v);

	lept_init(&v);
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
	EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
	EXPECT_EQ_SIZE_T(4, lept_get_array_size(&v));
	for (i = 0; i < 4; i++) {
		lept_value* a = lept_get_array_element(&v, i);
		EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(a));
		EXPECT_EQ_SIZE_T(i, lept_get_array_size(a));
		for (j = 0; j < i; j++) {
			lept_value* e = lept_get_array_element(a, j);
			EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(e));
			EXPECT_EQ_DOUBLE((double)j, lept_get_number(e));
		}
	}
	lept_free(&v);
}


static void test_parse_object() {
	lept_value v;
	size_t i;

	lept_init(&v);
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, " { } "));
	EXPECT_EQ_INT(LEPT_OBJECT, lept_get_type(&v));
	EXPECT_EQ_SIZE_T(0, lept_get_object_size(&v));
	lept_free(&v);

	lept_init(&v);
	
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v,
		" { "
		"\"n\" : null , "
		"\"f\" : false , "
		"\"t\" : true , "
		"\"i\" : 123 , "
		"\"s\" : \"abc\", "
		"\"a\" : [ 1, 2, 3 ],"
		"\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
		" } "
	));
	EXPECT_EQ_INT(LEPT_OBJECT, lept_get_type(&v));
	EXPECT_EQ_SIZE_T(7, lept_get_object_size(&v));
	EXPECT_EQ_STRING("n", lept_get_object_key(&v, 0), lept_get_object_key_length(&v, 0));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(lept_get_object_value(&v, 0)));
	EXPECT_EQ_STRING("f", lept_get_object_key(&v, 1), lept_get_object_key_length(&v, 1));
	EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(lept_get_object_value(&v, 1)));
	EXPECT_EQ_STRING("t", lept_get_object_key(&v, 2), lept_get_object_key_length(&v, 2));
	EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(lept_get_object_value(&v, 2)));
	EXPECT_EQ_STRING("i", lept_get_object_key(&v, 3), lept_get_object_key_length(&v, 3));
	EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(lept_get_object_value(&v, 3)));
	EXPECT_EQ_DOUBLE(123.0, lept_get_number(lept_get_object_value(&v, 3)));
	EXPECT_EQ_STRING("s", lept_get_object_key(&v, 4), lept_get_object_key_length(&v, 4));
	EXPECT_EQ_INT(LEPT_STRING, lept_get_type(lept_get_object_value(&v, 4)));
	EXPECT_EQ_STRING("abc", lept_get_string(lept_get_object_value(&v, 4)), lept_get_string_length(lept_get_object_value(&v, 4)));
	EXPECT_EQ_STRING("a", lept_get_object_key(&v, 5), lept_get_object_key_length(&v, 5));
	EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(lept_get_object_value(&v, 5)));
	EXPECT_EQ_SIZE_T(3, lept_get_array_size(lept_get_object_value(&v, 5)));
	for (i = 0; i < 3; i++) {
		lept_value* e = lept_get_array_element(lept_get_object_value(&v, 5), i);
		EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(e));
		EXPECT_EQ_DOUBLE(i + 1.0, lept_get_number(e));
	}
	EXPECT_EQ_STRING("o", lept_get_object_key(&v, 6), lept_get_object_key_length(&v, 6));
	{
		lept_value* o = lept_get_object_value(&v, 6);
		EXPECT_EQ_INT(LEPT_OBJECT, lept_get_type(o));
		for (i = 0; i < 3; i++) {
			lept_value* ov = lept_get_object_value(o, i);
			EXPECT_TRUE('1' + i == lept_get_object_key(o, i)[0]);
			EXPECT_EQ_SIZE_T(1, lept_get_object_key_length(o, i));
			EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(ov));
			EXPECT_EQ_DOUBLE(i + 1.0, lept_get_number(ov));
		}
	}
	lept_free(&v);
}

void test_except_value() {

	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, " ");
}

void test_invalid_value() {
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nul");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "?");

	/*notice:ע������ע�͵ķ�ʽ��һ���ע�Ͳ�����Ƕ�ף������ַ�ʽ����Ƕ��
	������Ҫȡ��ע��ֻ��Ҫ�ĳ�if 1*/
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

static void test_parse_miss_comma_or_square_bracket() {
#if 0
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
#endif
}

/*notice: ���к�����Ҫ��������*/
static void test_access_null() {
	lept_value v;
	lept_init(&v);
	lept_set_string(&v, "a", 1);
	lept_set_null(&v);
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
	lept_free(&v);
}

//��Ҫ������lept_set��ʱ����û�н���lept_free
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

//���ڽ���UTF-8�Ĵ���Ƚ��鷳������ֻ���Դ��������´��صĴ�����
static void test_parse_invalid_unicode_hex() {
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate() {
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
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

	test_parse_invalid_unicode_hex();
	test_parse_invalid_unicode_surrogate();
	//array
	test_parse_array();
	test_parse_miss_comma_or_square_bracket();
	//object
	test_parse_object();
}

static void mytest() {
}
int main() {
	//question: ��֪��Ϊʲô����˵��û�м�����ڴ�й©
#ifdef _WINDOWS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	mytest();
	test_parse();
	return main_ret;
}