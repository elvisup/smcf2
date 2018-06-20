#ifndef __CORE_H__
#define __CORE_H__

#include <smcf.h>
#include <msg.h>
#include <utils/slog.h>

#define MAX_MODULE_CHANNEL_CNT           4
#define MODULE_CHANNEL_ENABLE            1
#define MODULE_CHANNEL_DISABLE           0

typedef struct __module_block module_block_t;
typedef struct __module_block {
	module_t *module;
	/* used to get msg, when module work as receiver */
	int msgqb_id;
	module_block_t *next;
	pthread_mutex_t mutex;
} module_block_t;

typedef struct __module_list_ctx {
	module_block_t *module_block_list;
	int module_cnt;
} module_list_ctx_t;

typedef struct __smcf_context {
	module_list_ctx_t *module_list_ctx;
	//msgq_context_t *msgq_context;
	int msgq_context;

	/*
	 *module data channel bind info and other
	 */
	int data_channel_context;
} smcf_context_t;

#endif /* __CORE_H__ */
