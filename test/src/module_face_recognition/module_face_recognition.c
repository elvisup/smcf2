#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <pthread.h>
#include <linux/input.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>

#include <smcf2.h>
#include <module_face_recognition.h>
#include <module_rtsp_video.h>

#define MODULE_TAG "face_fr"
module_t *module_face_recognition;

pthread_mutex_t fr_mutex_msg;

static int msg_process(int sender_id, int msgid, char data[16])
{
	int ret = -1;
	pthread_mutex_lock(&fr_mutex_msg);

	switch (msgid) {
	case MSG_I_SET_RECOGNITION_THRESHOLD:
		printf("%s:%d -> MSG_I_SET_RECOGNITION_THRESHOLD, data:%s\n", __func__, __LINE__, data);
		break;
	default:
		printf("%s:%d -> default, data:%s\n", __func__, __LINE__, data);
		break;
	}

	pthread_mutex_unlock(&fr_mutex_msg);
	return ret;
}

void *fr_get_data_from_ch0(void *arg)
{
	char *rbuf = malloc(1024);
	while (1) {
		int context = smcf2_recever_get_data(MODULE_ID_FACE_RECOGNITION, 0, (void **)&rbuf);
		if (context == -1) {
			printf("%s:%d recever_get_data error!\n", __func__, __LINE__);
			usleep(1000*30);
		} else {
			printf("%s:%d -> [%s], ctx: 0x%x\n", __func__, __LINE__, rbuf, context);
			smcf2_recever_release_data(context, MODULE_ID_FACE_RECOGNITION, 0);
		}
	}
}
int Module_FaceRecognition_Init(void)
{
	int ret;

	module_face_recognition = (module_t *)malloc(sizeof(module_t));
	if (!module_face_recognition) {
		printf("%s(ERROR): malloc memory error!\n", MODULE_TAG);
		return -1;
	}
	module_face_recognition->name        = MODULE_FACE_RECOGNITION_NAME;
	module_face_recognition->id          = MODULE_ID_FACE_RECOGNITION;
	module_face_recognition->msg_process = msg_process;

	//CHN0 ATTR
	module_face_recognition->data_channel[0].enable = 1;
	module_face_recognition->data_channel[0].role = DATA_CHANNEL_RECEIVER;
	module_face_recognition->data_channel[0].size = 1024;
	module_face_recognition->data_channel[0].num  = 8;
	module_face_recognition->data_channel[0].module_id  = MODULE_ID_RTSP_VIDEO;
	module_face_recognition->data_channel[0].channel    = 1;

	pthread_mutex_init(&fr_mutex_msg, NULL);

	pthread_t frpid0;
	ret = pthread_create(&frpid0, NULL, fr_get_data_from_ch0, NULL);
	if (ret) {
		printf("pthread create frpid0 error\n");
	}
	pthread_detach(frpid0);

	/* register module */
	ret = smcf2_module_register(module_face_recognition);
	if (ret) {
		printf("%s(ERROR): module register error!\n", MODULE_TAG);
		return -1;
	}

	return 0;
}
