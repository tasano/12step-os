#ifndef _SERIAL_H_INCLUDE_
#define _SERIAL_H_INCLUDE_

#include "defines.h"

int   serial_init( int index );                /* デバイス初期化 */
int   serial_is_send_enable( int index );      /* 送信可能か？ */
int   serial_send_byte( int index, uint8 c );  /* 1文字送信 */
int   serial_is_recv_enable( int index );      /* 受信可能か？ */
uint8 serial_recv_byte( int index );           /* 1文字受信 */
int   serial_intr_is_send_enable( int index ); /* 送信割込み有効か？ */
void  serial_intr_send_enable( int index );    /* 送信割込み有効化 */
void  serial_intr_send_disable( int index );   /* 送信割込み無効化 */
int   serial_intr_is_recv_enable( int index ); /* 受信割込み有効か？ */
void  serial_intr_recv_enable( int index );    /* 受信割込み有効化 */
void  serial_intr_recv_disable( int index );   /* 受信割込み無効化 */

#endif
