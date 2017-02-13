#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include"leptjson.h"
#include<assert.h>
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

/*��װ��һ���ṹ���ֹ����������*/
typedef struct {
	const char* json;
	char* stack;
	size_t top, size;
}lept_context;

#define ISDIGIT(c) (c>='0' && c<='9')
#define ISDIGIT1TO9(c) (c>='1'&&c<='9')

#define EXPECT(c,ch) do{\
		assert(*(c->json)==ch);\
		c->json++;\
	}while(0)

#define PUTC(con,ch) \
	do{ \
		*((char*)lept_context_push(con,1)) = ch;\
	} while (0)

#define STRING_ERROR(ret) do{\
	c->top = head;\
	return ret;}while(0)
	

/*�����ո�*/
static void lept_parse_whitespace(lept_context* lc) {
	char *t = lc->json;
	while (*t == ' '||*t =='\t'||*t == '\n'||*t == '\r')
	{
		t++;
	}
	lc->json = t;
}

static int lept_parse_literal(lept_context*c, lept_value*v,const char* lit,lept_type type) {
	assert(lit != NULL&& lit[0]); //�����ַ�������strlen��undefined behavior
	int len = strlen(lit);
	char first_c = lit[0];

	EXPECT(c, first_c);//֮�������ﻹҪexcept����assertһ�£�����Ϊ��������Ҫ�в��Զ�����
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
static int lept_parse_number(lept_context*c, lept_value* v) {
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
	v->u.n = strtod(c->json, NULL);
	if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
		return LEPT_PARSE_NUMBER_TOO_BIG;
	v->type = LEPT_NUMBER;
	c->json = p;
	return LEPT_PARSE_OK;
}

/* ����һ����ַ���������и�ֵ,����byte��ѹ��
notice: �������ں���������и�ֵ
*/
static void* lept_context_push(lept_context* c, size_t size) {
	/*mistake: ����assert*/
	assert(size > 0);
	if (c->size == 0) {
		c->size = LEPT_PARSE_STACK_INIT_SIZE;
		c->stack = (char*)malloc(c->size * sizeof(char));
	}
	if (c->top + size > c->size) {
		while (c->top + size > c->size)
		{
			c->size += c->size >> 1; //1.5������
			c->stack = (char*)realloc(c->stack, c->size * sizeof(char));
		}
	}
	/*qustion��������㲻����voidҲ����ʽת����*/
	void* ret = c->stack + c->top;
	c->top += size;
	
	return ret;
}
static void* lept_contest_pop(lept_context* c,size_t size) {
	assert(c->top >= size);
	c->top -= size;
	return c->stack + (c->top -= size);
}
//mistake:��Ӧ�����ó�staitc,��Ϊset����
/*
 * c: stack in lept_contest
 * size: lenth
 * return: void
 */
void lept_set_string(lept_value*v, const char* c,size_t size) {
	//û�б�Ҫ����lept_contest
	assert(v != NULL && (c != NULL || size == 0));
	/*mistake:û��free*/
	lept_free(v);
	/*mistake: realloc�����ﲢû���ڹ��캯���ĵط�����ռ䣬�������õ�ʱ�����*/
	v->u.s.s = (char*)malloc( (size+1) * sizeof(char));
	memcpy(v->u.s.s, c, size);
	v->u.s.s[size] = '\0';
	v->type = LEPT_STRING;
	v->u.s.len = size;
}
/*�ַ�����4��char��ת4λ16����unsigned int*/
static const char* lept_parse_hex4(const char* p, unsigned* u) {
	int i;
	*u = 0;
	for (i = 0; i < 4; i++) {
		char ch = *p++;
		*u <<= 4;
		if (ch >= '0' && ch <= '9')  *u |= ch - '0';
		else if (ch >= 'A' && ch <= 'F')  *u |= ch - ('A' - 10);
		else if (ch >= 'a' && ch <= 'f')  *u |= ch - ('a' - 10);
		else return NULL;
	}
	return p;
}

static void lept_encode_utf8(lept_context* c, unsigned u) {

	if (u <= 0x7F) //0X7F 111 1111
		PUTC(c, u & 0xFF);//0XFF 1111 1111
	else if (u <= 0x7FF) {
		PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
		PUTC(c, 0x80 | (u & 0x3F));
	}
	else if (u <= 0xFFFF) {
		PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
		PUTC(c, 0x80 | ((u >> 6) & 0x3F));
		PUTC(c, 0x80 | (u & 0x3F));
	}
	else {
		assert(u <= 0x10FFFF);
		PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
		PUTC(c, 0x80 | ((u >> 12) & 0x3F));
		PUTC(c, 0x80 | ((u >> 6) & 0x3F));
		PUTC(c, 0x80 | (u & 0x3F));
	}
}


/*������LEPT_PARSE_INVALID_STRING_ESCAPE�ع�c->stack*/
static int lept_parse_string(lept_context*c, lept_value* v) {
	EXPECT(c, '\"');
	size_t head = c->top; //mistake����ʼ�Ҿ���stack����head�ˣ���ֻ��Ŀǰ�����
	unsigned u, u2;
	char* p = c->json; //EXPECT�� �Ѿ�++
	int len = 0;
	while (1) {
		switch (*p)
		{
		case '"':  //����Ӳ���\������
			/*mistake:һ��ʼд����v->u.s.s = (char*)lept_contest_pop(c, len);
			������contest���ٺ�lept_valueҲû��,�������ӽ���������*/
			lept_set_string(v, c->stack, c->top- head);
			c->json = p + 1;
			return LEPT_PARSE_OK;
		case '\0':
			c->top = head;//mistake:�����������ˣ���Ϊ���صĲ���mistake,����һ���µ�����
			p++;
			return LEPT_PARSE_MISS_QUOTATION_MARK;
		case '\\':
			p++;
			switch (*p)
			{
			case '\"': PUTC(c, '\"'); break; 
			case '\\': PUTC(c, '\\'); break;
			case '/':  PUTC(c, '/'); break;
			case 'b':  PUTC(c, '\b'); break;
			case 'f':  PUTC(c, '\f'); break;
			case 'n':  PUTC(c, '\n'); break;
			case 'r':  PUTC(c, '\r'); break;
			case 't':  PUTC(c, '\t'); break;
			case 'u':
				p++;
				if (!(p = lept_parse_hex4(p, &u)))
					STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
				if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
					if (*p++ != '\\')
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
					if (*p++ != 'u')
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
					if (!(p = lept_parse_hex4(p, &u2)))
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
					if (u2 < 0xDC00 || u2 > 0xDFFF)
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
					u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
				}
				lept_encode_utf8(c, u);
				break;
			default:
				//c->top = head; //notice: ����Ҫ�ع��ģ���������յģ�Ϊ����ǿ³���ԣ�
				//return LEPT_PARSE_INVALID_STRING_ESCAPE;
				STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
			}
		default:
			if ((unsigned char)*p < 0x20) {
				STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
			}
			PUTC(c, *p);
			p++;
			break;
		}
	}
	
}
//mistake: �໥����ǰ����������
static int lept_parse_value(lept_context* c, lept_value* v);
/*v->u.a ��������ķ�ʽ�洢LEPT_ARRAY���ͣ������ڴ�*/
static int lept_parse_array(lept_context*c, lept_value*v) {
	EXPECT(c, '[');
	//init
	v->u.a.len = 0;
	//"[]"
	if (*(c->json) == ']') {
		v->type = LEPT_ARRAY;
		v->u.a.len = 0;
		v->u.a.e = NULL;
		return LEPT_PARSE_OK;
	}
	for (;;) {
		//parse and copy
		lept_value e;
		lept_init(&e);
		int ret = lept_parse_value(c, &e);
		if (ret != LEPT_PARSE_OK) return ret;
		v->u.a.len++;
		char* addr = lept_context_push(c, sizeof(lept_value));//wati untill ']' to pop to v->u.a.e
		memcpy(addr, &e, sizeof(lept_value));
		//ֱ����c->json�ɱ������
		//p = c->json;
		if (*(c->json)++ == ',') {
			//mistake: Ӧ����lept_parse_value��ֱ�Ӽӣ��������������']'
			//v->u.a.len++;
			continue;
		}
		//pop to u->u.a.e
		else if (*(c->json)++ == ']') {
			v->u.a.len++;
			//mistake: ���Ǹ�ֵtype 
			v->type = LEPT_ARRAY;
			size_t size = v->u.a.len * sizeof(lept_value);
			void* src = lept_contest_pop(c, size);
			//mistake: ���� memset�ˣ�����ֱ�Ӹ���Ұָ����
			v->u.a.e = (lept_value*)malloc(size);
			memcpy(v->u.a.e, src, size);
			return LEPT_PARSE_OK;
		}
		else
			return LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
	}
	

}
/*�û�����lept_parse����Ҫ���б�������free�ַ�������ռ�*/
void lept_free(lept_value*v) {
	
	/*mistake:�����������жϺ�assert*/
	assert(v != NULL);
	if (v->type == LEPT_STRING)
		free(v->u.s.s);
	v->type = LEPT_NULL;
}
/*����֮���Բ�д��lept_parse��Ҿ�����Ҫ���ǹ��ܵ�һ�����������ֻ��Ϊ��parse ws value ws�е�
value���������Ǳ��*/
/*������ȷ���ߴ�����*/
/*����д���ֳɺü���С������ֹ�������������������if�����߸��ӵ�case���*/
static int lept_parse_value(lept_context* lc, lept_value*v) {
	switch (lc->json[0])
	{
	case 'n':return lept_parse_literal(lc,v,"null",LEPT_NULL); break;
	case 't':return lept_parse_literal(lc,v,"true",LEPT_TRUE); break;
	case 'f':return lept_parse_literal(lc,v,"false",LEPT_FALSE); break;
	case '\0':return LEPT_PARSE_EXPECT_VALUE; break;
	case '"':return lept_parse_string(lc, v);
	case '[':return lept_parse_array(lc, v);
	default: return lept_parse_number(lc,v);
		break;
	}

}

/*�������ͨ���ƶ�jsonָ��ķ�ʽ�ڸ��������䴫�ε�ʱ���öഫһ����ʾλ�õĲ���*/
int lept_parse(lept_value* lvalue, const char* json) {
	lept_context lc;
	lc.json = json;
	lc.size = 0;
	lc.top = 0;
	assert(lvalue != NULL); //����Ҫ���json�Ƿ�Ϊnull,��Ϊ���ﲻ�Ǻ������ߵ����Σ������û������Σ�����������mark:question
	lvalue->type = LEPT_NULL; //Ĭ��Ϊnull

	lept_parse_whitespace(&lc);
	int ret = lept_parse_value(&lc, lvalue);
	lept_parse_whitespace(&lc);
	if (ret == LEPT_PARSE_OK&&*(lc.json) != '\0') ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
	return ret;
}

// get method
lept_type lept_get_type(const lept_value* v) {
	return v->type;
}
double lept_get_number(const lept_value*v) {
	assert(v != NULL && v->type == LEPT_NUMBER);
	return v->u.n;
}
//mistake: Ӧ�÷���const char*
const char* lept_get_string(const lept_value*v) {
	assert(v != NULL && v->type == LEPT_STRING);
	return v->u.s.s;
}
size_t lept_get_string_length(const lept_value*v) {
	//mistake: ����assert ����
	assert(v != NULL && v->type == LEPT_STRING);
	return v->u.s.len;
}
/*LEPT_TRUE ���� 1 FALSE ���� 0*/
int lept_get_boolean(const lept_value* v) {
	assert(v != NULL && (v->type == LEPT_FALSE || v->type == LEPT_TRUE));
	return v->type == LEPT_TRUE;
}

void lept_set_boolean(lept_value* v, int b) {
	//mistake: û����һ�в�����
	lept_free(v);
	v->type = b ? LEPT_TRUE : LEPT_FALSE;
}

void lept_set_number(lept_value* v, double n) {
	lept_free(v);
	v->type = LEPT_NUMBER;
	v->u.n = n;
}


size_t lept_get_array_size(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_ARRAY);
	return v->u.a.len;
}

lept_value* lept_get_array_element(const lept_value* v, size_t index) {
	assert(v != NULL && v->type == LEPT_ARRAY);
	assert(index < v->u.a.len);
	return &v->u.a.e[index];
}
