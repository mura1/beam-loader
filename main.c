#include <stdio.h>

struct operand {
	unsigned tag : 3;
	union {
		struct {
			unsigned flags : 1;
			unsigned value : 4;
		};
		struct {
			unsigned flags : 2;
			unsigned value_hi : 3;

			unsigned value_lo : 8;
		};
		struct {
			unsigned flags : 2;	
			unsigned len_code : 3;  /* +2 if len_code < 7 */

			unsigned value : 8; 	/* sub-tag must be TAG_u and count = value + 9 */
						/* The value must be fitted into one machine word except for primary tag TAG_i. */
		};
	};
};



enum {
	TAG_i,
	TAG_o,
	TAG_u,
};

struct state;
struct device {
	unsigned int (*step)(struct state *s);	
};


typedef void *(*step_t)(struct state *s);

struct state {
	step_t		step;

	unsigned char 	*buf;
	unsigned int 	len;

	unsigned int 	type;
	unsigned int 	value;

	unsigned int	varlen;
	unsigned int	error;
};


static void *step_init(struct state *s);
static void *step_tag_select(struct state *s);
static void *step_4bits(struct state *s);
static void *step_11bits(struct state *s);
static void *step_varlen(struct state *s);
static void *step_varbits_tag_i(struct state *s);
static void *step_varbits_tag_others(struct state *s);
static void *step_varbits_tag_others_word_A(struct state *s);
static void *step_varbits_tag_others_word_B(struct state *s);
static void *step_varbits_tag_others_word_C(struct state *s);

static void *step_init(struct state *s)
{
	return step_tag_select;
}
static void *step_tag_select(struct state *s)
{
	step_t ret;
	unsigned char dat = s->buf[0];

	s->type = dat & 0x07;

	if ((dat & 0x10) == 0) {
		return step_4bits;
	} else if ((dat & 0x10) == 0) {
		return step_11bits;
	} else {
		return step_varlen;
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
	return step_varbits_tag_i;
}
static void *step_varbits_tag_i_select(struct state *s)
{
	step_t ret;
	if (s->type == TAG_i)
		return step_varbits_tag_i;	
	else
		return step_varbits_tag_others;
}

static void *step_varbits_tag_i(struct state *s)
{
	switch
}
static void *step_varbits_tag_others(struct state *s)
{
	switch (s->varlen) {
	case WORD_SIZE+1:
		return step_varbits_tag_others_word_A;
	case WORD_SIZE:
		return step_varbits_tag_others_word_B;
	default:
		return step_varbits_tag_others_word_C;
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
	step = (step_t)step_init;

	while (step = step(s));
	
	return step ? -1 : 0;
};
int main(int argc, char **argv)
{
	return 0;
}
