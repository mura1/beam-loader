struct atom_key {
	byte *name;
	byte len;
};

struct atom_value {
	int pos;
};

struct atom_table;


extern int atom_table_insert(struct atom_table *tbl, struct atom_key *from_k, struct atom_value *from_v);
extern int atom_table_lookup(struct atom_table *tbl, struct atom_key *from, struct atom_value *to);
extern int atom_table_init(struct atom_table *tbl);
extern int atom_table_deinit(struct atom_table *tbl);
