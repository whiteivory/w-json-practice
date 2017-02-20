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
	

/*跳过空格*/
static void lept_parse_whitespace(lept_context* lc) {
	char *t = lc->json;
	while (*t == ' '||*t =='\t'||*t == '\n'||*t == '\r')
	{
		t++;
	}
	lc->json = t;
}

static int lept_parse_literal(lept_context*c, lept_value*v,const char* lit,lept_type type) {
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
static int lept_parse_number(lept_context*c, lept_value* v) {
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
static void* lept_context_push(lept_context* c, size_t size) {
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
	/*qustion：这里就算不加上void也会隐式转换吧*/
	void* ret = c->stack + c->top;
	c->top += size;
	
	return ret;
}
static void* lept_context_pop(lept_context* c,size_t size) {
	assert(c->top >= size);
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
/*字符串（4个char）转4位16进制unsigned int*/
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

//notice: 重构，为了parse key of object，在测试代码齐全的时候可以保证重构的可行性
/*
*ch: return the add of the stack,need copy yourself,and does not have \0
*
*/
static int lept_parse_string_raw(lept_context*c, char** ch, size_t* plen) {
	EXPECT(c, '\"');
	size_t head = c->top; //mistake：开始我觉得stack就是head了，这只对目前的情况
	unsigned u, u2;
	char* p = c->json; //EXPECT里 已经++
	while (1) {
		switch (*p)
		{
		case '"':  //这里加不加\都可以
				   /*mistake:一开始写成了v->u.s.s = (char*)lept_contest_pop(c, len);
				   这样在contest销毁后lept_value也没了,而且忘加结束符号了*/
			*plen = c->top - head;
			*ch = (const char*)lept_context_pop(c, *plen);

			//lept_set_string(v, (const char*)lept_context_pop(c, len), c->top - head);
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
				//c->top = head; //notice: 这里要回滚的，而不是清空的，为了增强鲁棒性？
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
/*解析到LEPT_PARSE_INVALID_STRING_ESCAPE回滚c->stack*/
static int lept_parse_string(lept_context*c, lept_value* v) {
	size_t len;
	char* s; //mistake: 命名混淆，一开始用ch命名，导致和c混淆
	int ret = lept_parse_string_raw(c, &s, &len);
	if (ret == LEPT_PARSE_OK)
		lept_set_string(v,s,len);
	return ret;

	
}

static int lept_parse_object(lept_context*c, lept_value* v) {
	EXPECT(c, '{');
	//init
	v->u.o.len = 0;
	//size_t size = v->u.a.len;
	size_t i = 0, size;
	int ret = 0;
	lept_member m; //notice: 注意这个技巧，如果放在循环里面处理出错情况对于m.k就会在每个ret里面都要根据条件释放，而放在外面就只需要在循环外面free一下即可
	char* head = c->stack + c->top;
	lept_parse_whitespace(c);
	//empty
	if (*c->json == '}') {
		c->json++;//mistake:　forget this
		v->type = LEPT_OBJECT;
		v->u.o.len = 0;
		v->u.o.m = NULL;
		return LEPT_PARSE_OK;
	}

	for (;;) {

		m.len = 0;//init

		//parse one lept_member
		lept_parse_whitespace(c);
		//key
		if (*c->json != '"') {
			ret = LEPT_PARSE_MISS_QUOTATION_MARK;
			break;
		}
		char* s;
		ret = lept_parse_string_raw(c, &s, &m.len);
		memcpy(m.k = (char*)malloc(m.len + 1), s, m.len); //notice: +1 for \0
		m.k[m.len] = '\0';

		if (ret != LEPT_PARSE_OK)
			break;
		lept_parse_whitespace(c);
		//colon ':'
		if (*c->json != ':') {
			free(m.k); //FREE KEY
			ret =  LEPT_PARSE_MISS_COLON;
			break;
		}
		c->json++;
		lept_parse_whitespace(c);
		//parse one lept_value as the v of lept_member(m)
		//important mistake: 我一开始m里面的是lept_value* v，这样虽然也可以，但是每次用的时候都要记得malloc和free
		//e.g. malloc(sizeof(lept_value)) 直接设置成lept_value v 可以更符合本环境，更方便操作
		lept_init(&m.v);
		ret = lept_parse_value(c, &m.v);
		if (ret != LEPT_PARSE_OK) 
			break;
		//copy to stack
		//notice:这里和解析array的时候一样，不能直接在stack上，这样v的引用会在重新分配栈的时候保持原地址造成野指针
		//所以两个cpy是必要的
		memcpy(lept_context_push(c, sizeof(lept_member)),&m,sizeof(lept_member));
		//len++
		v->u.o.len++;
		lept_parse_whitespace(c);
		//comma,允许最后一个后面有或者没有,
		if (*c->json == ',')
			c->json++;
		//FINISH
		if (*c->json == '}') {
			v->type = LEPT_OBJECT;
			c->json++;
			size = sizeof(lept_member)*v->u.o.len; //in bytes
			memcpy(v->u.o.m=malloc(size), lept_context_pop(c, size), size);//mistake: 没有malloc就直接memcpy是不行的，所以说有多少malloc就有多少free，跟memcpy是没有关系的
			return LEPT_PARSE_OK;
		}
	}

	//ret!=LEPT_PARSE_OK free malloc memory
	//important notice:
	free(m.k);//这里的技巧见m的定义处
	for (i=0; i < v->u.o.len; i++) {
		lept_member* m = (lept_member*)lept_context_pop(c, sizeof(lept_member));
		//important notice:注意这种直接从内存转化成变量的赋值方式
		free(m->k);
		lept_free(&m->v);
		/*
		或者context_pop一次pop出，
		再通过下标依次free，但上面的写法更自然
		*/
	}
	v->type = LEPT_NULL;
	return ret;

}

//mistake: 相互引用前向声明问题
static int lept_parse_value(lept_context* c, lept_value* v);
/*v->u.a 采用数组的方式存储LEPT_ARRAY类型，连续内存*/
static int lept_parse_array(lept_context*c, lept_value*v) {
	EXPECT(c, '[');
	//init
	v->u.a.len = 0;
	size_t size = v->u.a.len;
	size_t i = 0;
	lept_parse_whitespace(c);
	int ret = 0;
	//"[]"
	//mistake: *c-<json++ 这样的写法不行
	if (*c->json == ']') {
		c->json++;
		v->type = LEPT_ARRAY;
		//v->u.a.len = 0;
		v->u.a.e = NULL;
		return LEPT_PARSE_OK;
	}
	for (;;) {
		//parse and copy
		lept_value e;
		lept_init(&e);
		int ret = lept_parse_value(c, &e);
		if (ret != LEPT_PARSE_OK) break;
		v->u.a.len++;
		char* addr = lept_context_push(c, sizeof(lept_value));//wati untill ']' to pop to v->u.a.e
		memcpy(addr, &e, sizeof(lept_value));
		lept_parse_whitespace(c);
		//直接用c->json可避免出错
		//p = c->json;
		if (*c->json == ',') {
			//mistake: 应该在lept_parse_value后直接加，而不是在这里和']'
			//v->u.a.len++;
			c->json++;
			lept_parse_whitespace(c);
			continue;
		}
		//pop to u->u.a.e
		else if (*c->json == ']') {
			c->json++;
			//mistake: 忘记赋值type 
			v->type = LEPT_ARRAY;
			size = v->u.a.len * sizeof(lept_value);
			void* src = lept_context_pop(c, size);
			//mistake: 忘记 memset了，这样直接赋给野指针了
			v->u.a.e = (lept_value*)malloc(size);
			memcpy(v->u.a.e, src, size);
			//不出错就不需要break free了，因为成功返回后进行释放是使用者的责任
			return LEPT_PARSE_OK;
		}
		else {
			ret = LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
			break;
		}
	}
	//当错伏解析后要进行free
	for (i = 0; i < size; i++)
		lept_free((lept_value*)lept_context_pop(c, sizeof(lept_value)));
	return ret;

}
/*用户用完lept_parse后需要运行本函数，free字符串分配空间*/
void lept_free(lept_value* v) {
	size_t i;
	assert(v != NULL);
	switch (v->type) {
	case LEPT_STRING:
		free(v->u.s.s);
		break;
	case LEPT_ARRAY:
		for (i = 0; i < v->u.a.len; i++)
			lept_free(&v->u.a.e[i]);
		free(v->u.a.e); //important mistake: 这个数组也是在parse里面malloc的最大的一个空间，需要free，有多少malloc就应该有多少free(而memcpy不是，因为memcpy需要copy到已经malloc的里面）
		break;
	case LEPT_OBJECT:
		for (i = 0; i < v->u.o.len; i++) { //只free动态分配的部分
			free(v->u.o.m[i].k); //free string
			lept_free(&v->u.o.m[i].v);//free lept_value v
		}
		free(v->u.o.m);
		break;
	default: break;
	}
	v->type = LEPT_NULL;
}
/*这里之所以不写在lept_parse里，我觉得主要还是功能单一化，这个函数只是为了parse ws value ws中的
value，而不考虑别的*/
/*返回正确或者错误码*/
/*这种写法分成好几个小函数防止这个函数里面包含过多的if语句或者复杂的case语句*/
static int lept_parse_value(lept_context* lc, lept_value*v) {
	switch (lc->json[0])
	{
	case 'n':return lept_parse_literal(lc,v,"null",LEPT_NULL); break;
	case 't':return lept_parse_literal(lc,v,"true",LEPT_TRUE); break;
	case 'f':return lept_parse_literal(lc,v,"false",LEPT_FALSE); break;
	case '\0':return LEPT_PARSE_EXPECT_VALUE; break;
	case '"':return lept_parse_string(lc, v);
	case '[':return lept_parse_array(lc, v);
	case'{':return lept_parse_object(lc, v);
	default: return lept_parse_number(lc,v);
		break;
	}

}

/*很巧妙得通过移动json指针的方式在各个函数间传参的时候不用多传一个表示位置的参数*/
int lept_parse(lept_value* lvalue, const char* json) {
	lept_context lc;
	//big mistake: lc.stack必须要赋初值，否则如果没有调用lept_push进行malloc，free的时候就会出问题
	lc.stack = NULL;
	lc.json = json;
	lc.size = 0;
	lc.top = 0;
	assert(lvalue != NULL); //不需要检查json是否为null,因为这里不是函数作者的责任，而是用户的责任，这样理解对吗mark:question
	lvalue->type = LEPT_NULL; //默认为null

	lept_parse_whitespace(&lc);
	int ret = lept_parse_value(&lc, lvalue);
	lept_parse_whitespace(&lc);
	if (ret == LEPT_PARSE_OK&&*(lc.json) != '\0') ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
	//notice: 严谨
	assert(lc.top == 0);
	//free
	free(lc.stack);
	
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


size_t lept_get_array_size(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_ARRAY);
	return v->u.a.len;
}

lept_value* lept_get_array_element(const lept_value* v, size_t index) {
	assert(v != NULL && v->type == LEPT_ARRAY);
	assert(index < v->u.a.len);
	return &v->u.a.e[index];
}

size_t lept_get_object_size(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_OBJECT);
	return v->u.o.len;
}
const char* lept_get_object_key(const lept_value* v, size_t index) {
	assert(v != NULL && v->type == LEPT_OBJECT);
	assert(index < v->u.o.len);
	return v->u.o.m[index].k;
}

size_t lept_get_object_key_length(const lept_value* v, size_t index) {
	assert(v != NULL && v->type == LEPT_OBJECT);
	assert(index < v->u.o.len);
	return v->u.o.m[index].len;
}

lept_value* lept_get_object_value(const lept_value* v, size_t index) {
	assert(v != NULL && v->type == LEPT_OBJECT);
	assert(index < v->u.o.len);
	return &v->u.o.m[index].v;
}