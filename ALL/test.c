/*
	tdd ��������������
	��д���ԣ�Ȼ�����У��µĲ��Ի�ʧ�ܣ�Ȼ���д����ʵ�֣�����ʧ�����±�д�޸Ĵ���
	�����ĺô��ǲ���д����Ĵ��룬�ұ�֤�˲��Եĸ����ʣ������������˲��ԵĹ�������������
*/

/*
�����
notice ��Ҫע��ĵط�
mistake ������ʱ��ͱ�׼��һ���ĵط�
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

/*���� do while�﷨�ֺܺõý���˱�������������⣬��ֹ�ظ�����v*/
/*���parseʧ�ܵĴ������������lept_value type��Ա�����ó�LEPT_NULL*/
#define TEST_ERROR(expect_ret,test_value) \
	do{ \
	lept_value v;\
	v.type = LEPT_FALSE;\
	EXPECT_EQ_INT(expect_ret, lept_parse(&v, test_value));\
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));\
	}while(0)

//mistake ������Ҫ�������������ܴﵽĿ��
#define TEST_NUMBER(expect,test_value) \
	do{ \
		lept_value v; \
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, test_value));\
		EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));\
		EXPECT_EQ_DOUBLE(expect,lept_get_number(&v));\
	}while(0)



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
}
static void test_parse_number_too_big() {
#if 0
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

void test_parse() {
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_invalid_value();
	test_except_value();
	test_root_not_single();
	test_parse_number();
}

int main() {
	test_parse();
	return main_ret;
}