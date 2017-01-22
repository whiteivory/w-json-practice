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

/*返回错误或正确码*/
static int lept_parse_null(lept_contest* c, lept_value*v) {
	EXCEPT(c, 'n');//之所以这里还要except或者assert一下，是因为各个函数要有测试独立性
	char* t = c->json;
	//下面这条语句成功得利用||的特点，避免了越界访问的发生，还有结束符的原因
	if (t[0] != 'u'|| t[1] != 'l'||t[2] != 'l') //EXCEPT里面已经加了1了
		return LEPT_PARSE_INVALID_VALUE;
	c->json += 3;//EXCEPT里面已经加了1了
	v->type = LEPT_NULL;
	return LEPT_PARSE_OK;
}
static int lept_parse_true(lept_contest* c, lept_value*v) {
	EXCEPT(c, 't');
	char*t = c->json;
	if (t[0] != 'r' || t[1] != 'u' || t[2] != 'e')
		return LEPT_PARSE_INVALID_VALUE;
	c->json += 3;
	v->type = LEPT_TRUE;
	return LEPT_PARSE_OK;
}
static int lept_parse_false(lept_contest* c, lept_value*v) {
	EXCEPT(c, 'f');
	char *t = c->json;
	if (t[0] != 'a' || t[1] != 'l' || t[2] != 's'||t[3]!='e')
		return LEPT_PARSE_INVALID_VALUE;
	c->json += 4;
	v->type = LEPT_FALSE;
	return LEPT_PARSE_OK;
}


/*这里之所以不写在lept_parse里，我觉得主要还是功能单一化，这个函数只是为了parse ws value ws中的
value，而不考虑别的*/
/*返回正确或者错误码*/
/*这种写法分成好几个小函数防止这个函数里面包含过多的if语句或者复杂的case语句*/
static int lept_parse_value(lept_contest* lc, lept_value*v) {
	switch (lc->json[0])
	{
	case 'n':return lept_parse_null(lc,v); break;
	case 't':return lept_parse_true(lc,v); break;
	case 'f':return lept_parse_false(lc,v); break;
	case '\0':return LEPT_PARSE_EXPECT_VALUE; break;
	default: return LEPT_PARSE_INVALID_VALUE;
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
