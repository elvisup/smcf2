#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <linux/input.h>

#include <smcf2.h>
#include <module_t01_control.h>
#include <module_face_capture.h>
#include <module_rtsp_video.h>

#define MODULE_TAG "t01 ctl"
module_t *module_t01_control;

pthread_mutex_t ctl_mutex_msg;

static int msg_process(int sender_id, int msgid, char data[16])
{
	pthread_mutex_lock(&ctl_mutex_msg);

	int ret = 0;

	switch (msgid) {
	case MSG_I_SET_TC_OFFSET: {
		printf("%s:%d -> MSG_I_SET_TC_OFFSET, data:%s\n", __func__, __LINE__, data);
		break;
	}
	case MSG_I_SET_TC_SCALES: {
		printf("%s:%d -> MSG_I_SET_TC_SCALES, data:%s\n", __func__, __LINE__, data);
		break;
	}
	default:
		printf("%s:%d -> default, data:%s\n", __func__, __LINE__, data);
		break;
	}

	pthread_mutex_unlock(&ctl_mutex_msg);

	return ret;
}

int ctl_sync_msg_process(int sender_id, int msgid, char data[16])
{
	printf("INFO: %s[%d], senderid: %d, msgid: %d, msg: %s\n", __func__, __LINE__, sender_id, msgid, data);

	//SEND SYNC MSG
	switch (sender_id){
		case MODULE_ID_FACE_CAPTURE:
			smcf2_send_msg_sync(MODULE_ID_T01_CONTROL, MODULE_ID_FACE_CAPTURE, 3, "ctl recv fc msg");
			break;
		case MODULE_ID_RTSP_VIDEO:
			smcf2_send_msg(MODULE_ID_T01_CONTROL, MODULE_ID_RTSP_VIDEO, MSG_PRI_LOW, MSG_I_SET_RTSP_OSD_SHOW, "ctl send rtsp");
			break;
		default:
			printf("INFO: %s[%d], I dont know who are you!\n", __func__, __LINE__);
			break;
	}

	return 0;
}

void *ctl_get_data_from_hookch0(void *arg)
{
	char *rbuf = malloc(1024);
	while (1) {
		int context = smcf2_recever_get_hook_data(MODULE_ID_T01_CONTROL, 0, (void **)&rbuf);
		if (context == -1) {
			printf("%s:%d recever_get_hook_data error!\n", __func__, __LINE__);
			usleep(1000*30);
		} else {
			printf("%s:%d -> [%s], ctx: 0x%x\n", __func__, __LINE__, rbuf, context);
			smcf2_recever_put_hook_data(context, MODULE_ID_T01_CONTROL, 0);
		}
	}
}

int ctl_hook_ch0_data_release(void *data)
{
	/*printf("%s:%d -> %p\n", __func__, __LINE__, data);*/
	return 0;
}

int Module_T01_Control_Init(void)
{
	int ret;

	module_t01_control = (module_t *)malloc(sizeof(module_t));
	if (!module_t01_control) {
		printf("%s(ERROR): malloc memory error!\n", MODULE_TAG);
		return -1;
	}
	module_t01_control->name        = MODULE_T01_CONTROL_NAME;
	module_t01_control->id          = MODULE_ID_T01_CONTROL;
	module_t01_control->msg_process = msg_process;

	module_t01_control->auto_msg_process_en = 1;
	module_t01_control->sync_msg_process_en = 1;
	module_t01_control->sync_msg_process = ctl_sync_msg_process;

	//HOOK CHN
	module_t01_control->data_hchannel[0].enable = 1;
	module_t01_control->data_hchannel[0].role = DATA_CHANNEL_RECEIVER;
	module_t01_control->data_hchannel[0].num  = 8;
	module_t01_control->data_hchannel[0].module_id  = MODULE_ID_FACE_CAPTURE;
	module_t01_control->data_hchannel[0].channel    = 0;
	module_t01_control->data_hchannel[0].release    = ctl_hook_ch0_data_release;

	pthread_t pidhook0;
	ret = pthread_create(&pidhook0, NULL, ctl_get_data_from_hookch0, NULL);
	if (ret) {
		printf("pthread create pidhook0 error\n");
	}
	pthread_detach(pidhook0);

	pthread_mutex_init(&ctl_mutex_msg, NULL);
	ret = smcf2_module_register(module_t01_control);
	if (ret) {
		printf("%s(ERROR): module register error!\n", MODULE_TAG);
		return -1;
	}

	return 0;
}

int Module_T01_Control_DeInit(void)
{
	return 0;
}
