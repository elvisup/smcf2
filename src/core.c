#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>

#include <core.h>
#include <msg.h>
#include <module_manager.h>

smcf_context_t *context = NULL;

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
			printf("WARNING: module [%s(ID:%d)] get an null message\n", module_name, module_id);
			continue;
		}

		/* process msg */
		ret = module->msg_process(msg->sender_id, msg->msgid, msg->data);
		if (ret) {
			printf("ERROR\n");
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
		printf("%s:%d -> malloc smcf_context_t error!\n", __func__, __LINE__);
		goto error_context;
	}
	memset(context, 0x0, sizeof(smcf_context_t));

	context->module_list_ctx = (module_list_ctx_t *)malloc(sizeof(module_list_ctx_t));
	if (!context->module_list_ctx) {
		printf("%s:%d -> malloc module_list_ctx error!\n", __func__, __LINE__);
		goto error_module_list_ctx;
	}
	memset(context->module_list_ctx, 0x0, sizeof(module_list_ctx_t));
	context->module_list_ctx->module_block_list = NULL;
	context->module_list_ctx->module_cnt = 0;

	context->msgq_context = msgq_init(MAX_MSGQUEUE_BLOCK_NUM);

	return 0;

error_module_list_ctx:
	free(context);
error_context:
	return -1;
}

int smcf_start(void)
{
	int ret = -1;

	/*ret = smcf_module_register(context->lm);*/

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
		printf("ERROR: alloc memory for module_block error!\n");
		return -1;
	}

	module_block->module = module;
	module_block->msgqb_id = msgq_bind(context->msgq_context, module->id);;
	pthread_mutex_init(&module_block->mutex , NULL);

	ret = add_module2mblock_list(module_block);
	if (ret) {
		printf("ERROR\n");
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
			printf("error\n");
		}
	}
	printf("%s:%d -> register %s to smcf ok!\n", __func__, __LINE__, module->name);
	return ret;

error:
	printf("%s:%d -> register %s to smcf error!\n", __func__, __LINE__, module->name);
	free(module_block);
	return ret;
}

