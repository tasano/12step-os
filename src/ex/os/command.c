#include "consdrv.h"
#include "defines.h"
#include "kozos.h"
#include "lib.h"
#include "timerdrv.h"

/* コンソール・ドライバの使用開始をコンソール・ドライバに依頼する */
static void send_use( int index )
{
    char* p;
    p    = kz_kmalloc( 3 );
    p[0] = '0';
    p[1] = CONSDRV_CMD_USE;
    p[2] = '0' + index;
    kz_send( MSGBOX_ID_CONSOUTPUT, 3, p );
}

/* コンソールへの文字列出力をコンソール・ドライバに依頼する */
static void send_write( char* str )
{
    char* p;
    int   len;
    len  = strlen( str );
    p    = kz_kmalloc( len + 2 );
    p[0] = '0';
    p[1] = CONSDRV_CMD_WRITE;
    memcpy( &p[2], str, len );
    kz_send( MSGBOX_ID_CONSOUTPUT, len + 2, p );
}

/* タイマのカウント開始をタイマ・ドライバに依頼する */
static void send_start( int msec )
{
    struct timerreq* req;
    req       = kz_kmalloc( sizeof( *req ) );
    req->id   = MSGBOX_ID_CONSINPUT;
    req->msec = msec;
    kz_send( MSGBOX_ID_TIMDRIVE, TIMERDRV_CMD_START, ( char* )req );
}

int command_main( int argc, char* argv[] )
{
    char* p;
    int   size;

    send_use( SERIAL_DEFAULT_DEVICE );

    while ( 1 ) {
        /* プロンプト表示 */
        send_write( "command> " );

        /* コンソールからの受信文字列を受け取る */
        kz_recv( MSGBOX_ID_CONSINPUT, &size, &p );
        if ( p == NULL ) {
            send_write( "expired.\n" );
            continue;
        }
        p[size] = '\0';

        if ( !strncmp( p, "echo", 4 ) ) { /* echoコマンド */
            /* echoに続く文字列を出力する */
            send_write( p + 4 );
            send_write( "\n" );
        } else if ( !strncmp( p, "timer", 5 ) ) { /* timerコマンド */
            send_write( "timer start.\n" );
            send_start( 1000 );
        } else {
            send_write( "unknown.\n" );
        }

        kz_kmfree( p );
    }

    return 0;
}
