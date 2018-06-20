#ifndef __MODULE_FACE_RECOGNITION_H__
#define __MODULE_FACE_RECOGNITION_H__

#define MODULE_ID_FACE_RECOGNITION       1
#define MODULE_FACE_RECOGNITION_NAME     "face_recognition"

/* msg can send out by this module */

/* msg can process by this module */
#define MSG_I_SET_RECOGNITION_THRESHOLD		(((MODULE_ID_FACE_RECOGNITION) << 16) | 0x01)

/**
 * driver_insmod_shell: if don't use this, it should be NULL.
 */
int Module_FaceRecognition_Init(void);
int Module_FaceRecognition_DeInit(void);

#endif /*__MODULE_FACE_RECOGNITION_H__*/
