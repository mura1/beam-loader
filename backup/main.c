#include <stdio.h>

#define SIZE_OF_WORD sizeof(unsigned int)

#define SMALL_BITS (28)
#define IS_SMALL_INTEGER(n) (-0x8000000<=(n) && (n)<=0x07ffffff) 

/*
	+--------+
	|xxxxxxxx|    MSB is the leftmost bit as convention.
	+--------+
	 H      L (Order)

	
	+--------+
	|vvvv0TTT|            Single-byte tagged value
	+--------+

	
	+--------+--------+
	|hhh10TTT|llllllll|   Double-byte tagged value
	+--------+--------+


	tag != TAG_i

	+--------+--------+--------+--------+--------+--------+
	|LLL?1TTT|mmmmmmmm|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|   TAG_o if the leftmost byte != 0
	+--------+--------+--------+--------+--------+--------+
	+--------+--------+--------+--------+--------+
	|LLL?1TTT|vvvvvvvv|vvvvvvvv|vvvvvvvv|mvvvvvvv|
	+--------+--------+--------+--------+--------+
	+--------+--------+--------+--------+
	|LLL?1TTT|vvvvvvvv|vvvvvvvv|mvvvvvvv|                     TAG_o if the most significant bit(MSB) != 0
	+--------+--------+--------+--------+
	+--------+--------+--------+
	|LLL?1TTT|vvvvvvvv|mvvvvvvv|
	+--------+--------+--------+
	+--------+--------+
	|LLL?1TTT|mvvvvvvv|
	+--------+--------+

	tag == TAG_i

	+--------+--------+--------+--------+--------+
	|LLL?1TTT|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|
	+--------+--------+--------+--------+--------+
	+--------+--------+--------+--------+
	|LLL?1TTT|vvvvvvvv|vvvvvvvv|vvvvvvvv|
	+--------+--------+--------+--------+
	+--------+--------+--------+
	|LLL?1TTT|vvvvvvvv|vvvvvvvv|
	+--------+--------+--------+
	+--------+--------+
	|LLL?1TTT|vvvvvvvv|
	+--------+--------+


	+--------+--------+~~~           ~~~+--------+
	|LLL?1TTT|vvvvvvvv|...           ...|vvvvvvvv|
	+--------+--------+~~~           ~~~+--------+


*/

enum {
	TAG_i,
	TAG_o,
	TAG_u,
	TAG_q,
};


struct state {
	unsigned char 	*buf;
	unsigned int 	len;

	unsigned int 	type;
	unsigned int 	value;

	unsigned int	varlen;
	unsigned int	error;
};

typedef void *(*step_t)(struct state *s);


static void *step_start(struct state *s);
static void *step_tag_select(struct state *s);
static void *step_4bits(struct state *s);
static void *step_11bits(struct state *s);
static void *step_varlen(struct state *s);
static void *step_varbits_tag_i_or_others(struct state *s);

static void *step_varbits_tag_i(struct state *s);
static void *step_varbits_tag_i_smallnum_tag_i(struct state *s);
static void *step_varbits_tag_i_smallnum_tag_q(struct state *s);
static void *step_varbits_tag_i_bignum_tag_q(struct state *s);


static void *step_varbits_tag_others(struct state *s);
static void *step_varbits_tag_others_word_A(struct state *s);
static void *step_varbits_tag_others_word_B(struct state *s);
static void *step_varbits_tag_others_word_C(struct state *s);

int operand_decode(struct state *s, unsigned char *buf, unsigned int size);

static unsigned int __create_small_integer(unsigned char *buf, unsigned int len)
{
	unsigned int i, ret;
	for (i = 0; i < len; i++)
		ret |= buf[i] << (8*i);
	return ret;
}
static unsigned int __get_literal_address(unsigned char *buf, unsigned int len)
{
	return 0;
}


static unsigned int __get_len_word(struct state *s)
{
	operand_decode(s, s->buf, s->len);
	if (s->type != TAG_u) {
		s->error = 1;
		return 0;
	}
	return s->value;	
}

static void *step_start(struct state *s)
{
	return step_tag_select(s);
}
static void *step_tag_select(struct state *s)
{
	step_t ret;
	unsigned char dat = s->buf[0];

	s->type = dat & 0x07;

	if ((dat & 0x08) == 0) {
		return step_4bits(s);
	} else if ((dat & 0x10) == 0) {
		return step_11bits(s);
	} else {
		return step_varlen(s);
	}
}
static void *step_4bits(struct state *s)
{
	unsigned int val = (unsigned int)s->buf[0];

	s->type = val & 0x07;
	s->value = val >> 4;
	s->buf++;
	s->len--;
	return 0;
}
static void *step_11bits(struct state *s)
{
	unsigned int val0 = (unsigned int)s->buf[0];
	unsigned int val1 = (unsigned int)s->buf[1];

	s->type = val0 & 0x07;
	s->value = ((val0 >> 5) << 8) | val1; 
	s->buf++; 
	s->len--;
	return 0;
}
static void *step_varlen(struct state *s)
{
	struct state __s;
	step_t ret;
	unsigned char val;
	unsigned int len_code;
	unsigned int len_word;
	unsigned int len;

	val = s->buf[0];
	len_code  = val >> 5;

	if (len_code < 7)
		len = len_code+2;
	else {
		len_word = __get_len_word(s);
		if (s->error)
			return 0;
		len = len_word+9;	
	}
	s->varlen = len;
	return step_varbits_tag_i_or_others(s);
}
static void *step_varbits_tag_i_or_others(struct state *s)
{
	step_t ret;
	if (s->type == TAG_i)
		return step_varbits_tag_i(s);
	else
		return step_varbits_tag_others(s);
}

static void *step_varbits_tag_i(struct state *s)
{
	unsigned int val = *(unsigned int *)&s->buf[0];

	if (s->varlen <= SIZE_OF_WORD) {
		if (IS_SMALL_INTEGER(val))
			return step_varbits_tag_i_smallnum_tag_i(s);
		else
			return step_varbits_tag_i_smallnum_tag_q(s);
	} else {
		return step_varbits_tag_i_bignum_tag_q(s);
	}
}
static void *step_varbits_tag_i_smallnum_tag_i(struct state *s)
{
	s->value = __create_small_integer(s->buf, s->varlen);
	s->type = TAG_i;
	return 0;
}
static void *step_varbits_tag_i_smallnum_tag_q(struct state *s)
{
	s->value = __get_literal_address(s->buf, s->varlen);
	s->type = TAG_q;
	return 0;
}
static void *step_varbits_tag_i_bignum_tag_q(struct state *s)
{
	s->value = __get_literal_address(s->buf, s->varlen);
	s->type = TAG_q;
	return 0;
}

static void *step_varbits_tag_others(struct state *s)
{
	switch (s->varlen) {
	case SIZE_OF_WORD+1:
		return step_varbits_tag_others_word_A(s);
	case SIZE_OF_WORD:
		return step_varbits_tag_others_word_B(s);
	default:
		return step_varbits_tag_others_word_C(s);
	}
}

static void *step_varbits_tag_others_word_A(struct state *s)
{
	unsigned int msb;
	msb = s->buf[0];	

	if (msb)
		s->type = TAG_o;
	s->value = *(unsigned int *)&s->buf[1];
	return 0;
}
static void *step_varbits_tag_others_word_B(struct state *s)
{
	s->value = *(unsigned int *)&s->buf[0];
	return 0;
}
static void *step_varbits_tag_others_word_C(struct state *s)
{
	s->value = s->buf[0] | (s->buf[1] << 8) | (s->buf[2] << 16);	
	return 0;	
}


int operand_decode(struct state *s, unsigned char *buf, unsigned int size)
{
	step_t step;

	s->buf = buf;
	s->len = size;
	s->error = 0;
	step_start(s);
	s->buf += s->varlen;
	s->len -= s->varlen;
	
	return step ? -1 : 0;
};
int main(int argc, char **argv)
{
	return 0;
}

