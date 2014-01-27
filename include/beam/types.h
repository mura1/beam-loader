typedef unsigned int 	uint32_t;
typedef unsigned short 	uint16_t;
typedef unsigned char	uint8_t;
typedef unsigned char	byte;

typedef unsigned int	size_t;

#define NULL 		((void *)0)

struct slice {
	void 	*addr;
	size_t	len;
	size_t	cap;
};


typedef int (*slice_handler_t)(struct slice *buf, void *state);
