#ifndef __MODULE_T01_CONTROL_H__
#define __MODULE_T01_CONTROL_H__

#define MODULE_ID_T01_CONTROL       3
#define MODULE_T01_CONTROL_NAME     "t01_control"

/* msg can send out by this module */

/* msg can process by this module */
#define MSG_I_SET_TC_OFFSET            (((MODULE_ID_T01_CONTROL) << 16) | 0x01)
#define MSG_I_SET_TC_SCALES            (((MODULE_ID_T01_CONTROL) << 16) | 0x02)
/**
 * driver_insmod_shell: if don't use this, it should be NULL.
 */
int Module_T01_Control_Init(void);
int Module_T01_Control_DeInit(void);

#endif /*__MODULE_T01_CONTROL_H__*/
