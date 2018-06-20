#ifndef __MODULE_FACE_CAPTURE_H__
#define __MODULE_FACE_CAPTURE_H__

#define MODULE_ID_FACE_CAPTURE       0
#define MODULE_FACE_CAPTURE_NAME     "face_capture"

/* msg can send out by this module */

/* msg can process by this module */
#define MSG_I_START_FACE_CAPTURE	(((MODULE_ID_FACE_CAPTURE) << 16) | 0x01)
#define MSG_I_STOP_FACE_CAPTURE		(((MODULE_ID_FACE_CAPTURE) << 16) | 0x02)
#define MSG_I_SEND_MEM_ADRESS		(((MODULE_ID_FACE_CAPTURE) << 16) | 0x03)
#define MSG_I_SEND_FACE_SHOWN		(((MODULE_ID_FACE_CAPTURE) << 16) | 0x04)

#define MSG_I_SET_FC_YAW            (((MODULE_ID_FACE_CAPTURE) << 16) | 0x05)
#define MSG_I_SET_FC_TILT           (((MODULE_ID_FACE_CAPTURE) << 16) | 0x06)
#define MSG_I_SET_FC_STRENGTH       (((MODULE_ID_FACE_CAPTURE) << 16) | 0x07)
#define MSG_I_SET_FC_FACE_IMG_SIZE  (((MODULE_ID_FACE_CAPTURE) << 16) | 0x08)
#define MSG_I_SET_FC_FILTER_SCORE   (((MODULE_ID_FACE_CAPTURE) << 16) | 0x09)

/**
 * driver_insmod_shell: if don't use this, it should be NULL.
 */
int module_face_capture_init(void);

#endif /*__MODULE_FACE_CAPTURE_H__*/
