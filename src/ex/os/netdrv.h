#ifndef _NETDRV_H_INCLUDE_
#define _NETDRV_H_INCLUDE_

#include "defines.h"

#define NETDRV_DEVICE_NUM   1
#define NETDRV_CMD_USE      'u' /* イーサネット・ドライバの使用開始 */
#define NETDRV_CMD_SEND     's' /* イーサネットへのフレーム出力 */
#define NETDRV_CMD_RECVINTR 'r' /* 受信割り込み */
#define NETDRV_CMD_SENDINTR 'i' /* 送信割り込み */

#define MACADDR_SIZE 6

#define DEFAULT_NETBUF_SIZE 1800

struct netbuf {
    unsigned char  cmd;
    short          size;
    struct netbuf* next;

    union {
        union {
            struct {
                uint8 addr[MACADDR_SIZE];
            } macaddr;
        } common;
    } option;

    /*
     * unionのサイズが2の倍数でもデータの先頭が確実に4バイトアライメント
     * されるように、ここでポインタを定義する
     */
    char* top; /* データの先頭を指すポインタ */

    char data[0];
};

#endif
