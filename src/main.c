#include <stdio.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <beam/types.h>
#include <beam/errno.h>
#include <beam/atom_table.h>
#include <beam/iff.h>


static void print_buf(unsigned char *buf, size_t size)
{
	int i;
	
	if (size == 0)
		return;
	for (i = 0; i < size; i++)
		printf("%02x", buf[i]);
	printf("\r\n");
}
int dummy_handler(struct slice *buf, void *state)
{
	printf("[dummy handler] [%.4s]:", (byte *)buf->addr);
	print_buf(buf->addr, buf->len);
	return 0;
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





static int atom_chunk_handler(struct slice *buf, void *state) __attribute__((__weakref__("dummy_handler")));
static int code_chunk_handler(struct slice *buf, void *state) __attribute__((__weakref__("dummy_handler")));
static int str_chunk_handler(struct slice *buf, void *state) __attribute__((__weakref__("dummy_handler")));
static int imp_chunk_handler(struct slice *buf, void *state) __attribute__((__weakref__("dummy_handler")));
static int exp_chunk_handler(struct slice *buf, void *state) __attribute__((__weakref__("dummy_handler")));

static int lambda_chunk_handler(struct slice *buf, void *state) __attribute__((__weakref__("dummy_handler")));
static int literal_chunk_handler(struct slice *buf, void *state) __attribute__((__weakref__("dummy_handler")));
static int attrubute_chunk_handler(struct slice *buf, void *state) __attribute__((__weakref__("dummy_handler")));
static int compile_chunk_handler(struct slice *buf, void *state) __attribute__((__weakref__("dummy_handler")));
static int line_chunk_handler(struct slice *buf, void *state) __attribute__((__weakref__("dummy_handler")));
static int loc_chunk_handler(struct slice *buf, void *state) __attribute__((__weakref__("dummy_handler")));
static int trac_chunk_handler(struct slice *buf, void *state) __attribute__((__weakref__("dummy_handler")));
static int abst_chunk_handler(struct slice *buf, void *state) __attribute__((__weakref__("dummy_handler")));

static slice_handler_t chunk_handler_list[CHUNKS_TOTAL] = {
	[CHUNK_ATOM] 	= atom_chunk_handler,
	[CHUNK_CODE] 	= code_chunk_handler,
	[CHUNK_STR] 	= str_chunk_handler,
	[CHUNK_IMP] 	= imp_chunk_handler,
	[CHUNK_EXP] 	= exp_chunk_handler,

	[CHUNK_LAMBDA] 	= lambda_chunk_handler,
	[CHUNK_LITERAL]	= literal_chunk_handler,
	[CHUNK_ATTRIBUTE] = attrubute_chunk_handler,
	[CHUNK_COMPILE]	= compile_chunk_handler,
	[CHUNK_LINE] 	= line_chunk_handler,
	[CHUNK_LocT] 	= loc_chunk_handler,
	[CHUNK_Trac] 	= trac_chunk_handler,
	[CHUNK_Abst] 	= abst_chunk_handler,
};




static parse_argv_get_char(struct slice *to_head, struct slice *to_tail, struct slice *from, byte c)
{
	int yes = *(byte *)from->addr == c;

	to_head->addr = from->addr;
	to_tail->addr = yes ? from->addr + 1 : from->addr;
	to_head->len = yes ? 1 : 0; 
	to_tail->len = yes ? from->len-1 : from->len;
	to_head->cap = to_tail->cap = from->cap;
}

static parse_argv_get_string(struct slice *to_head, struct slice *to_tail, struct slice *from, byte *s)
{
	int yes = (strncmp(from->addr, s, from->len) == 0);
	
}
int parse_argv(struct slice *to_head, struct slice *to_tail, struct slice *from, byte *format)
{

	to_head->addr = from->addr;
	to_head->len = from->len;
	to_head->cap = from->cap;

	do {
	
		
	} while (to_head->len > 0);

	
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

	ret = iff_handler(&s, dummy_handler, chunk_handler_list, &iff_device);
	printf("iff_handler()=%d\r\n", ret);

	put_file_buf(&s);
err:	return ret;
}
