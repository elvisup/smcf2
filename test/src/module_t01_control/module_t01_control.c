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

#include <smcf.h>
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
			send_msg_sync(MODULE_ID_T01_CONTROL, MODULE_ID_FACE_CAPTURE, 3, "ctl recv fc msg");
			break;
		case MODULE_ID_RTSP_VIDEO:
			send_msg_sync(MODULE_ID_T01_CONTROL, MODULE_ID_RTSP_VIDEO, 3, "ctl recv rp msg");
			break;
		default:
			printf("INFO: %s[%d], I dont know who are you!\n", __func__, __LINE__);
			break;
	}

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

	pthread_mutex_init(&ctl_mutex_msg, NULL);
	ret = smcf_module_register(module_t01_control);
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
