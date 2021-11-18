#include "defines.h"
#include "interrupt.h"
#include "intr.h"
#include "lib.h"
#include "serial.h"

static void intr( softvec_type_t type, unsigned long sp )
{
    static char buf[32];
    static int  len;

    int c = getc();
    if ( c != '\n' ) {
        buf[len++] = c;
    } else {
        buf[len++] = '\0';
        if ( !strncmp( buf, "echo", 4 ) ) {
            puts( buf + 4 );
            puts( "\n" );
        } else {
            puts( "unknown.\n" );
        }
        puts( "> " );
        len = 0;
    }
}

int main( void )
{
    /* 割込み無効にする */
    INTR_DISABLE;

    puts( "kozos boot succeed!\n" );

    softvec_setintr( SOFTVEC_TYPE_SERINTR, intr );
    serial_intr_recv_enable( SERIAL_DEFAULT_DEVICE );

    puts( "> " );

    /* 割込み有効にする */
    INTR_ENABLE;

    while ( 1 ) {
        /* 省電力モードに移行 */
        asm volatile( "sleep" );
    }

    return 0;
}
