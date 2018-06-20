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
#include <module_rtsp_video.h>
#include <module_face_capture.h>

#define MODULE_TAG "rtsp"
module_t *module_rtsp_video;

pthread_mutex_t rtsp_mutex_msg;

static int msg_process(int sender_id, int msgid, char data[16])
{
	pthread_mutex_lock(&rtsp_mutex_msg);

	int ret = 0;

	switch (msgid) {
	case MSG_I_RTSP_GET_DATA: {
		printf("%s:%d -> MSG_I_RTSP_GET_DATA, data:%s\n", __func__, __LINE__, data);
		break;
	}
	case MSG_I_START_RTSP_VIDEO: {
		printf("%s:%d -> MSG_I_START_RTSP_VIDEO, data:%s\n", __func__, __LINE__, data);
		break;
	}
	case MSG_I_SET_RTSP_OSD_SHOW: {
		printf("%s:%d -> MSG_I_SET_RTSP_OSD_SHOW, data:%s\n", __func__, __LINE__, data);
		break;
	}
	default:
		printf("%s:%d -> default, data:%s\n", __func__, __LINE__, data);
		break;
	}

	pthread_mutex_unlock(&rtsp_mutex_msg);
	return 0;
}

void *rtsp_put_data_to_ch0(void *arg)
{
	char *email = "rtsp module: ";
	int i = 0;
	while (1) {
		if (smcf_startup) {
			char buf[32] = {0};
			sprintf(buf, "%s%d", email, i);
			put_data(MODULE_ID_RTSP_VIDEO, 1, buf);
			i++;
			sleep(1);
		} else {
			usleep(1000*30);
		}
	}
}

void *rtsp_get_data_from_ch0(void *arg)
{
	while (1) {
		if (smcf_startup) {
			char buffer[128] = {0};
			/*get_data(MODULE_ID_FACE_CAPTURE, 0, buffer);*/
			get_data(MODULE_ID_RTSP_VIDEO, 0, buffer);
			printf("%s:%d -> [%s]\n", __func__, __LINE__, buffer);
			send_msg(MODULE_ID_RTSP_VIDEO, MODULE_ID_FACE_CAPTURE, MSG_PRI_LOW, MSG_I_START_FACE_CAPTURE, "rtsp send fc");

			//SEND SYNC MSG
			send_msg_sync(MODULE_ID_RTSP_VIDEO, MODULE_ID_FACE_CAPTURE, 2, "from rtsp");
		} else {
			usleep(1000*30);
		}
	}
}

int module_rtsp_video_init(void)
{
	int ret;

	module_rtsp_video = (module_t *)malloc(sizeof(module_t));
	if (!module_rtsp_video) {
		printf("%s(ERROR): malloc module_rtsp_video error!\n", MODULE_TAG);
		return -1;
	}
	module_rtsp_video->name        = MODULE_RTSP_VIDEO_NAME;
	module_rtsp_video->id          = MODULE_ID_RTSP_VIDEO;
	module_rtsp_video->msg_process = msg_process;

	module_rtsp_video->auto_msg_process_en = 1;
	module_rtsp_video->sync_msg_process_en = 0;

	//CHN0 ATTR
	module_rtsp_video->data_channel[0].enable = 1;
	module_rtsp_video->data_channel[0].role = DATA_CHANNEL_RECEIVER;
	module_rtsp_video->data_channel[0].size = 1024;
	module_rtsp_video->data_channel[0].num  = 8;
	module_rtsp_video->data_channel[0].module_id  = MODULE_ID_FACE_CAPTURE;
	module_rtsp_video->data_channel[0].channel    = 0;

	//CHN1 ATTR
	module_rtsp_video->data_channel[1].enable = 1;
	module_rtsp_video->data_channel[1].role = DATA_CHANNEL_SENDER;
	module_rtsp_video->data_channel[1].size = 1024;
	module_rtsp_video->data_channel[1].num  = 8;
	module_rtsp_video->data_channel[1].module_id  = -1;
	module_rtsp_video->data_channel[1].channel    = -1;

	pthread_mutex_init(&rtsp_mutex_msg, NULL);

	pthread_t pid0;
	ret = pthread_create(&pid0, NULL, rtsp_get_data_from_ch0, NULL);
	if (ret) {
		printf("pthread create pid0 error\n");
	}
	pthread_detach(pid0);

	pthread_t rp_pidmd;
	ret = pthread_create(&rp_pidmd, NULL, rtsp_put_data_to_ch0, NULL);
	if (ret) {
		printf("pthread create rp_pidmd error\n");
	}
	pthread_detach(rp_pidmd);

	ret = smcf_module_register(module_rtsp_video);
	if (ret == -1) {
		printf("ERROR\n");
	}

	return 0;
}
