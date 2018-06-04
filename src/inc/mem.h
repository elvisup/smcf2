#ifndef __MEM_H__
#define __MEM_H__

/**
 * alloc memory and add to use list
 **/
void *mem_alloc(int context);

/**
 * move mem_unit to free list
 */
void mem_free(int context, void *p);

/**
 * unit_size: size of one unit
 * return context
 **/
int mem_init(int unit_size);

/**
 * free all memory alloced
 **/
void mem_deinit(int context);

#endif /* __MEM_H__ */
