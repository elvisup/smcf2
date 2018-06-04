#ifndef __SLIST_H__
#define __SLIST_H__

#ifndef NULL
#define NULL (void*)0
#endif

struct slist_head {
    struct slist_head *next;
};

static inline void slist_init(struct slist_head *head)
{
	head->next = NULL;
}

/**
 * add to the head of list
 **/
static inline void slist_add(struct slist_head *head, struct slist_head *pos)
{
	struct slist_head *next = NULL;

	next = head->next;
	head->next = pos;
	pos->next = next;
}

/**
 * can't delete the head ot the list
 **/
static inline void slist_del(struct slist_head *head, struct slist_head *pos)
{
	struct slist_head *p, *next = NULL;

	next = head;
	do {
		p = next;
		next = p->next;
		if (next == pos) {
			p->next = next->next;
			pos->next = NULL;
			return;
		}
	} while (next);
}

#define slist_offsetof(TYPE, MEMBER) ((unsigned int) &((TYPE *)0)->MEMBER)

#define slist_for_each(pos, head) \
	for (pos = (head)->next; pos != NULL; pos = pos->next)

#define slist_for_each_ext(pos, head, next)			\
	for (next = (head)->next; pos = next, (next ? next = next->next : NULL), pos != NULL;)

#define slist_container_of(ptr, type, member) ({			\
			const typeof( ((type *)0)->member ) *__mptr = (ptr); \
			(type *)( (char *)__mptr - slist_offsetof(type,member) );})

#define slist_entry(ptr, type, member) slist_container_of(ptr, type, member)

#endif /* __SLIST_H__ */
