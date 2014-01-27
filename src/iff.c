
#include <stdio.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <beam/types.h>

static void print_buf(unsigned char *buf, size_t size)
{
	int i;
	
	if (size == 0)
		return;
	for (i = 0; i < size; i++)
		printf("%02x", buf[i]);
	printf("\r\n");
}


enum IFF_ERROR {
	OK,
	EIFF_INVALID_HEADER_MAGIC,
	EIFF_INVALID_FORM_TYPE,
	EIFF_FORM_LEN_TOO_SHORT,
	EIFF_INVALID_CHUNK_MAGIC,
};

enum {
	CHUNK_ATOM 	= 0,
	CHUNK_CODE 	= 1,
	CHUNK_STR	= 2,
	CHUNK_IMP	= 3,
	CHUNK_EXP	= 4,

	CHUNK_LAMBDA	= 5,
	CHUNK_LITERAL	= 6,
	CHUNK_ATTRIBUTE	= 7,
	CHUNK_COMPILE	= 8,
	CHUNK_LINE	= 9,
	CHUNK_LocT	= 10,
	CHUNK_Trac	= 11,
	CHUNK_Abst	= 12,
	CHUNKS_TOTAL
};


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


static int test_handler(struct slice *buf, void *state);
static slice_handler_t chunk_handler_list[CHUNKS_TOTAL] = {
	[CHUNK_ATOM] 	= test_handler,
	[CHUNK_CODE] 	= test_handler,
	[CHUNK_STR] 	= test_handler,
	[CHUNK_IMP] 	= test_handler,
	[CHUNK_EXP] 	= test_handler,

	[CHUNK_LAMBDA] 	= test_handler,
	[CHUNK_LITERAL]	= test_handler,
	[CHUNK_ATTRIBUTE] = test_handler,
	[CHUNK_COMPILE]	= test_handler,
	[CHUNK_LINE] 	= test_handler,
	[CHUNK_LocT] 	= test_handler,
	[CHUNK_Trac] 	= test_handler,
	[CHUNK_Abst] 	= test_handler,
};



struct iff_header {
	byte magic[4];
	byte length[4];
	byte type[4];
	byte data[0];
};
struct chunk_header {
	byte	magic[4];
	byte 	length[4];	
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


static int test_handler(struct slice *buf, void *state)
{
	printf("[test handler] [%.4s]:", (byte *)buf->addr);
	print_buf(buf->addr, buf->len);
	return 0;
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


static int iff_handler(struct slice *buf, slice_handler_t header_handler, slice_handler_t *chunks_handler_list, void *state)
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

int get_file_buffer(struct slice *s, char *filename)
{
	struct stat sb;
	int fd;
	int ret;
	void *addr;

	fd = open(filename, O_RDONLY);
	if (fd == -1)
		goto err0;

	if ((ret = fstat(fd, &sb)) != 0)
		goto err1;

	if ((addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
		goto err2;

	close(fd);

	s->addr = addr;
	s->cap = s->len = sb.st_size;

err2:	return errno;
err1:	return ret;
err0:	return fd;
}

int put_file_buf(struct slice *s)
{
	return munmap(s->addr, s->len);
}

struct iff_device {
	uint32_t data;
};





static struct iff_device iff_device;

int main(int argc, char **argv)
{
	struct slice s;
	int ret;
	
	ret = 0;

	if (argc != 2) 
		goto err;

	if ((ret = get_file_buffer(&s, argv[1])) != 0)
		goto err;

	ret = iff_handler(&s, test_handler, chunk_handler_list, &iff_device);
	printf("iff_handler()=%d\r\n", ret);

	put_file_buf(&s);
err:	return ret;
}
