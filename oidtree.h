#ifndef OIDTREE_H
#define OIDTREE_H

#include "cbtree.h"
#include "hash.h"

struct alloc_state;
struct oidtree {
	struct cb_tree t;
	struct alloc_state *mempool;
};

#define OIDTREE_INIT { .t = CBTREE_INIT, .mempool = NULL }

static inline void oidtree_init(struct oidtree *ot)
{
	cb_init(&ot->t);
	ot->mempool = NULL;
}

void oidtree_destroy(struct oidtree *);
void oidtree_insert(struct oidtree *, const struct object_id *);
int oidtree_contains(struct oidtree *, const struct object_id *);

typedef enum cb_next (*oidtree_iter)(const struct object_id *, void *arg);
void oidtree_each(struct oidtree *, const struct object_id *,
			size_t oidhexlen, oidtree_iter, void *arg);

#endif /* OIDTREE_H */
