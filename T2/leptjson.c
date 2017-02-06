#include"leptjson.h"
#include<assert.h>
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */

/*��װ��һ���ṹ���ֹ����������*/
typedef struct {
	const char* json;
}lept_contest;

#define ISDIGIT(c) (c>='0' && c<='9')
#define ISDIGIT1TO9(c) (c>='1'&&c<='9')

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

static int lept_parse_literal(lept_contest*c, lept_value*v,const char* lit,lept_type type) {
	assert(lit != NULL&& lit[0]); //�����ַ�������strlen��undefined behavior
	int len = strlen(lit);
	char first_c = lit[0];

	EXCEPT(c, first_c);//֮�������ﻹҪexcept����assertһ�£�����Ϊ��������Ҫ�в��Զ�����
	char* t = c->json;
	//�����������ɹ�������||���ص㣬������Խ����ʵķ��������н�������ԭ��
	int i = 0;
	while (i<len-1)//EXCEPT�����Ѿ�����1��
	{
		if (t[i] != lit[i + 1]) 
			return LEPT_PARSE_INVALID_VALUE;
		i++;
	}
	c->json += len - 1; //EXCEPT�����Ѿ�����1��
	v->type = type;
	return LEPT_PARSE_OK;
}
/*����״̬��*/
/* trick: �Լ�д�Ĵ�������ж�strtod������json��׼�Ĳ��֣������ͨ��strtod��ת��
���ڳ���TOO_BIG֮��Ĵ���ȫ�����Լ��жϣ�����Ҫ����END���жϣ�Ҳ����Ҫͨ��end������c->json��λ��
����ע�� notice: strtod�ж�number����ķ��ش���ķ�ʽ*/
static int lept_parse_number(lept_contest*c, lept_value* v) {
	char* p = c->json;
	if (*p == '-') p++; //�߹���һ����,��-����ѡ����������������÷��ش���
	//����һ���ȹ���
	if (*p == '0') p++;
	else if (ISDIGIT1TO9(*p)) {
		while (ISDIGIT1TO9(*p)) p++;
	}
	else return LEPT_PARSE_INVALID_VALUE; //�˵��Ǳ��뾭���ģ����Բ������������Ϊ����

	//��ѡ��frac
	if (*p == '.') {
		p++;
		if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;//С�����������һ������
		while (ISDIGIT(*p)) p++;
	}

	//��ѡ�� exp
	if (*p == 'e' || *p == 'E') {
		p++;
		if (*p == '+' || *p == '-') p++;
		if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;//e��������һ������
		while (ISDIGIT(*p)) p++;
	}

	errno = 0;
	v->n = strtod(c->json, NULL);
	if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
		return LEPT_PARSE_NUMBER_TOO_BIG;
	v->type = LEPT_NUMBER;
	c->json = p;
	return LEPT_PARSE_OK;
}

/*����֮���Բ�д��lept_parse��Ҿ�����Ҫ���ǹ��ܵ�һ�����������ֻ��Ϊ��parse ws value ws�е�
value���������Ǳ��*/
/*������ȷ���ߴ�����*/
/*����д���ֳɺü���С������ֹ�������������������if�����߸��ӵ�case���*/
static int lept_parse_value(lept_contest* lc, lept_value*v) {
	switch (lc->json[0])
	{
	case 'n':return lept_parse_literal(lc,v,"null",LEPT_NULL); break;
	case 't':return lept_parse_literal(lc,v,"true",LEPT_TRUE); break;
	case 'f':return lept_parse_literal(lc,v,"false",LEPT_FALSE); break;
	case '\0':return LEPT_PARSE_EXPECT_VALUE; break;
	default: return lept_parse_number(lc,v);
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
	if (ret == LEPT_PARSE_OK&&*(lc.json) != '\0') ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
	return ret;
}

lept_type lept_get_type(const lept_value* v) {
	return v->type;
}
double lept_get_number(const lept_value*v) {
	return v->n;
}