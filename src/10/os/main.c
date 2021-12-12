#include "defines.h"
#include "interrupt.h"
#include "kozos.h"
#include "lib.h"

/* システム・タスクとユーザ・スレッドの起動 */
static int start_threads( int argc, char* argv[] )
{
    kz_run( test10_1_main, "test10_1", 1, 0x100, 0, NULL );

    /* 優先順位を下げて、アイドルスレッドに移行する */
    kz_chpri( 15 );

    /* 割込み有効にする */
    INTR_ENABLE;

    while ( 1 ) {
        /* 省電力モードに移行 */
        asm volatile( "sleep" );
    }

    return 0;
}

int main( void )
{
    /* 割込み無効にする */
    INTR_DISABLE;

    puts( "kozos boot succeed!\n" );

    /* OSの動作開始 */
    kz_start( start_threads, "idle", 0, 0x100, 0, NULL );

    /* ここには戻ってこない */

    return 0;
}
