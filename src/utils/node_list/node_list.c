/* node_list.c
 *
 * node list manager;
 *
 * Copyright (c) 2018 Ingenic
 * Author:Elvis <huan.wang@ingenic.com>
 *
 * Version v1.0: node list init version
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#include <utils/node_list.h>
#include <utils/slog.h>

typedef struct node_list {
	/* free list */
	struct list_head free;
	unsigned int free_cnt;	/* for futher use */
	sem_t free_sem;
	pthread_mutex_t free_mutex;

	/* use list */
	struct list_head use;
	unsigned int use_cnt;	/* for futher use */
	sem_t use_sem;
	pthread_mutex_t use_mutex;

	/* other */
	int keep_update;
	volatile int block;
	void *node_mem;
} node_list_t;

node_t *Get_Use_Node(int context);

/**********************************
 *  rx_node_list
 *********************************/
node_t *Get_Free_Node(int context)
{
	if (context) {
		node_list_t *list = (node_list_t *)context;
		node_t *node = NULL;
		struct list_head *head = &list->free;

		if (list_empty(head) && list->keep_update) {
			node = Get_Use_Node(context);
			if (!node) {
				printf("%s: FATAL ERROR: lost nodes\n", __func__);
				return NULL;
			}
			return node;
		} else
			sem_wait(&list->free_sem);

		pthread_mutex_lock(&list->free_mutex);
		{
			/*printf("c: %p, p: %p, n:%p\n", head->next, head->next->prev, head->next->next);*/
			node = list_entry(head->next, node_t, head);
			list_del(head->next);
			list->free_cnt--;
		}
		pthread_mutex_unlock(&list->free_mutex);

		return node;
	}
	slog(LOG_DBG, "%s:%d context is NULL\n", __func__, __LINE__);

	return NULL;
}

void Put_Use_Node(int context, node_t *node)
{
	if (context) {
		node_list_t *list = (node_list_t *)context;
		struct list_head *head = &list->use;

		if (node == NULL) {
			printf("%s: ERROR node can't be NULL\n", __func__);
			return;
		}

		pthread_mutex_lock(&list->use_mutex);
		{
			list_add_tail(&node->head, head);
			list->use_cnt++;
			sem_post(&list->use_sem);
		}
		pthread_mutex_unlock(&list->use_mutex);
	}
}

/**
 * blocked get rx use node
 **/
node_t *Get_Use_Node(int context)
{
	if (context) {
		node_list_t *list = (node_list_t *)context;
		node_t *node = NULL;
		struct list_head *head = &list->use;

		if (list_empty(head) && !list->block)
			return NULL;
		else {
			sem_wait(&list->use_sem);
			if (list_empty(head)) {
				return NULL;
			}
		}

		pthread_mutex_lock(&list->use_mutex);
		{
			node = list_entry(head->next, node_t, head);
			list_del(head->next);
			list->use_cnt--;
		}
		pthread_mutex_unlock(&list->use_mutex);

		return node;
	}

	return NULL;
}

void Put_Free_Node(int context, node_t *node)
{
	if (context) {
		node_list_t *list = (node_list_t *)context;
		struct list_head *head = &list->free;

		if (node == NULL) {
			printf("%s: ERROR node can't be NULL\n", __func__);
			return;
		}

		pthread_mutex_lock(&list->free_mutex);
		{
			list_add_tail(&node->head, head);
			list->free_cnt++;
			sem_post(&list->free_sem);
		}
		pthread_mutex_unlock(&list->free_mutex);
	}
}

void Drop_Use_Node(int context)
{
	if (context) {
		node_list_t *list = (node_list_t *)context;
		struct list_head *head = &list->use;
		node_t *node = NULL;

		while (!list_empty(head)) {
			pthread_mutex_lock(&list->use_mutex);
			{
				node = list_entry(head->next, node_t, head);
				list_del(head->next);
				list->use_cnt--;
			}
			pthread_mutex_unlock(&list->use_mutex);

			Put_Free_Node(context, node);
		}
	}
}

/**
 * node ops
 **/
void Set_Node_Private(node_t *node, int private)
{
	node->_private = private;
}

/**
 * node_size: node buf size
 * nodes: number of nodes
 * keep_update: keep update the use list
 **/
int Node_List_Init(unsigned int node_size, unsigned int nodes,
			 int keep_update, int block, int type)
{
	int i;
	node_list_t *list = NULL;
	void *node_mem = NULL;

	/* check params */
	if ((node_size < 0) || (nodes < 2)) {
		printf("%s: ERROR: argument error!, nodesize : %d, nodes: %d\n", __func__, node_size, nodes);
		goto error;
	}

	if (type == NODE_DATA_TYPE_POINTER)
		node_size = 0;

	/*slog(LOG_DBG, "%s:%d -> node size: %d\n", __func__, __LINE__, node_size);*/

	/* alloc context */
	list = malloc(sizeof(node_list_t));
	if (!list) {
		printf("%s: ERROR: alloc context error!\n", __func__);
		goto error;
	}
	memset(list, 0x0, sizeof(node_list_t));

	/* init list head */
	INIT_LIST_HEAD(&(list->free));
	INIT_LIST_HEAD(&(list->use));

	/* init sem and mutex */
	sem_init(&list->free_sem, 0, 0);
	sem_init(&list->use_sem, 0, 0);
	pthread_mutex_init(&list->free_mutex, NULL);
	pthread_mutex_init(&list->use_mutex, NULL);

	/* alloc node and data */
	node_mem = malloc(nodes * (sizeof(node_t) + node_size));
	if (!node_mem) {
		printf("%s: ERROR: alloc memory for node error!\n", __func__);
		goto error1;
	}
	memset(node_mem, 0x0, nodes * (sizeof(node_t) + node_size));

	for (i = 0; i < nodes; i++) {
		node_t *node = (node_t *)((char *)node_mem + i * (sizeof(node_t) + node_size));

		if (type == NODE_DATA_TYPE_POINTER)
			node->data = NULL;
		else
			node->data = (char *)node + sizeof(node_t);

		node->context = (int)list;
		node->_private = 0;
		/*slog(LOG_DBG, "%s:%d -> node addr: %p, data addr: %p\n", __func__, __LINE__, node, node->data);*/
		/*slog(LOG_DBG, "\t%s:%d -> node addr: %p\n", __func__, __LINE__, node);*/
		Put_Free_Node((int)list, node);
	}

	/* store node_mem and data_mem */
	list->node_mem = node_mem;
	list->keep_update = keep_update;
	list->block = block;

	return (int)list;

error1:
	free(list);
error:
	return 0;
}

void Node_List_Set_Block(int context, int block)
{
	if (context) {
		node_list_t *list = (node_list_t *)context;
		if (list->block && !block) {
			int sem_value;
			sem_getvalue(&list->use_sem, &sem_value);
			if (sem_value == 0) {
				sem_post(&list->use_sem);
			}
			list->block = 0;
		} else if (!list->block && block) {
			list->block = 1;
		}
	}
}

int Node_List_Deinit(int context)
{
	if (context) {
		node_list_t *list = (node_list_t *)context;
		free(list->node_mem);
		free(list);
	}

	return 0;
}
