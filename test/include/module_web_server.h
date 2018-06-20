#ifndef __MODULE_WEB_SERVER_H__
#define __MODULE_WEB_SERVER_H__

#define MODULE_ID_WEB_SERVER       4
#define MODULE_WEB_SERVER_NAME     "web_server"

/* msg can send out by this module */
#define MSG_I_START_WEB_SERVER     (((MODULE_ID_WEB_SERVER) << 16) | 0x01)

/* msg can process by this module */

/**
 * driver_insmod_shell: if don't use this, it should be NULL.
 */
int module_web_server_init(void);
int module_web_server_deinit(void);

#endif /*__MODULE_WEB_SERVER_H__*/
