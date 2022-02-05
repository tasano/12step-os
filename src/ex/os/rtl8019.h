#ifndef _RTL8019_H_INCLUDE_
#define _RTL8019_H_INCLUDE_

int  rtl8019_init( int index, unsigned char* macaddr );
int  rtl8019_intr_is_send_enable( int index );
void rtl8019_intr_send_enable( int index );
void rtl8019_intr_send_disable( int index );
int  rtl8019_intr_is_recv_enable( int index );
void rtl8019_intr_recv_enable( int index );
void rtl8019_intr_recv_disable( int index );
int  rtl8019_is_send_enable( int index );
int  rtl8019_is_recv_enable( int index );
int  rtl8019_intr_clear( int index );
int  rtl8019_intr_clear_send( int index );
int  rtl8019_intr_clear_recv( int index );
int  rtl8019_send( int index, int size, char* buf );
int  rtl8019_recv( int index, char* buf );

#endif
