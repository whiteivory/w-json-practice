#include"leptjson.h"
#include<assert.h>
/*��װ��һ���ṹ���ֹ����������*/
typedef struct {
	const char* json;
}lept_contest;

#define EXCEPT(c,ch) do{\
		assert(*(c->json)==ch);\
		c->json++;\
	}while(0)

/*�����ո�*/
static void lept_parse_whitespace(lept_contest* lc) {
	char *t = lc->json;
	while (*t == ' '||*t =='\t'||*t == '\n'||*t == '\r')
	{
		t++;
	}
	lc->json = t;
}

/*���ش������ȷ��*/
static int lept_parse_null(lept_contest* c, lept_value*v) {
	EXCEPT(c, 'n');//֮�������ﻹҪexcept����assertһ�£�����Ϊ��������Ҫ�в��Զ�����
	char* t = c->json;
	if (t[0] != 'u'|| t[1] != 'l'||t[2] != 'l') //EXCEPT�����Ѿ�����1��
		return LEPT_PARSE_INVALID_VALUE;
	c->json += 3;//EXCEPT�����Ѿ�����1��
	v->type = LEPT_NULL;
	return LEPT_PARSE_OK;
}
static int lept_parse_true(lept_contest* c, lept_value*v) {
	EXCEPT(c, 't');
	char*t = c->json;
	if (t[1] != 'r' || t[2] != 'u' || t[3] != 'e')
		return LEPT_PARSE_INVALID_VALUE;
	c->json += 3;
	v->type = LEPT_TRUE;
	return LEPT_PARSE_OK;
}
static int lept_parse_false(lept_contest* c, lept_value*v) {
	EXCEPT(c, 'f');
	char *t = c->json;
	if (t[1] != 'a' || t[2] != 'l' || t[3] != 's'||t[4]!='e')
		return LEPT_PARSE_INVALID_VALUE;
	c->json += 3;
	v->type = LEPT_FALSE;
	return LEPT_PARSE_OK;
}


/*����֮���Բ�д��lept_parse��Ҿ�����Ҫ���ǹ��ܵ�һ�����������ֻ��Ϊ��parse ws value ws�е�
value���������Ǳ��*/
/*������ȷ���ߴ�����*/
/*����д���ֳɺü���С������ֹ�������������������if�����߸��ӵ�case���*/
static int lept_parse_value(lept_contest* lc, lept_value*v) {
	switch (lc->json[0])
	{
	case 'n':return lept_parse_null(lc,v); break;
	case 't':return lept_parse_true(lc,v); break;
	case 'f':return lept_parse_false(lc,v); break;
	case '\0':return LEPT_PARSE_EXCEPT_VALUE; break;
	default: return LEPT_PARSE_INVALID_VALUE;
		break;
	}

}

/*�������ͨ���ƶ�jsonָ��ķ�ʽ�ڸ��������䴫�ε�ʱ���öഫһ����ʾλ�õĲ���*/
int lept_parse(lept_value* lvalue, const char* json) {
	lept_contest lc;
	lc.json = json;
	assert(lvalue != NULL); //����Ҫ���json�Ƿ�Ϊnull,��Ϊ���ﲻ�Ǻ������ߵ����Σ������û������Σ�����������mark:question
	lvalue->type = LEPT_NULL; //Ĭ��Ϊnull

	lept_parse_whitespace(&lc);
	int ret = lept_parse_value(&lc, lvalue);
	lept_parse_whitespace(&lc);
	if (*(lc.json) != '\0') ret = LEPT_PARSE_NOTSIGULAR_VALUE;
	return ret;
}
