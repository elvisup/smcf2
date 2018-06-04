#include <stdio.h>
#include <module_manager.h>

int send_msg_sync(int sender_id, int receiver_id, int msgid, char data[16])
{
	int ret = -1;
	module_t *receiver_module = NULL;
	receiver_module = search_module_from_mblock_list(receiver_id);
	if (!receiver_module) {
		printf("ERROR\n");
		return -1;
	}

	if (receiver_module->sync_msg_process_en == 0) {
		printf("Waring: [%s] module do not support sync msg!\n", receiver_module->name);
		return -1;
	}

	ret = receiver_module->sync_msg_process(sender_id, msgid, data);
	if (ret) {
		printf("ERROR\n");
	}

	return ret;
}
