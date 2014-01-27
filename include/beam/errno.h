enum IFF_ERROR {
	OK,
	EIFF_INVALID_HEADER_MAGIC,
	EIFF_INVALID_FORM_TYPE,
	EIFF_FORM_LEN_TOO_SHORT,
	EIFF_INVALID_CHUNK_MAGIC,
	EIFF_UNIMPLEMENTED_HANDLER,

	EATOMTABLE_FULL,
	EATOMTABLE_NO_ATOM,
};


