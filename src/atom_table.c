#include <beam/types.h>

struct atom_key {
};

struct atom_value {
};

struct atom {
	struct atom_key;
	struct atom_value;	
};

struct atom_table {
	struct slice buf;
};


int atom_table_insert(struct atom_table *tbl, struct atom_key *from_k, struct atom_value *form_v)
{
	return 0;
}
int atom_table_lookup(struct atom_table *tbl, struct atom_key *from, struct atom_value *to)
{
	return 0;
}

int atom_table_init(struct atom_table *tbl)
{
	return 0;
}
int atom_table_deinit(struct atom_table *tbl)
{
	return 0;
}
