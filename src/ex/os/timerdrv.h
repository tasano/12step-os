#ifndef _TIMERDRV_H_INCLUDE_
#define _TIMERDRV_H_INCLUDE_

#include "defines.h"

#define TIMERDRV_DEVICE_NUM 1
#define TIMERDRV_CMD_EXPIRE 'e' /* タイマ満了 */
#define TIMERDRV_CMD_START  's' /* タイマのスタート */

struct timerreq {
    kz_msgbox_id_t id; /* タイマ満了時のメッセージ送信先 */
    int            msec;
};

#endif
