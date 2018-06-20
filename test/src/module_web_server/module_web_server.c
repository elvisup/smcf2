#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <linux/input.h>

#include <smcf.h>
#include <module_web_server.h>

#define MODULE_TAG "web server"
module_t *module_web_server;

module_t *module_web_server = NULL;

static int msg_process(int sender_id, int msgid, char data[16])
{
	int ret = 0;

	switch (msgid) {
	case MSG_I_START_WEB_SERVER: {
		printf("%s:%d -> MSG_I_START_WEB_SERVER, data:%s\n", __func__, __LINE__, data);
		break;
	}
	default:
		printf("%s:%d -> default, data:%s\n", __func__, __LINE__, data);
		break;
	}

	return ret;
}

int module_web_server_init(void)
{
	int ret;

	module_web_server = (module_t *)malloc(sizeof(module_t *));
	if (!module_web_server) {
		printf("%s(ERROR): malloc module_web_server error!\n", MODULE_TAG);
		return -1;
	}
	module_web_server->name        = MODULE_WEB_SERVER_NAME;
	module_web_server->id          = MODULE_ID_WEB_SERVER;
	module_web_server->msg_process = msg_process;

	ret = smcf_module_register(module_web_server);
	if (ret) {
		printf("%s(%d) smcf_module_register error!\n", __func__, __LINE__);
		return -1;
	}

	return 0;
}
