/**
 * smcf.h
 **/
#ifndef __SMCF_H__
#define __SMCF_H__

/*******************************************************************************\
 * async msg                                                                   *
\*******************************************************************************/

/**
 * async msg priority
 **/
#define MSG_PRI_HIGH          0
#define MSG_PRI_MIDDLE        1
#define MSG_PRI_LOW           2

/**
 * func: send_msg
 * input:
 *   sender_id: the id of modules send msg
 *   receiver_id: the id of modules receiver msg
 *   msgpri: priority of this msg
 *   msgid: msg correspondence detial method in recriver module
 *   data: msg carries 16byte data to receiver module
 *
 * note:
 *   data: send module can carries 16byte data to receiver module,
 *         if we needn't carry data to receiver module, please set
 *         data == NULL.
 **/
int send_msg(int sender_id, int receiver_id, int msgpri, int msgid, char data[16]);

/**
 * func: recv_msg
 * input:
 *   sender_id: the id of modules that this msg send from
 *   receiver_id: the id of modules receiver msg
 *   msgid: msg correspondence detial method in recriver module
 *   data: msg carries 16byte data to receiver module
 *
 * note:
 *   data: send module can carries 16byte data to receiver module,
 *         if we needn't carry data to receiver module, please set
 *         data == NULL.
 *
 *   if auto_msg_process == 1, recv_msg invalied, modules receive
 *   msg by smcf auto manager, you can not recv_msg manually, otherwise
 *   this api will return -1.
 **/
int recv_msg(int *sender_id, int receiver_id, int *msgid, char data[16]);

/*******************************************************************************\
 * sync msg                                                                    *
\*******************************************************************************/

/**
 * func: send_msg_sync
 * input:
 *   sender_id: the id of modules send msg
 *   receiver_id: the id of modules receiver msg
 *   msgid: msg correspondence detial method in recriver module
 *   data: msg carries 16byte data to receiver module
 *
 * note:
 *   this API used to send sync msg, so you should ensure receive
 *   module support sync msg. if receive module do not support sync
 *   msg, this API wii return error.
 **/
int send_msg_sync(int sender_id, int receiver_id, int msgid, char data[16]);

/*******************************************************************************\
 * data                                                                        *
\*******************************************************************************/

/**
 * data channel type
 **/
#define DATA_CHANNEL_SENDER    0
#define DATA_CHANNEL_RECEIVER  1

/**
 * data_channel_t
 * this struct used to config data channel attr
 **/
typedef struct __data_channel {
	int enable;		/* this channel enable or not, if not 1 indecate disable */
	int role; 		/* sender or receiver */
	int size; 		/* data channel node size */
	int num; 		/* node number of data channel */
	int module_id;		/* if module is sender set -1, if is receiver set sender id */
	int channel;		/* if module is sender set -1, if is receiver set sender channel id */
} data_channel_t;

/* reserved for special use */
typedef struct __data_channel_hook {
	int enable;		/* this channel enable or not, if not 1 indecate disable */
	int role; 		/* sender or receiver */
	int num; 		/* node number of data channel */
	int module_id;		/* if module is sender set -1, if is receiver set sender id */
	int channel;		/* if module is sender set -1, if is receiver set sender channel id */
} data_channel_hook_t;

typedef struct node node_t;

/**
 * get_normal_chn_data
 * input:
 *   current_module_id: get data module's id
 *   channel_id: get data module channel's id
 *
 * return:
 *   when this channel is sender, will return a free data node,
 *   otherwise this channel is receiver, will return a use data node
 * ...
 **/
void *get_normal_chn_data(int current_module_id, int channel_id);
void *get_hook_chn_data(int current_module_id, int channel_id);

/**
 * put_normal_chn_data
 * input:
 *   current_module_id: get data module's id
 *   channel_id: get data module channel's id
 *   data: data buffer
 *
 * note:
 *   when this channel is sender, will put data to use node list,
 *   otherwise this channel is receiver, will put data to free node list.
 * ...
 **/
int put_normal_chn_data(int current_module_id, int channel_id, void *data);
int put_hook_chn_data(int current_module_id, int channel_id, void *data);

/*******************************************************************************\
 * smcf                                                                        *
\*******************************************************************************/

typedef struct __module {
	int id;					/* module id */
	char *name;				/* module name */
	/* message process handler, the newer message can't be processed before
	 * current process finished, so don't blocked in this function */
	int auto_msg_process_en;
	int (*msg_process)(int sender_id, int msgid, char data[16]);
 	/* if sync_msg_process_en == 0, call this func report error */
	int sync_msg_process_en;
	int (*sync_msg_process)(int sender_id, int msgid, char data[16]);

	/* module data channel */
	data_channel_t data_channel[4];
	/* reserved module data hook channel */
	data_channel_hook_t data_hchannel[4];
} module_t;

int smcf_init(void);
int smcf_start(void);
void smcf_stop(void);
int smcf_module_register(module_t *module);

#endif /* __SMCF_H__ */
