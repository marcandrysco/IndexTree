#ifndef IDXTREE_H
#define IDXTREE_H

/*
 * common headers
 */

#include <stdint.h>
#include <stdlib.h>


/**
 * AVL index tree node storage.
 *   @balance: The current balance of the node, between '-2' to '2'.
 *   @child, parent: The child and parent nodes.
 *   @size: The size of the subtree
 */

struct idxtree_node_t {
	int8_t balance;
	struct idxtree_node_t *child[2], *parent;
	unsigned int size;
};

/**
 * AVL index tree root structure.
 *   @node: The root node.
 */

struct idxtree_root_t {
	struct idxtree_node_t *node;
};


/*
 * index tree function declarations
 */

struct idxtree_root_t idxtree_init();
struct idxtree_node_t *idxtree_get(struct idxtree_root_t *root, unsigned int index);
void idxtree_insert(struct idxtree_root_t *root, unsigned int index, struct idxtree_node_t *node);
struct idxtree_node_t *idxtree_root_remove(struct idxtree_root_t *root, unsigned int index);
struct idxtree_node_t *idxtree_set(struct idxtree_root_t *root, unsigned int index, struct idxtree_node_t *node);

#endif
