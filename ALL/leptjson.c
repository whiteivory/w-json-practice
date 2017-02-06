#include"leptjson.h"
#include<assert.h>
/*封装成一个结构体防止传入更多参数*/
typedef struct {
	const char* json;
}lept_contest;

#define EXCEPT(c,ch) do{\
		assert(*(c->json)==ch);\
		c->json++;\
	}while(0)

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

	EXCEPT(c, first_c);//之所以这里还要except或者assert一下，是因为各个函数要有测试独立性
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
static int lept_parse_number(lept_contest*c,lept_value* v) {
	char* end;
	
	double n = strtod(c->json, &end);
	if (end != c->json) {
		c->json = end;
		v->type = LEPT_NUMBER;
		v->n = n;
		return LEPT_PARSE_OK;
	}
	else
		return LEPT_PARSE_INVALID_VALUE;

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
	default: return lept_parse_number(lc,v);
		break;
	}

}

/*很巧妙得通过移动json指针的方式在各个函数间传参的时候不用多传一个表示位置的参数*/
int lept_parse(lept_value* lvalue, const char* json) {
	lept_contest lc;
	lc.json = json;
	assert(lvalue != NULL); //不需要检查json是否为null,因为这里不是函数作者的责任，而是用户的责任，这样理解对吗mark:question
	lvalue->type = LEPT_NULL; //默认为null

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