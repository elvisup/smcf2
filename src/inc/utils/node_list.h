#ifndef __NODE_LIST_H__
#define __NODE_LIST_H__

#include "list.h"

#define NODE_DATA_TYPE_POINTER	0
#define NODE_DATA_TYPE_BUFFER	1

#define NODE_NOT_UPDATE         0
#define NODE_KEEP_UPDATE        1

#define NODE_NOBLOCK            0
#define NODE_BLOCK              1

typedef struct node {
	struct list_head head;
	void *data;			/* need alloc memory */
	int context;			/* context of node belongs */
	int _private;			/* private data */
} node_t;

/**
 * used for node producer
 **/
node_t *Get_Free_Node(int context);
void Put_Use_Node(int context, node_t *node);

/**
 * used for node consumer
 **/
node_t *Get_Use_Node(int context);
void Put_Free_Node(int context, node_t *node);
void Drop_Use_Node(int context);

/**
 * node ops
 **/
void Set_Node_Private(node_t *node, int private);

/**
 * Init
 * node_size: node buf size;
 * nodes: number of nodes, must >= 2;
 * keep_update: keep update the use list, 1: keep update use list, 0: keep old data;
 * block: if use list empty, get use node return NULL.
 * Return: context, 0 is fail
 **/
int Node_List_Init(unsigned int node_size, unsigned int nodes,
			 int keep_update, int block, int type);

/**
 * Deinit
 * Return: 0 if success
 **/
int Node_List_Deinit(int context);

#endif /* __NODE_LIST_H__ */
