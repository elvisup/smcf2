#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mem.h>
#include <slist.h>

#define MIN_UNIT_SIZE	0
#define MAX_UNIT_SIZE	32768

typedef struct __mem_unit {
	struct slist_head head;
	void *p;
} mem_unit;

typedef struct __mem_context {
	int unit_size;
	int use_count;
	struct slist_head use;
	int free_count;
	struct slist_head free;
} mem_context;

/**
 * alloc memory and add to use list
 **/
void *mem_alloc(int context)
{
	mem_context *context_t = (mem_context *)context;
	mem_unit *unit = NULL;
	struct slist_head *head = NULL;

	if (context_t->free_count) {
		head = context_t->free.next;
		slist_del(&context_t->free, head);
		context_t->free_count--;
		unit = slist_container_of(head, mem_unit, head);
	} else {
		//TODO: do not need size
		unit = malloc(sizeof(mem_unit) + context_t->unit_size);
		unit->p = (void*)unit + sizeof(mem_unit);
		//TODO: malloc 4KB space per time. if not enthough, malloc another 4KB.
	}

	slist_add(&context_t->use, &unit->head);
	context_t->use_count++;

	return unit->p;
}

/**
 * move mem_unit to free list
 */
void mem_free(int context, void *p)
{
	mem_context *context_t = (mem_context *)context;
	mem_unit *unit = NULL;
	struct slist_head *head, *pos;

	head = &context_t->use;
	slist_for_each(pos, head) {
		unit = slist_container_of(pos, mem_unit, head);
		if (unit->p == p) {
			slist_del(&context_t->use, pos);
			context_t->use_count--;
			slist_add(&context_t->free, pos);
			context_t->free_count++;
			return;
		}
	}

	printf("%s:%d -> this msg do not in msg_list!\n", __func__, __LINE__);
	return;
}

/**
 * unit_size: size of one unit
 * return context
 **/
int mem_init(int unit_size)
{
	mem_context *context = NULL;

	if (unit_size <= MIN_UNIT_SIZE || unit_size >= MAX_UNIT_SIZE) {
		printf("ERROR: %s[%d] -> unit_size = %d, error!\n", __func__, __LINE__, unit_size);
		return -1;
	}

	context = malloc(sizeof(mem_context));
	if (!context) {
		printf("ERROR: %s[%d] -> mem_context malloc error!\n", __func__, __LINE__);
		return -1;
	}

	context->unit_size = unit_size;
	context->use_count = 0;
	context->free_count = 0;
	slist_init(&context->use);
	slist_init(&context->free);

	return (int)context;
}

void mem_deinit(int context)
{
	mem_context *context_t = (mem_context *)context;
	struct slist_head *head, *pos, *next;
	mem_unit *unit;

	if (!context_t) {
		printf("ERROR:");
		return;
	}

	/* free free list */
	head = &context_t->free;
	slist_for_each_ext(pos, head, next) {
		unit = slist_container_of(pos, mem_unit, head);
		free(unit);
	}

	/* free use list */
	head = &context_t->use;
	slist_for_each_ext(pos, head, next) {
		unit = slist_container_of(pos, mem_unit, head);
		free(unit);
	}

	return;
}
