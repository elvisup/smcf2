#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <signal.h>
#include <pthread.h>

#include <smcf2.h>
#include <module_rtsp_video.h>
#include <module_face_capture.h>
#include <module_face_recognition.h>
#include <module_web_server.h>
#include <module_t01_control.h>

pthread_mutex_t mutex_msg;
static int stop = 0;

static void auto_deinit(void)
{
	/* int ret = -1; */

	/*ret = smcf2_stop();*/
	/*ret = smcf_deinit(msg_recv_handler);*/

	stop = 1;

	return;
}

static void signal_handler(int signal)
{
	switch (signal) {
	case SIGINT:  /* cutout by ctrl + c */
	case SIGTSTP: /* cutout by ctrl + z */
	case SIGKILL:
	case SIGTERM:
		auto_deinit();
	break;
	}
}

static void signal_init(void)
{
	signal(SIGINT,  signal_handler);
	signal(SIGTSTP, signal_handler);
	signal(SIGKILL, signal_handler);
	signal(SIGTERM, signal_handler);
}

static int start_up(void)
{
	return 0;
}

int main(int argc, char **argv)
{
	int ret = 0;

	pthread_mutex_init(&mutex_msg, NULL);

	/* init simple module communaction framework */
	ret = smcf2_init();
	if (ret) {
		printf("%s:%d -> smcf_init error!\n", __func__, __LINE__);
		return -1;
	}

	/* rtsp video init */
	ret = module_rtsp_video_init();
	if (ret) {
		printf("%s:%d -> module_rtsp_video_init error!\n", __func__, __LINE__);
		return -1;
	}

#if 1
	/* face recognition init */
	ret = Module_FaceRecognition_Init();
	if (ret) {
		printf("%s:%d -> Module_FaceRecognition_Init error!\n", __func__, __LINE__);
		return -1;
	}

	/* face capture init */
	ret = module_face_capture_init();
	if (ret) {
		printf("%s:%d -> module_face_capture_init error!\n", __func__, __LINE__);
		return -1;
	}

	/* t01 control init */
	ret = Module_T01_Control_Init();
	if (ret) {
		printf("%s:%d -> Module_T01_Control_Init error!\n", __func__, __LINE__);
		return -1;
	}
#endif

	/* start simple module communaction framework */
	ret = smcf2_start();
	if (ret) {
		printf("%s:%d -> smcf_start error!\n", __func__, __LINE__);
		return -1;
	}

	/* start up */
	ret = start_up();
	if (ret) {
		/* TODO: error */
		return -1;
	}

	/* set signal handler */
	signal_init();

	/* set autoexit */
	atexit(auto_deinit);

	/* loop */
	for(;;){
		if (stop)
			break;
		else
			sleep(1);
	}

	return ret;
}
