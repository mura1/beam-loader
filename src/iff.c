
#include <string.h>

#include <beam/types.h>
#include <beam/errno.h>
#include <beam/atom_table.h>
#include <beam/iff.h>


#define CHUNK_MAGIC_LENGTH 4

static const byte CHUNK_MAGIC_TABLE[CHUNKS_TOTAL][CHUNK_MAGIC_LENGTH] = {
	[CHUNK_ATOM] 	= {'A','t','o','m'},
	[CHUNK_CODE] 	= {'C','o','d','e'},
	[CHUNK_STR] 	= {'S','t','r','T'},
	[CHUNK_IMP] 	= {'I','m','p','T'},
	[CHUNK_EXP] 	= {'E','x','p','T'},

	[CHUNK_LAMBDA] 	= {'F','u','n','T'},
	[CHUNK_LITERAL]	= {'L','i','t','T'},
	[CHUNK_ATTRIBUTE] = {'A','t','t','r'},
	[CHUNK_COMPILE]	= {'C','I','n','f'},
	[CHUNK_LINE] 	= {'L','i','n','e'},
	[CHUNK_LocT] 	= {'L','o','c','T'},
	[CHUNK_Trac] 	= {'T','r','a','c'},
	[CHUNK_Abst] 	= {'A','b','s','t'},
};

static uint32_t uint32_from_be(byte *buf)
{
	return ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
}


static int get_chunk_id(byte *buf)
{
	int i;
	
	for (i = 0; i < CHUNKS_TOTAL; i++) {
		if (memcmp(&CHUNK_MAGIC_TABLE[i][0], buf, CHUNK_MAGIC_LENGTH) == 0)
			return i; 
	}
	return -1;
}
static int get_next_chunks(struct slice *to_head, struct slice *to_tail, struct slice *from) 
{
	int ret;
	uint32_t chunk_len_aligned;

	if (from->len < 4)
		goto err0;

	if ((ret = get_chunk_id(from->addr)) < 0)
		goto err1;
		
	chunk_len_aligned = ((uint32_from_be(((struct chunk_header *)from->addr)->length) + 0x3) + sizeof(struct chunk_header)) & ~0x3;

	to_head->addr = from->addr;
	to_head->len = chunk_len_aligned;
	to_head->cap = from->cap;

	to_tail->addr = from->addr + chunk_len_aligned;
	to_tail->len = from->len - chunk_len_aligned;
	to_tail->cap = from->cap;	
	return ret;
err1:	return -EIFF_INVALID_CHUNK_MAGIC;
err0:	return -EIFF_FORM_LEN_TOO_SHORT;
}


static int get_iff_header(struct slice *head, struct slice *tail, struct slice *from)
{
	uint32_t form_len;

	if (memcmp(((struct iff_header *)from->addr)->magic, "FOR1", 4) != 0)
		goto err0;
	
	form_len = uint32_from_be(((struct iff_header *)from->addr)->length);

	if (memcmp(((struct iff_header *)from->addr)->type, "BEAM", 4) != 0)
		goto err1;
	
	
	head->addr = from->addr;
	head->len = sizeof(struct iff_header);
	head->cap = from->cap;

	tail->addr = from->addr + sizeof(struct iff_header);
	tail->len = from->len - sizeof(struct iff_header);
	tail->cap = from->cap;
	return 0;
err1:	return -EIFF_INVALID_FORM_TYPE;
err0:	return -EIFF_INVALID_HEADER_MAGIC;
}


int iff_handler(struct slice *buf, slice_handler_t header_handler, slice_handler_t *chunk_handler_list, void *state)
{
	struct slice head, tail;
	int ret;

	if ((ret = get_iff_header(&head, &tail, buf)) < 0)
		goto err0;

	if ((ret = header_handler(&head, state)) < 0)
		goto err0;

	do {
		if ((ret = get_next_chunks(&head, &tail, &tail)) < 0)
			goto err0;
		if ((ret = chunk_handler_list[ret](&head, state)) < 0)
			goto err0;
	} while (tail.len > 0);
	
err0:	return ret;
}


