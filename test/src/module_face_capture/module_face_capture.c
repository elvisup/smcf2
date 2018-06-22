#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <semaphore.h>
#include <dirent.h>
#include <sys/statvfs.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/time.h>


#include <smcf2.h>
#include <module_face_capture.h>
#include <module_rtsp_video.h>
#include <module_t01_control.h>

#define MODULE_TAG "face_capture"
module_t *module_face_capture;

pthread_mutex_t fc_mutex_msg;
static int msg_process(int sender_id, int msgid, char data[16])
{
	pthread_mutex_lock(&fc_mutex_msg);

	int ret = 0;

	switch(msgid) {
	case MSG_I_START_FACE_CAPTURE: {
		printf("%s:%d -> MSG_I_START_FACE_CAPTURE, data: %s\n", __func__, __LINE__, data);
		break;
	}
	case MSG_I_STOP_FACE_CAPTURE: {
		printf("%s:%d -> MSG_I_STOP_FACE_CAPTURE, data:%s\n", __func__, __LINE__, data);
		break;
	}
	case MSG_I_SEND_FACE_SHOWN: {
		printf("%s:%d -> MSG_I_SEND_FACE_SHOWN, data:%s\n", __func__, __LINE__, data);
		break;
	}
	default:
		printf("%s:%d -> default, data:%s\n", __func__, __LINE__, data);
		break;
	}

	pthread_mutex_unlock(&fc_mutex_msg);
	return ret;
}

void *put_data_to_ch0(void *arg)
{
	char *email = "cape module: ";
	int i = 0;
	while (1) {
		char buf[32] = {0};
		sprintf(buf, "%s%d", email, i);
		char *p = buf;
		int context = smcf2_sender_request_data(MODULE_ID_FACE_CAPTURE, 0);
		if (context == -1) {
			printf("%s:%d smcf2_sender_request_data error!\n", __func__, __LINE__);
			usleep(1000*30);
		} else {
			smcf2_sender_put_data(context, MODULE_ID_FACE_CAPTURE, 0, (void **)&p);
			smcf2_send_msg(MODULE_ID_FACE_CAPTURE, MODULE_ID_RTSP_VIDEO, MSG_PRI_LOW, MSG_I_RTSP_GET_DATA, "fc send rtsp");
			i++;

			//SEND SYNC MSG
			smcf2_send_msg_sync(MODULE_ID_FACE_CAPTURE, MODULE_ID_T01_CONTROL, 2, "sync msg from fc");
			sleep(1);
		}
	}
}

void *put_data_to_hch0(void *arg)
{
	char *email = "fc hookchn: ";
	int i = 0;
	char buf[32] = {0};
	char *p = NULL;
	while (1) {
		memset(buf, 0, 32);
		sprintf(buf, "%s%d", email, i);
		p = buf;
		int context = smcf2_sender_request_hook_data(MODULE_ID_FACE_CAPTURE, 0);
		if (context == -1) {
			printf("%s:%d sender_alloc_hook_data error!\n", __func__, __LINE__);
			usleep(1000*30);
		} else {
			smcf2_sender_put_hook_data(context, MODULE_ID_FACE_CAPTURE, 0, (void **)&p);
			i++;
			sleep(1);
		}
	}
}

int fc_hook_ch0_data_release(void *data)
{
	printf("%s:%d -> %p\n", __func__, __LINE__, data);
	return 0;
}

int fc_sync_msg_process(int sender_id, int msgid, char data[16])
{
	printf("INFO: %s[%d], senderid: %d, msgid: %d, msg: %s\n", __func__, __LINE__, sender_id, msgid, data);
	return 0;
}

int module_face_capture_init(void)
{
	int ret;

	module_face_capture = (module_t *)malloc(sizeof(module_t));
	if (!module_face_capture) {
		printf("ERROR: %s[%d] -> malloc module_face_capture error!\n", __func__, __LINE__);
		return -1;
	}
	module_face_capture->name        = MODULE_FACE_CAPTURE_NAME;
	module_face_capture->id          = MODULE_ID_FACE_CAPTURE;
	module_face_capture->msg_process = msg_process;
	module_face_capture->auto_msg_process_en = 1;
	module_face_capture->sync_msg_process_en = 1;
	module_face_capture->sync_msg_process = fc_sync_msg_process;

	//NORMAL CHN
	module_face_capture->data_channel[0].enable = 1;
	module_face_capture->data_channel[0].role = DATA_CHANNEL_SENDER;
	module_face_capture->data_channel[0].size = 1024;
	module_face_capture->data_channel[0].num  = 8;
	module_face_capture->data_channel[0].module_id  = -1;
	module_face_capture->data_channel[0].channel    = -1;

	//HOOK CHN
	module_face_capture->data_hchannel[0].enable = 1;
	module_face_capture->data_hchannel[0].role = DATA_CHANNEL_SENDER;
	module_face_capture->data_hchannel[0].num  = 8;
	module_face_capture->data_hchannel[0].module_id  = -1;
	module_face_capture->data_hchannel[0].channel    = -1;
	module_face_capture->data_hchannel[0].release    = fc_hook_ch0_data_release;

	pthread_mutex_init(&fc_mutex_msg, NULL);

	pthread_t pidmd;
	ret = pthread_create(&pidmd, NULL, put_data_to_ch0, NULL);
	if (ret) {
		printf("pthread create pidmd error\n");
	}
	pthread_detach(pidmd);

	pthread_t pidhook0;
	ret = pthread_create(&pidhook0, NULL, put_data_to_hch0, NULL);
	if (ret) {
		printf("pthread create pidhook0 error\n");
	}
	pthread_detach(pidhook0);

	ret = smcf2_module_register(module_face_capture);
	if (ret == -1) {
		printf("ERROR\n");
	}

	return 0;
}

