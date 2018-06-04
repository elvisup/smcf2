#ifndef __MSG_H__
#define __MSG_H__

#include <semaphore.h>

#define MAX_MSGQ_PRIORITYS          3	/*high, middle, low */
#define MAX_MSGQUEUE_SIZE           128
#define MAX_MSGQUEUE_BLOCK_NUM      64

#define MSGQ_STATE_FREE	0
#define MSGQ_STATE_USE	1

typedef struct __msgq {
	int msgs;
	int head;
	int tail;
	int pmsg[MAX_MSGQUEUE_SIZE];
} msgq_t;

typedef struct __msgq_block {
	int state;
	int module_id;
	sem_t sem;
	pthread_mutex_t mutex;
	msgq_t msgq[MAX_MSGQ_PRIORITYS];
} msgq_block_t;

typedef struct _msgq_context {
	int qbcount;		/* msgqb count */
	int mem_context;	/* mem mangent for msg */
	msgq_block_t *qblist;	/* msgqb array */
} msgq_context_t;

typedef struct __msg {
	int sender_id;		/* module id of sender */
	int receiver_id;	/* module id of receiver */
	int msgid;		/* msg id of receiver */
	sem_t sem_sync;		/* only used for sync message */
	char data[16];
	int retval;
} msg_t;

int g_msgq_context;

int msgq_init(int qb_num);
int msgq_deinit(int context);
int msgq_bind(int context, int module_id);

int send_msg_async(int sender_id, int receiver_id,
		   int msgpri, int msgid, void *data);
int send_simple_msg_async(int sender_id, int receiver_id, int msgid);
int broadcast_simple_msg_async(int sender_id, int msgid);

msg_t *read_msg(int context, int receiver_id);
void free_msg(int context, msg_t *msg);

#endif /* __MSG_H__ */
