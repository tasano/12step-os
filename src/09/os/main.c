#include "defines.h"
#include "interrupt.h"
#include "kozos.h"
#include "lib.h"

kz_thread_id_t test09_1_id;
kz_thread_id_t test09_2_id;
kz_thread_id_t test09_3_id;

/* システム・タスクとユーザ・スレッドの起動 */
static int start_threads( int argc, char* argv[] )
{
    test09_1_id = kz_run( test09_1_main, "test09_1", 1, 0x100, 0, NULL );
    test09_2_id = kz_run( test09_2_main, "test09_2", 2, 0x100, 0, NULL );
    test09_3_id = kz_run( test09_3_main, "test09_3", 3, 0x100, 0, NULL );

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
