#ifndef __MODULE_RTSP_VIDEO_H__
#define __MODULE_RTSP_VIDEO_H__

#define MODULE_ID_RTSP_VIDEO		2
#define MODULE_RTSP_VIDEO_NAME		"rtsp_video"

/* msg can send out by this module */

/* msg can process by this module */
#define MSG_I_START_RTSP_VIDEO		(((MODULE_ID_RTSP_VIDEO) << 16) | 0x01)
#define MSG_I_SET_RTSP_OSD_SHOW		(((MODULE_ID_RTSP_VIDEO) << 16) | 0x02)
#define MSG_I_RTSP_GET_DATA 		(((MODULE_ID_RTSP_VIDEO) << 16) | 0x03)

/**
 * driver_insmod_shell: if don't use this, it should be NULL.
 */
int module_rtsp_video_init(void);
int module_rtsp_video_deinit(void);

#endif /* __MODULE_RTSP_VIDEO_H__ */
