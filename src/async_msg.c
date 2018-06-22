#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include <core.h>
#include <msg.h>
#include <mem.h>
#include <smcf2.h>
#include <module_manager.h>

/**
 * return context
 **/
int msgq_init(int qb_num)
{
	int size;
	int mem_context;
	msgq_context_t *context;

	if (qb_num <= 0 || qb_num > MAX_MSGQUEUE_SIZE) {
		slog(LOG_ERR, "%s:%d -> qb_num = %d, error!\n", __func__, __LINE__, qb_num);
		return 0;
	}
	size = sizeof(msgq_context_t) + sizeof(msgq_block_t) * qb_num;

	context = (msgq_context_t *)malloc(size);
	if (!context) {
		slog(LOG_ERR, "%s:%d -> msgq_context_t malloc error(%s)!\n", __func__, __LINE__, strerror(errno));
		return 0;
	}

	mem_context = mem_init(sizeof(msg_t));
	if (mem_context == -1) {
		slog(LOG_ERR, "%s:%d -> mem_init error!\n", __func__, __LINE__);
		free(context);
		return 0;
	}

	memset(context, 0, size);
	context->qblist = (msgq_block_t *)((void *)context + sizeof(msgq_context_t));
	context->mem_context = mem_context;
	context->qbcount = qb_num;

	g_msgq_context = (int)context;

	return g_msgq_context;
}

int msgq_deinit(int context)
{
	msgq_context_t *context_t = (msgq_context_t *)context;

	if (!context_t) {
		printf("ERROR:\n");
		return -1;
	}

	free(context_t);

	return 0;
}

/**
 * get free msgqueue id
 **/
static int get_free_msgqb_id(msgq_context_t *context_t)
{
	int qbid;
	int qbcount = context_t->qbcount;
	msgq_block_t *qblist = context_t->qblist;

	for (qbid = 0; qbid < qbcount; qbid++) {
		if (qblist[qbid].state == MSGQ_STATE_FREE)
			return qbid;
	}

	return -1;
}

/**
 * bind module and msgqb
 **/
int msgq_bind(int context, int module_id)
{
	int free_msgqb_id;
	msgq_block_t *msgqb;
	msgq_context_t *context_t = (msgq_context_t *)context;

	free_msgqb_id = get_free_msgqb_id(context_t);
	if (free_msgqb_id == -1) {
		slog(LOG_ERR, "%s:%d -> get_free_msgqb_id!\n", __func__, __LINE__);
		return -1;
	}

	msgqb = &context_t->qblist[free_msgqb_id];
	memset(msgqb, 0, sizeof(msgq_block_t));
	msgqb->state = MSGQ_STATE_USE;

	msgqb->module_id = module_id;
	sem_init(&msgqb->sem, 0, 0);
	pthread_mutex_init(&msgqb->mutex , NULL);

	return free_msgqb_id;
}

msg_t *alloc_msg(int context)
{
	msgq_context_t *context_t = (msgq_context_t *)context;
	msg_t *msg;

	msg = (msg_t *)mem_alloc(context_t->mem_context);
	if (!msg) {
		printf("%s:%d -> mem_alloc msg error!\n", __func__, __LINE__);
		return NULL;
	}

	return msg;
}

void free_msg(int context, msg_t *msg)
{
	msgq_context_t *context_t = (msgq_context_t *)context;

	mem_free(context_t->mem_context, msg);
}

static int get_msgqb_by_module_id(msgq_context_t *context_t, int module_id)
{
	int qbid;
	int qbcount = context_t->qbcount;
	msgq_block_t *qblist = context_t->qblist;

	for (qbid = 0; qbid < qbcount; qbid++) {
		if (qblist[qbid].module_id == module_id) {
			/*
			 *printf("%s:%d -> qbid = %d, fmodule_id = %d, omodule_id = %d\n", \
			 *        __func__, __LINE__, qbid, qblist[qbid].module_id, module_id);
			 */
			return qbid;
		}
	}

	return -1;
}

int write_msg(int context, msg_t *msg, int priority)
{
	int msgqb_id;
	int head, tail, msgs;
	msgq_t *msgq;
	msgq_block_t *msgqb;
	int module_id = msg->receiver_id;
	msgq_context_t *context_t = (msgq_context_t *)context;

	msgqb_id = get_msgqb_by_module_id(context_t, module_id);
	if (msgqb_id == -1) {
		printf("%s:%d -> ERROR: module_id = %d\n", __func__, __LINE__, module_id);
		return -1;
	}

	msgqb = &context_t->qblist[msgqb_id];
	msgq = &msgqb->msgq[priority];
	/*printf("%s:%d -> get_msgqb_by_module_id [%d] - %d, data:[%s]\n", __func__, __LINE__, msgqb_id, msgqb->module_id, msg->data);*/

	/* lock */
	pthread_mutex_lock(&msgqb->mutex);
	msgs = msgq->msgs;
	if (msgs == MAX_MSGQUEUE_SIZE) {
		printf("WARNING: drop old message of moudle [%d]\n", module_id);
		head = msgq->head;
		free_msg(context, (msg_t *)msgq->pmsg[head]);
		msgq->head = (++head == MAX_MSGQUEUE_SIZE) ? 0 : head;
		msgs--;
	}
	tail = msgq->tail;
	msgq->pmsg[tail] = (int)msg;
	msgq->tail = (++tail == MAX_MSGQUEUE_SIZE) ? 0 : tail;
	msgs++;
	msgq->msgs = msgs;
	/* unlock */
	pthread_mutex_unlock(&msgqb->mutex);

	sem_post(&msgqb->sem);

	return 0;
}

msg_t *read_msg(int context, int receiver_id)
{
	int head;
	int msgqb_id;
	int priority;
	msg_t *msg = NULL;
	msgq_t *msgq;
	msgq_block_t *msgqb;
	int module_id = receiver_id;
	msgq_context_t *context_t = (msgq_context_t *)context;

	msgqb_id = get_msgqb_by_module_id(context_t, module_id);
	if (msgqb_id == -1) {
		printf("ERROR:");
		return NULL;
	}

	msgqb = &context_t->qblist[msgqb_id];
	/*printf("%s:%d -> get_msgqb_by_module_id [%d] - %d\n", __func__, __LINE__, msgqb_id, msgqb->module_id);*/

	sem_wait(&msgqb->sem);

	/* lock */
	pthread_mutex_lock(&msgqb->mutex);
	for (priority = MSG_PRI_HIGH; priority < MAX_MSGQ_PRIORITYS; priority++) {
		msgq = &msgqb->msgq[priority];
		if (msgq->msgs) {
			head = msgq->head;
			msg = (msg_t *)msgq->pmsg[head];
			msgq->head = (++head == MAX_MSGQUEUE_SIZE) ? 0 : head;
			msgq->msgs--;
			break;
		}
	}
	/* unlock */
	pthread_mutex_unlock(&msgqb->mutex);

	return msg;
}

/**
 * smcf2_recv_msg:
 *
 * called by sender, send msg from sender to receiver
 *
 * return 0 if success.
 **/
int smcf2_recv_msg(int *sender_id, int receiver_id, int *msgid, char data[16])
{
	msg_t *msg;

	module_t *receiver_module = NULL;
	receiver_module = search_module_from_mblock_list(receiver_id);
	if (!receiver_module) {
		printf("ERROR\n");
		return -1;
	}

	if (receiver_module->auto_msg_process_en == 1) {
		printf("ERROR: [%s] module do not support recv msg manual!\n", receiver_module->name);
		return -1;
	}

	msg = read_msg(g_msgq_context, receiver_id);
	if (!msg) {
		printf("ERROR\n");
		return -1;
	}

	*sender_id = msg->sender_id;
	*msgid = msg->msgid;
	memcpy(data, msg->data, 16);

	return 0;
}

/**
 * smcf2_send_msg:
 *
 * called by sender, send msg from sender to receiver
 *
 * return 0 if success.
 **/
int smcf2_send_msg(int sender_id, int receiver_id,
	     int msgpri, int msgid, char data[16])
{
	msg_t *msg;

	msg = alloc_msg(g_msgq_context);
	if (!msg) {
		printf("%s:%d -> ERROR: \n", __func__, __LINE__);
		return -1;
	}

	msg->sender_id = sender_id;
	msg->receiver_id = receiver_id;
	msg->msgid = msgid;
	//TODO: memcpy 16 byte
	memcpy((void *)(msg->data), (void *)data, 16);

	return write_msg(g_msgq_context, msg, msgpri);
}

/**
 * send_simple_msg:
 *
 * called by sender, send simple msg from sender to receiver
 *
 * return 0 if success.
 **/
int send_simple_msg(int sender_id, int receiver_id, int msgid)
{
	return smcf2_send_msg(sender_id, receiver_id, MSG_PRI_LOW, msgid, NULL);
}

/**
 * broadcast_simple_msg_async:
 *
 * called by sender, broadcast msg from sender to all receiver
 *
 * return 0 if success.
 **/
int broadcast_simple_msg(int sender_id, int msgid)
{
	int i, ret;
	int receiver_id;
	msgq_context_t *context_t = (msgq_context_t *)g_msgq_context;
	int qbcount = context_t->qbcount;
	msgq_block_t *qblist = context_t->qblist;
	msgq_block_t *msgqb;

	for (i = 0; i < qbcount; i++) {
		msgqb = &qblist[i];
		if (msgqb->state == MSGQ_STATE_USE) {
			receiver_id = msgqb->module_id;
			ret = send_simple_msg(sender_id, receiver_id, msgid);
			if (ret) {
				printf("ERROR: \n");
			}
		}
	}

	return 0;
}
