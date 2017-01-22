/*
	tdd ��������������
	��д���ԣ�Ȼ�����У��µĲ��Ի�ʧ�ܣ�Ȼ���д����ʵ�֣�����ʧ�����±�д�޸Ĵ���
	�����ĺô��ǲ���д����Ĵ��룬�ұ�֤�˲��Եĸ����ʣ������������˲��ԵĹ�������������
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
#define EXPECT_EQ_BASE(equality,except,actual,format) \
	do{ \
		if(equality == 1)\
			test_pass ++;\
		else{ \
			fprintf(stderr,"%s:%d except value: "format"actual value: "format"\n",__FILE__,__LINE__,except,actual);\
			main_ret = 1; \
			}\
	}while(0) \
/*����int����,����ָlept_type*/
#define EXPECT_EQ_INT(except,actual) \
	EXPECT_EQ_BASE((except == actual),except,actual,"%d");

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
	//�������v������û������ģ�������NULL����
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