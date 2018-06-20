#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include <core.h>
#include <msg.h>
#include <data.h>
#include <module_manager.h>

smcf_context_t *context = NULL;
extern data_chn_table_t gdc_table;
volatile int smcf_startup;


static void *msg_process_entry(void *data) {
	int ret;
	msg_t *msg;
	module_block_t *module_block = (module_block_t *)data;
	module_t *module = module_block->module;
	int module_id = module->id;
	char *module_name = module->name;

	/* set thread name */
	prctl(PR_SET_NAME, (unsigned long)module_name, 0, 0, 0);

	for (;;) {
		/* get msg */
		msg = read_msg(context->msgq_context, module_id);
		if (!msg) {
			slog(LOG_INFO, "module [%s(ID:%d)] get an null message\n", module_name, module_id);
			continue;
		}

		/* process msg */
		ret = module->msg_process(msg->sender_id, msg->msgid, msg->data);
		if (ret) {
			slog(LOG_ERR, "%s:%d -> %s msg_process error!\n", __func__, __LINE__, module_name);
		}

		/* free msg and msg->data */
		free_msg(context->msgq_context, msg);
	}

	return 0;
}

int smcf_init(void)
{
	context = (smcf_context_t *)malloc(sizeof(smcf_context_t));
	if (!context) {
		slog(LOG_ERR, "%s:%d -> malloc smcf_context_t error(%s)!\n", __func__, __LINE__, strerror(errno));
		goto error_context;
	}
	memset(context, 0x0, sizeof(smcf_context_t));

	context->module_list_ctx = (module_list_ctx_t *)malloc(sizeof(module_list_ctx_t));
	if (!context->module_list_ctx) {
		slog(LOG_ERR, "%s:%d -> malloc module_list_ctx error(%s)!\n", __func__, __LINE__, strerror(errno));
		goto error_module_list_ctx;
	}
	memset(context->module_list_ctx, 0x0, sizeof(module_list_ctx_t));
	context->module_list_ctx->module_block_list = NULL;
	context->module_list_ctx->module_cnt = 0;

	context->msgq_context = msgq_init(MAX_MSGQUEUE_BLOCK_NUM);
	if (context->msgq_context == 0) {
		slog(LOG_ERR, "%s:%d -> msgq_init error!\n", __func__, __LINE__);
		goto error_msgq_init;
	}

	return 0;

error_msgq_init:
	free(context->module_list_ctx);
error_module_list_ctx:
	free(context);
error_context:
	return -1;
}

int smcf_start(void)
{
	int ret = -1;

	ret = attach_module_channel();
	if (ret) {
		slog(LOG_ERR, "%s:%d -> attach_module_channel!\n", __func__, __LINE__);
	} else {
		smcf_startup = 1;
	}

	return ret;
}

void smcf_stop(void)
{
	return;
}

int smcf_module_register(module_t *module)
{
	int ret = -1;

	module_block_t *module_block = (module_block_t *)malloc(sizeof(module_block_t));
	if (!module_block) {
		slog(LOG_ERR, "%s:%d -> alloc memory for module_block error(%s)!\n", __func__, __LINE__, strerror(errno));
		return -1;
	}

	module_block->module = module;
	module_block->msgqb_id = msgq_bind(context->msgq_context, module->id);;
	pthread_mutex_init(&module_block->mutex , NULL);

	ret = add_module2mblock_list(module_block);
	if (ret) {
		slog(LOG_ERR, "%s:%d -> add_module2mblock_list!\n", __func__, __LINE__);
		goto error;
	}

	ret = module_data_channel_init(module);
	if (ret == -1) {
		slog(LOG_ERR, "%s:%d -> module_data_channel_init error!\n", __func__, __LINE__);
		goto error;
	}

	if (module->auto_msg_process_en) {
		if (module->msg_process) {
			pthread_t thread;
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
			ret = pthread_create(&thread, &attr, &msg_process_entry, (void*)module_block);
		} else {
			//TODO
			slog(LOG_ERR, "%s:%d -> msg_process, maybe you should manager message manual!\n", __func__, __LINE__);
		}
	}
	slog(LOG_INFO, "%s:%d -> register %s to smcf ok!\n", __func__, __LINE__, module->name);
	return ret;

error:
	slog(LOG_ERR, "%s:%d -> register %s to smcf error!\n", __func__, __LINE__, module->name);
	free(module_block);
	return ret;
}

