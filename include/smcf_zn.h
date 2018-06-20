/**
 * smcf.h
 **/
#ifndef __SMCF_H__
#define __SMCF_H__

extern volatile int smcf_startup;

/*******************************************************************************\
 * 异步消息                                                                   *
\*******************************************************************************/

/**
 * 异步消息优先级
 **/
#define MSG_PRI_HIGH          0
#define MSG_PRI_MIDDLE        1
#define MSG_PRI_LOW           2

/**
 * 函数: send_msg
 * 输入:
 *   sender_id: 发送该消息的模块ID
 *   receiver_id: 接收该消息的模块ID
 *   msgpri: 该消息的优先级
 *   msgid: 发送消息模块发送给接收消息模块的消息ID，该消息ID在接收模块中定义，对应与接收消息模块中的一套消息处理方法
 *   data: 发送消息模块可以从发送模块中携带16byte的数据到接收模块中
 *
 * 注意:
 *   data: 发送模块可以携带16byte的数据到接收模块中，如果不需要携带数据，请将dara == NULL
 **/
int send_msg(int sender_id, int receiver_id, int msgpri, int msgid, char data[16]);

/**
 * 函数: recv_msg
 * 输入:
 *   receiver_id: 接收该消息的模块ID
 * 输出：
 *   sender_id: 发送该消息的模块ID
 *   msgid: 发送消息模块发送给接收消息模块的消息ID，该消息ID在接收模块中定义，对应与接收消息模块中的一套消息处理方法
 *   data: 发送消息模块可以从发送模块中携带16byte的数据到接收模块中，携带的数据保存在data中
 *
 * 注意:
 *   data: 发送模块可以携带16byte的数据到接收模块中，用户在接收数据时自行判断
 *
 *   如果auto_msg_process == 1，对于该模块来说recv_msg无效，模块接收数据由smcf自动管理，用户无法手动调用该接口进行
 *   接收数据，否则该接口返回错误-1
 **/
int recv_msg(int *sender_id, int receiver_id, int *msgid, char data[16]);

/*******************************************************************************\
 * 同步消息                                                                    *
\*******************************************************************************/

/**
 * 函数: send_msg_sync
 * 输入:
 *   sender_id: 发送该消息的模块ID
 *   receiver_id: 接收该消息的模块ID
 *   msgid: 发送消息模块发送给接收消息模块的消息ID，该消息ID在接收模块中定义，对应与接收消息模块中的一套消息处理方法
 *   data: 发送消息模块可以从发送模块中携带16byte的数据到接收模块中
 *
 * 注意:
 *   该接口用于发送同步消息，所以你应该确定接收模块支持同步消息，如果接收模块不支持同步消息，该接口将会返回错误
 **/
int send_msg_sync(int sender_id, int receiver_id, int msgid, char data[16]);

/*******************************************************************************\
 * 数据                                                                        *
\*******************************************************************************/

/**
 * 为了隐藏细节，数据接口有一些改动（Normal data接口）
 * 1. 对于发送数据模块，只需要将数据准备好，调用send_data接口即可发送数据
 * 2. 对于接收数据模块，只需要调用recv_data接口即可接收数据
 * 
 * 对于任何模块，不管是数据的发送还是接收，都只需要调用一个接口即可。
 **/

/**
 * 讨论点（Hook data接口）
 * 1. 对于Hook接口，设计初衷在于减少数据的拷贝
 * 2. 要想实现第一点，据需要暴露node_t结构
 * 3. 也可以不暴露node_t结构，但是代价就是多一次拷贝，即，normal接口需要2次拷贝，hook接口需要1次拷贝
 **/

/**
 * 数据通道类型
 **/
#define DATA_CHANNEL_SENDER    0
#define DATA_CHANNEL_RECEIVER  1

/**
 * data_channel_t
 * 配置数据通道属性结构体
 **/
typedef struct __data_channel {
	int enable;		/* 是否使能该数据通道，1：使能，0：不使能 */
	int role; 		/* 该通道角色：发送数据通道 or 接收数据通道 */
	int size; 		/* 该数据通道中每个数据节点的大小（单位：字节） */
	int num; 		/* 该数据通道中数据节点数量 */
	int module_id;		/* 如果该通道为发送数据通道，该域设置为-1，如果该通道为接收数据通道，该域设置为向该通道中发送数据的模块ID */
	int channel;		/* 如果该通道为发送数据通道，该域设置为-1，如果该通道为接收数据通道，该域设置为向该通道中发送数据的模块对应的数据通道ID */
} data_channel_t;

/* reserved for special use */
typedef struct __data_channel_hook {
	int enable;		/* 是否使能该数据通道，1：使能，0：不使能 */
	int role; 		/* 该通道角色：发送数据通道 or 接收数据通道 */
	int num; 		/* 该数据通道中数据节点数量 */
	int module_id;		/* 如果该通道为发送数据通道，该域设置为-1，如果该通道为接收数据通道，该域设置为向该通道中发送数据的模块ID */
	int channel;		/* 如果该通道为发送数据通道，该域设置为-1，如果该通道为接收数据通道，该域设置为向该通道中发送数据的模块对应的数据通道ID */
} data_channel_hook_t;

/**
 * 函数：get_data
 * 输入：
 *   current_module_id: 获取数据的模块的模块ID
 *   channel_id: 获取数据的通道ID
 *   data: 获取到的数据保存地址
 *
 * 注意:
 *   获取到的数据保存在data中，所以请保证data已经开辟足够的空间保存所要接收的数据。
 **/
void *get_data(int current_module_id, int channel_id, void *data);

/**
 * 函数：get_hook_data
 * 输入：
 *   current_module_id: 获取数据的模块的模块ID
 *   channel_id: 获取数据的通道ID
 *   data: 获取到的数据保存地址
 *   dsize: data size，表示要从该接口中获取多少数据
 *
 * 注意:
 *   获取到的数据保存在data中，所以请保证data已经开辟足够的空间保存所要接收的数据。
 **/
void *get_hook_data(int current_module_id, int channel_id, void *data, unsigned int dsize);

/**
 * 函数：put_data
 * 输入：
 *   current_module_id: 获取数据的模块的模块ID
 *   channel_id: 获取数据的通道ID
 *   data: 存放需要发送的数据地址
 * ...
 **/
int put_data(int current_module_id, int channel_id, void *data);
int put_hook_data(int current_module_id, int channel_id, void *data);

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
