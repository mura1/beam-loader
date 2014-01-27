#include <string.h>

#include <beam/types.h>
#include <beam/errno.h>

struct atom_key {
	byte *name;
	byte len;
};

struct atom_value {
	int pos;
};

struct atom {
	struct atom_key key;
	struct atom_value value;
};

struct atom_table {
	struct slice slots;
};






int atom_table_insert(struct atom_table *tbl, struct atom_key *from_k, struct atom_value *from_v)
{
	struct atom *to_slot;

	if (tbl->slots.len >= tbl->slots.cap)
		return -EATOMTABLE_FULL;

	to_slot = &((struct atom *)tbl->slots.addr)[tbl->slots.len]; 
	memcpy(&to_slot->key, from_k, sizeof(to_slot->key));
	memcpy(&to_slot->value, from_v, sizeof(to_slot->value));

	return 0;
}
int atom_table_lookup(struct atom_table *tbl, struct atom_key *from, struct atom_value *to)
{
	int i;
	for (i = 0; i < tbl->slots.len; i++) {
		struct atom *slot = &((struct atom *)tbl->slots.addr)[i];
		if (slot->key.len == from->len && memcmp(slot->key.name, from->name, from->len) == 0) {
			to->pos = slot->value.pos;
			return 0;
		}
	}
	return -EATOMTABLE_NO_ATOM;
}

int atom_table_init(struct atom_table *tbl)
{
	static struct atom __buf[1024];

	tbl->slots.addr = __buf;
	tbl->slots.cap = sizeof(__buf)/sizeof(__buf[0]);
	tbl->slots.len = 0;

	return 0;
}
int atom_table_deinit(struct atom_table *tbl)
{
	tbl->slots.addr = NULL;
	tbl->slots.cap = 0;
	tbl->slots.len = 0;

	return 0;
}
