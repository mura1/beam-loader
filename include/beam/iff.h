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


extern int iff_handler(struct slice *buf, slice_handler_t header_handler, slice_handler_t *chunks_handler_list, void *state);
