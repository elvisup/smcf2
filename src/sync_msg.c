#include <stdio.h>
#include <core.h>
#include <module_manager.h>

int smcf2_send_msg_sync(int sender_id, int receiver_id, int msgid, char data[16])
{
	int ret = -1;
	module_t *receiver_module = NULL;
	if (smcf_startup != 1) {
		slog(LOG_INFO, "%s:%d -> smcf libray not init completely, pls wait!\n", __func__, __LINE__);
		return -1;
	}

	receiver_module = search_module_from_mblock_list(receiver_id);
	if (!receiver_module) {
		printf("ERROR\n");
		return -1;
	}

	if (receiver_module->sync_msg_process_en == 0) {
		slog(LOG_ERR, "%s:%d -> [%s] module do not support sync msg!!\n", __func__, __LINE__, receiver_module->name);
		return -1;
	}

	ret = receiver_module->sync_msg_process(sender_id, msgid, data);
	if (ret) {
		printf("ERROR\n");
	}

	return ret;
}
