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

/*封装成一个结构体防止传入更多参数*/
typedef struct {
	const char* json;
	char* stack;
	size_t top, size;
}lept_contest;

#define ISDIGIT(c) (c>='0' && c<='9')
#define ISDIGIT1TO9(c) (c>='1'&&c<='9')

#define EXPECT(c,ch) do{\
		assert(*(c->json)==ch);\
		c->json++;\
	}while(0)

#define PUTC(con,ch) \
	do{ \
		*((char*)lept_contest_push(con,1)) = ch;\
	} while (0);
/*跳过空格*/
static void lept_parse_whitespace(lept_contest* lc) {
	char *t = lc->json;
	while (*t == ' '||*t =='\t'||*t == '\n'||*t == '\r')
	{
		t++;
	}
	lc->json = t;
}

static int lept_parse_literal(lept_contest*c, lept_value*v,const char* lit,lept_type type) {
	assert(lit != NULL&& lit[0]); //将空字符串传入strlen是undefined behavior
	int len = strlen(lit);
	char first_c = lit[0];

	EXPECT(c, first_c);//之所以这里还要except或者assert一下，是因为各个函数要有测试独立性
	char* t = c->json;
	//下面这条语句成功得利用||的特点，避免了越界访问的发生，还有结束符的原因
	int i = 0;
	while (i<len-1)//EXCEPT里面已经加了1了
	{
		if (t[i] != lit[i + 1]) 
			return LEPT_PARSE_INVALID_VALUE;
		i++;
	}
	c->json += len - 1; //EXCEPT里面已经加了1了
	v->type = type;
	return LEPT_PARSE_OK;
}
/*返回状态码*/
/* trick: 自己写的代码仅是判断strtod不符合json标准的部分，最后还是通过strtod来转换
由于除了TOO_BIG之外的错误全部有自己判断，不需要传入END来判断，也不需要通过end来重置c->json的位置
另外注意 notice: strtod判断number过大的返回错误的方式*/
static int lept_parse_number(lept_contest*c, lept_value* v) {
	char* p = c->json;
	if (*p == '-') p++; //走过第一个点,‘-’可选，所以其他情况不用返回错误
	//走下一个比过点
	if (*p == '0') p++;
	else if (ISDIGIT1TO9(*p)) {
		while (ISDIGIT1TO9(*p)) p++;
	}
	else return LEPT_PARSE_INVALID_VALUE; //此点是必须经过的，所以不是以上情况皆为错误

	//可选项frac
	if (*p == '.') {
		p++;
		if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;//小数点后至少有一个数字
		while (ISDIGIT(*p)) p++;
	}

	//可选项 exp
	if (*p == 'e' || *p == 'E') {
		p++;
		if (*p == '+' || *p == '-') p++;
		if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;//e后至少有一个数字
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

/* 返回一个地址，对它进行赋值,按照byte来压入
notice: 并不是在函数里面进行赋值
*/
static void* lept_contest_push(lept_contest* c, size_t size) {
	/*mistake: 忘了assert*/
	assert(size > 0);
	if (c->size == 0) {
		c->size = LEPT_PARSE_STACK_INIT_SIZE;
		c->stack = (char*)malloc(c->size * sizeof(char));
	}
	if (c->top + size > c->size) {
		while (c->top + size > c->size)
		{
			c->size += c->size >> 1; //1.5倍增长
			c->stack = (char*)realloc(c->stack, c->size * sizeof(char));
		}
	}
	c->top += size;
	/*qustion：这里就算不加上void也会隐式转换吧*/
	return (void*)(c->stack + c->top);
}
static void* lept_contest_pop(lept_contest* c,size_t size) {
	assert(c->top >= size);
	c->top -= size;
	return c->stack + (c->top -= size);
}
//mistake:不应该设置成staitc,作为set函数
/*
 * c: stack in lept_contest
 * size: lenth
 * return: void
 */
void lept_set_string(lept_value*v, const char* c,size_t size) {
	//没有必要传入lept_contest
	assert(v != NULL && (c != NULL || size == 0));
	/*mistake:没有free*/
	lept_free(v);
	/*mistake: realloc，这里并没有在构造函数的地方分配空间，而是在用的时候分配*/
	v->u.s.s = (char*)malloc( (size+1) * sizeof(char));
	memcpy(v->u.s.s, c, size);
	v->u.s.s[size] = '\0';
	v->type = LEPT_STRING;
	v->u.s.len = size;
}
/*解析到LEPT_PARSE_INVALID_STRING_ESCAPE回滚c->stack*/
static int lept_parse_string(lept_contest*c, lept_value* v) {
	EXPECT(c, '\"');
	size_t head = c->top; //mistake：开始我觉得stack就是head了，这只对目前的情况
	char* p = c->json; //EXPECT里 已经++
	int len = 0;
	while (1) {
		switch (*p)
		{
		case '"':  //这里加不加\都可以
			/*mistake:一开始写成了v->u.s.s = (char*)lept_contest_pop(c, len);
			这样在contest销毁后lept_value也没了,而且忘加结束符号了*/
			lept_set_string(v, c->stack, c->top- head);
			c->json = p + 1;
			return LEPT_PARSE_OK;
		case '\0':
			c->top = head;//mistake:这两行忘加了，因为返回的不是mistake,而是一种新的类型
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
			default:
				c->top = head; //notice: 这里要回滚的，而不是清空的，为了增强鲁棒性？
				return LEPT_PARSE_INVALID_STRING_ESCAPE;
			}
		default:
			if ((unsigned char)*p < 0x20) {
				c->top = head;
				return LEPT_PARSE_INVALID_STRING_CHAR;
			}
			PUTC(c, *p);
			p++;
			break;
		}
	}
	
}
/*用户用完lept_parse后需要运行本函数，free字符串分配空间*/
void lept_free(lept_value*v) {
	
	/*mistake:忘记做类型判断和assert*/
	assert(v != NULL);
	if (v->type == LEPT_STRING)
		free(v->u.s.s);
	v->type = LEPT_NULL;
}
/*这里之所以不写在lept_parse里，我觉得主要还是功能单一化，这个函数只是为了parse ws value ws中的
value，而不考虑别的*/
/*返回正确或者错误码*/
/*这种写法分成好几个小函数防止这个函数里面包含过多的if语句或者复杂的case语句*/
static int lept_parse_value(lept_contest* lc, lept_value*v) {
	switch (lc->json[0])
	{
	case 'n':return lept_parse_literal(lc,v,"null",LEPT_NULL); break;
	case 't':return lept_parse_literal(lc,v,"true",LEPT_TRUE); break;
	case 'f':return lept_parse_literal(lc,v,"false",LEPT_FALSE); break;
	case '\0':return LEPT_PARSE_EXPECT_VALUE; break;
	case '"':return lept_parse_string(lc, v);
	default: return lept_parse_number(lc,v);
		break;
	}

}

/*很巧妙得通过移动json指针的方式在各个函数间传参的时候不用多传一个表示位置的参数*/
int lept_parse(lept_value* lvalue, const char* json) {
	lept_contest lc;
	lc.json = json;
	lc.size = 0;
	lc.top = 0;
	assert(lvalue != NULL); //不需要检查json是否为null,因为这里不是函数作者的责任，而是用户的责任，这样理解对吗mark:question
	lvalue->type = LEPT_NULL; //默认为null

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
//mistake: 应该返回const char*
const char* lept_get_string(const lept_value*v) {
	assert(v != NULL && v->type == LEPT_STRING);
	return v->u.s.s;
}
size_t lept_get_string_length(const lept_value*v) {
	//mistake: 忘记assert 类型
	assert(v != NULL && v->type == LEPT_STRING);
	return v->u.s.len;
}
/*LEPT_TRUE 返回 1 FALSE 返回 0*/
int lept_get_boolean(const lept_value* v) {
	assert(v != NULL && (v->type == LEPT_FALSE || v->type == LEPT_TRUE));
	return v->type == LEPT_TRUE;
}

void lept_set_boolean(lept_value* v, int b) {
	//mistake: 没有下一行不严密
	lept_free(v);
	v->type = b ? LEPT_TRUE : LEPT_FALSE;
}

void lept_set_number(lept_value* v, double n) {
	lept_free(v);
	v->type = LEPT_NUMBER;
	v->u.n = n;
}