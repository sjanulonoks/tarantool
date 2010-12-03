/*
 * Copyright (C) 2010 Mail.RU
 * Copyright (C) 2010 Yuriy Vostrikov
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef TARANTOOL_SILVERBOX_INDEX_H
#define TARANTOOL_SILVERBOX_INDEX_H

#include <mod/silverbox/assoc.h>

struct field {
	u32 len;
	union {
		u32 u32;

		u8 data[sizeof(void *)];

		void *data_ptr;
	};
};

enum field_data_type { NUM, NUM64, STR };

struct tree_index_member {
	struct box_tuple *tuple;
	struct field key[];
};

#define SIZEOF_TREE_INDEX_MEMBER(index) \
	(sizeof(struct tree_index_member) + sizeof(struct field) * (index)->key_cardinality)

#include <third_party/sptree.h>
SPTREE_DEF(str_t, realloc);

struct index {
	bool enabled;

	bool unique;

	struct box_tuple *(*find) (struct index * index, void *key);	/* only for unique lookups */
	struct box_tuple *(*find_by_tuple) (struct index * index, struct box_tuple * pattern);
	void (*remove) (struct index * index, struct box_tuple *);
	void (*replace) (struct index * index, struct box_tuple *, struct box_tuple *);
	void (*iterator_init) (struct index *, struct tree_index_member * pattern);
	struct box_tuple *(*iterator_next) (struct index *, struct tree_index_member * pattern);
	union {
		khash_t(lstr_ptr_map) * str_hash;
		khash_t(int_ptr_map) * int_hash;
		khash_t(int64_ptr_map) * int64_hash;
		khash_t(int_ptr_map) * hash;
		sptree_str_t *tree;
	} idx;
	void *iterator;
	bool iterator_empty;

	struct namespace *namespace;

	struct {
		struct {
			u32 fieldno;
			enum field_data_type type;
		} *key_field;
		u32 key_cardinality;

		u32 *field_cmp_order;
		u32 field_cmp_order_cnt;
	};

	struct tree_index_member *search_pattern;

	enum { HASH, TREE } type;
};

#define foreach_index(n, index_var)					\
	for (struct index *index_var = namespace[(n)].index;		\
	     index_var->key_cardinality != 0;				\
	     index_var++)						\
		if (index_var->enabled)

void index_hash_num(struct index *index, struct namespace *namespace, size_t estimated_rows);
void index_hash_num64(struct index *index, struct namespace *namespace, size_t estimated_rows);
void index_hash_str(struct index *index, struct namespace *namespace, size_t estimated_rows);
void index_tree(struct index *index, struct namespace *namespace, size_t estimated_rows __unused__);

struct tree_index_member * alloc_search_pattern(struct index *index, int key_cardinality, void *key);
void index_iterator_init_tree_str(struct index *self, struct tree_index_member *pattern);
struct box_tuple * index_iterator_next_tree_str(struct index *self, struct tree_index_member *pattern);

struct box_txn;
void validate_indeces(struct box_txn *txn);
void build_indexes(void);

#endif
