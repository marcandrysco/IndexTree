#include "idxtree.h"
#include <stdio.h>


/*
 * Node definitions and macros.
 *   @LEFT: The left child index.
 *   @RIGHT: The right child index.
 *   @CMP2NODE: Comparison to child index macro.
 *   @OTHERNODE: Retrieves the opposite child index macro.
 *   @NODEDIR: Child node index to direction macro.
 *   @MYDIR: The direction of the current node from its parent.
 */

#define LEFT	0
#define RIGHT	1
#define CMP2NODE(cmp)	(cmp > 0 ? RIGHT : LEFT)
#define OTHERNODE(node)	(node == RIGHT ? LEFT : RIGHT)
#define NODEDIR(node)	(node == RIGHT ? 1 : -1)
#define MYDIR(node)	(node->parent->child[LEFT] == node ? LEFT : RIGHT)
#define IDXTREE_MAX_HEIGHT	48
#define IDXTREE_NODE_INIT	(struct idxtree_node_t){ 0, { NULL, NULL }, NULL, 0 }


/*
 * local function declarations
 */

static struct idxtree_node_t *rotate_single(struct idxtree_node_t *node, uint8_t dir);
static struct idxtree_node_t *rotate_double(struct idxtree_node_t *node, uint8_t dir);
static void recount(struct idxtree_node_t *node);


/**
 * Create an empty root.
 *   &returns: The root.
 */

struct idxtree_root_t idxtree_init()
{
	return (struct idxtree_root_t){ NULL };
}

/**
 * Look up an AVL index tree node from the root.
 *   @root: The root.
 *   @index: The index.
 *   &returns: The node if found, 'NULL' if not requested index was too large.
 */

struct idxtree_node_t *idxtree_get(struct idxtree_root_t *root, unsigned int index)
{
	int cmp;
	unsigned int cur, left;
	struct idxtree_node_t *node = root->node;

	cur = 0;

	while(node != NULL) {
		left = cur;

		if(node->child[LEFT] != NULL)
			left += node->child[LEFT]->size;

		cmp = (int)index - (int)left;
		if(cmp == 0)
			return node;

		if(CMP2NODE(cmp) == RIGHT)
			cur = left + 1;

		node = node->child[CMP2NODE(cmp)];
	}

	if(cur <= index)
		return NULL;

	fprintf(stderr, "Invalid tree data.\n");
	abort();
}


/**
 * Insert a new node at the given index.
 *   @root: The root.
 *   @index: The destination index.
 *   @node: The node.
 */

void idxtree_insert(struct idxtree_root_t *root, unsigned int index, struct idxtree_node_t *node)
{
	short i, ii;
	unsigned int cur, left;
	uint8_t dir[IDXTREE_MAX_HEIGHT];
	struct idxtree_node_t *stack[IDXTREE_MAX_HEIGHT];

	*node = IDXTREE_NODE_INIT;

	if(root->node == NULL) {
		root->node = node;
		node->parent = NULL;

		return;
	}

	cur = 0;
	stack[0] = root->node;

	for(i = 0; i < IDXTREE_MAX_HEIGHT; i++) {
		if(stack[i] == NULL)
			break;

		left = cur;

		if(stack[i]->child[LEFT] != NULL)
			left += stack[i]->child[LEFT]->size;

		dir[i] = index > left ? RIGHT : LEFT;
		if(dir[i] == RIGHT)
			cur = left + 1;

		stack[i+1] = stack[i]->child[dir[i]];

	}
	if(i == IDXTREE_MAX_HEIGHT)
		fprintf(stderr, "Tree too tall.\n"), abort();

	i--;
	stack[i]->child[dir[i]] = node;
	node->parent = stack[i];

	for(ii = i; ii > -1; ii--)
		stack[ii]->size++;
	
	stack[i]->balance += NODEDIR(dir[i]);

	if(stack[i]->child[OTHERNODE(dir[i])] != NULL)
		return;

	while(i-- > 0) {
		struct idxtree_node_t *node;

		stack[i]->balance += NODEDIR(dir[i]);

		if(stack[i]->balance == 0)
			break;

		if((stack[i]->balance > -2) && (stack[i]->balance < 2))
			continue;

		if(dir[i+1] == CMP2NODE(stack[i]->balance))
			node = rotate_single(stack[i], OTHERNODE(CMP2NODE(stack[i]->balance)));
		else
			node = rotate_double(stack[i], OTHERNODE(CMP2NODE(stack[i]->balance)));

		if(i == 0)
			root->node = node;
		else
			stack[i-1]->child[dir[i-1]] = node;
		
		break;
	}
}

/**
 * Remove a node from the AVL index tree.
 *   @root: The root.
 *   @index: The index of the element to remove.
 *   &returns: The removed node or null.
 */

struct idxtree_node_t *idxtree_root_remove(struct idxtree_root_t *root, unsigned int index)
{
	short i, ii;
	unsigned int cur, left;
	uint8_t dir[IDXTREE_MAX_HEIGHT];
	struct idxtree_node_t *stack[IDXTREE_MAX_HEIGHT], *node, *retval;

	cur = 0;
	stack[0] = root->node;

	for(i = 0; i < IDXTREE_MAX_HEIGHT; i++) {
		if(stack[i] == NULL)
			return NULL;

		left = cur;

		if(stack[i]->child[LEFT] != NULL)
			left += stack[i]->child[LEFT]->size;

		if(index == left)
			break;

		dir[i] = index > left ? RIGHT : LEFT;
		if(dir[i] == RIGHT)
			cur = left + 1;

		stack[i+1] = stack[i]->child[dir[i]];
	}

	for(ii = i - 1; ii != -1; ii--)
		stack[ii]->size--;

	dir[i] = CMP2NODE(stack[i]->balance);

	ii = i;
	node = stack[i]->child[dir[i]];
	if(node != NULL) {
		while(node->child[OTHERNODE(dir[ii])] != NULL) {
			i++;
			stack[i] = node;
			dir[i] = OTHERNODE(dir[ii]);
			node = node->child[dir[i]];
		}

		stack[i]->child[dir[i]] = node->child[dir[ii]];
		if(node->child[dir[ii]] != NULL)
			node->child[dir[ii]]->parent = stack[i];

		i++;

		if(stack[ii]->child[LEFT] != NULL)
			stack[ii]->child[LEFT]->parent = node;

		if(stack[ii]->child[RIGHT] != NULL)
			stack[ii]->child[RIGHT]->parent = node;

		node->child[LEFT] = stack[ii]->child[LEFT];
		node->child[RIGHT] = stack[ii]->child[RIGHT];
		node->balance = stack[ii]->balance;
	}

	if(ii == 0) {
		root->node = node;

		if(node != NULL)
			node->parent = NULL;
	}
	else {
		stack[ii-1]->child[dir[ii-1]] = node;

		if(node != NULL)
			node->parent = stack[ii-1];
	}

	retval = stack[ii];
	stack[ii] = node;

	while(i-- > 0) {
		stack[i]->balance -= NODEDIR(dir[i]);

		if((stack[i]->balance > 1) || (stack[i]->balance < -1)) {
			if(stack[i]->balance == -2 * stack[i]->child[CMP2NODE(stack[i]->balance/2)]->balance)
				node = rotate_double(stack[i], OTHERNODE(CMP2NODE(stack[i]->balance)));
			else
				node = rotate_single(stack[i], OTHERNODE(CMP2NODE(stack[i]->balance)));

			if(i == 0)
				root->node = node;
			else
				stack[i-1]->child[dir[i-1]] = node;

			stack[i] = node;
		}

		if(stack[i]->balance != 0)
			break;
	}

	return retval;
}

/**
 * Given an index, replace the node. The node will not be added if the index
 * does not exist.
 *   @root; The root.
 *   @index: The index.
 *   @node: The node to add.
 *   &returns: The displaced node if found, null ortherwise.
 */

struct idxtree_node_t *idxtree_set(struct idxtree_root_t *root, unsigned int index, struct idxtree_node_t *node)
{
	unsigned int cur, left;
	struct idxtree_node_t *sel = root->node;

	cur = 0;

	while(sel != NULL) {
		left = cur;

		if(sel->child[LEFT] != NULL)
			left += sel->child[LEFT]->size;

		if(index == left)
			break;

		if(index > left)
			cur = left + 1;

		sel = sel->child[index > left ? RIGHT : LEFT];
	}

	if(sel == NULL) {
		if(cur >= index)
			return NULL;

		fprintf(stderr, "Invalid tree data.\n");
		abort();
	}

	if(sel == root->node)
		root->node = node;
	else
		sel->parent->child[MYDIR(sel)] = node;

	*node = *sel;

	return sel;
}


/**
 * Performs a single tree rotation of the given node. The node's child in the
 * opposite direction as the 'dir' paramter will replace itself as the parent,
 * placing the old parent as a child in the direction of the 'dir' parameter.
 * Wikipedia provides a good explanation with pictures.
 *   @node: The AVL index tree node.
 *   @dir: The direction to rotate, should be either the value 'LEFT' or
 *     'RIGHT'.
 *   &returns: The node that now takes the place of the node that was passed
 *     in.
 */

static struct idxtree_node_t *rotate_single(struct idxtree_node_t *node, uint8_t dir)
{
	struct idxtree_node_t *tmp;

	tmp = node->child[OTHERNODE(dir)];
	node->child[OTHERNODE(dir)] = tmp->child[dir];
	tmp->child[dir] = node;

	node->balance += NODEDIR(dir);
	if(NODEDIR(dir) * tmp->balance < 0)
		node->balance -= tmp->balance;

	tmp->balance += NODEDIR(dir);
	if(NODEDIR(dir) * node->balance > 0)
		tmp->balance += node->balance;

	tmp->parent = node->parent;
	node->parent = tmp;

	if(node->child[OTHERNODE(dir)] != NULL)
		node->child[OTHERNODE(dir)]->parent = node;

	recount(node);
	recount(tmp);

	return tmp;
}

/**
 * Performs a double rotation that is used to bring the grandchild to replace
 * its current position. Wikipedia provides a good explanation with pictures.
 *   @node: The AVL index tree node.
 *   @dir: The direction to rotate, should be either the value 'LEFT' or
 *     'RIGHT'.
 *   &returns: The node that now takes the place of the node that was passed
 *     in.
 */

static struct idxtree_node_t *rotate_double(struct idxtree_node_t *node, uint8_t dir)
{
	node->child[OTHERNODE(dir)] = rotate_single(node->child[OTHERNODE(dir)], OTHERNODE(dir));

	return rotate_single(node, dir);
}

/**
 * Recomputes the count for a given node based off of its child elements.
 *   @node: The node.
 *   &prop: noerror
 */

static void recount(struct idxtree_node_t *node)
{
	node->size = 1;

	if(node->child[LEFT] != NULL)
		node->size += node->child[LEFT]->size;

	if(node->child[RIGHT] != NULL)
		node->size += node->child[RIGHT]->size;
}
