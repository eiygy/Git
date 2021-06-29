/*
 * A wrapper around cbtree which stores oids
 * May be used to replace oid-array for prefix (abbreviation) matches
 */
#include "oidtree.h"
#include "alloc.h"
#include "hash.h"

struct oidtree_node {
	/* n.k[] is used to store "struct object_id" */
	struct cb_node n;
};

struct oidtree_iter_data {
	oidtree_iter fn;
	void *arg;
	size_t *last_nibble_at;
	int algo;
	uint8_t last_byte;
};

void oidtree_destroy(struct oidtree *ot)
{
	if (ot->mempool) {
		clear_alloc_state(ot->mempool);
		FREE_AND_NULL(ot->mempool);
	}
	oidtree_init(ot);
}

void oidtree_insert(struct oidtree *ot, const struct object_id *oid)
{
	struct oidtree_node *on;

	if (!ot->mempool)
		ot->mempool = allocate_alloc_state();
	if (!oid->algo)
		BUG("oidtree_insert requires oid->algo");

	on = alloc_from_state(ot->mempool, sizeof(*on) + sizeof(*oid));
	oidcpy_with_padding((struct object_id *)on->n.k, oid);

	/*
	 * n.b. we shouldn't get duplicates, here, but we'll have
	 * a small leak that won't be freed until oidtree_destroy
	 */
	cb_insert(&ot->t, &on->n, sizeof(*oid));
}

int oidtree_contains(struct oidtree *ot, const struct object_id *oid)
{
	struct object_id k = { 0 };
	size_t klen = sizeof(k);
	oidcpy_with_padding(&k, oid);

	if (oid->algo == GIT_HASH_UNKNOWN) {
		k.algo = hash_algo_by_ptr(the_hash_algo);
		klen -= sizeof(oid->algo);
	}

	return cb_lookup(&ot->t, (const uint8_t *)&k, klen) ? 1 : 0;
}

static enum cb_next iter(struct cb_node *n, void *arg)
{
	struct oidtree_iter_data *x = arg;
	const struct object_id *oid = (const struct object_id *)n->k;

	if (x->algo != GIT_HASH_UNKNOWN && x->algo != oid->algo)
		return CB_CONTINUE;

	if (x->last_nibble_at) {
		if ((oid->hash[*x->last_nibble_at] ^ x->last_byte) & 0xf0)
			return CB_CONTINUE;
	}

	return x->fn(oid, x->arg);
}

void oidtree_each(struct oidtree *ot, const struct object_id *oid,
			size_t oidhexlen, oidtree_iter fn, void *arg)
{
	size_t klen = oidhexlen / 2;
	struct oidtree_iter_data x = { 0 };

	x.fn = fn;
	x.arg = arg;
	x.algo = oid->algo;
	if (oidhexlen & 1) {
		x.last_byte = oid->hash[klen];
		x.last_nibble_at = &klen;
	}
	cb_each(&ot->t, (const uint8_t *)oid, klen, iter, &x);
}
