#ifndef _SERIAL_H_INCLUDE_
#define _SERIAL_H_INCLUDE_

#include "defines.h"

int   serial_init( int index );               /* デバイス初期化 */
int   serial_is_send_enable( int index );     /* 送信可能か？ */
int   serial_send_byte( int index, uint8 c ); /* 1文字送信 */
int   serial_is_recv_enable( int index );     /* 受信可能か？ */
uint8 serial_recv_byte( int index );          /* 1文字受信 */

#endif
