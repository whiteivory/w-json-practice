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
#define EXCEPT_EQ_BASE(equality,except,actual,format) \
	do{ \
		if(equality == 1)\
			test_pass ++;\
		else{ \
			fprintf(stderr,"%s:%d except value: "format"actual value: "format"\n",__FILE__,__LINE__,except,actual);\
			main_ret = 1; \
			}\
	}while(0) \
/*����int����,����ָlept_type*/
#define EXCEPT_EQ_INT(except,actual) \
	EXCEPT_EQ_BASE((except == actual),except,actual,"%d");

/*����lept_value�Ƿ�����������null������������ֵ��Ҫ���ԣ�һ�Ƿ��ص�״̬��
���Ǳ�����lept_type�Ƿ���ȷ*/
void test_parse_null() {
	lept_value v;
	v.type = LEPT_FALSE;
	int status_code = lept_parse(&v, " null ");
	EXCEPT_EQ_INT(LEPT_PARSE_OK, status_code);
	EXCEPT_EQ_INT(LEPT_NULL, v.type);//��һ�����Խ���

	v.type = LEPT_FALSE;
	status_code = lept_parse(&v, "null");
	EXCEPT_EQ_INT(LEPT_PARSE_OK, status_code);
	EXCEPT_EQ_INT(LEPT_NULL, v.type);//�ڶ������Խ���

}

//todo
//test
void test_parse_true() {


}
//todo
//test
void test_parse_false() {

}

void test_except_value() {

}

void test_invalid_value() {

}

void test_root_not_single() {

}

void test_parse() {
	test_parse_null();
	test_parse_true();
	test_parse_false();
}

int main() {
	test_parse();
	return main_ret;
}