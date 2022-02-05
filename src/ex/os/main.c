#include "defines.h"
#include "interrupt.h"
#include "kozos.h"
#include "lib.h"
#include "serial.h"

/* システム・タスクとユーザ・タスクの起動 */
static int start_threads( int argc, char* argv[] )
{
    kz_run( consdrv_main, "consdrv", 1, 0x100, 0, NULL );
    kz_run( command_main, "command", 8, 0x100, 0, NULL );
    kz_run( timerdrv_main, "timerdrv", 2, 0x100, 0, NULL );
#if 0
    kz_run( clock_main, "clock", 9, 0x100, 0, NULL );
#endif
    kz_run( netdrv_main, "netdrv", 10, 0x100, 0, NULL );
    kz_run( ethernet_main, "ethernet", 11, 0x100, 0, NULL );

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

#ifdef SIMULATOR
    extern int bss_start, ebss;

    memset( &bss_start, 0, ( long )&ebss - ( long )&bss_start );

    /* ソフトウェア・割込みベクタを初期化する */
    softvec_init();

    /* シリアルの初期化 */
    serial_init( SERIAL_DEFAULT_DEVICE );
#endif

    puts( "kozos boot succeed!\n" );

    /* OSの動作開始 */
    kz_start( start_threads, "idle", 0, 0x100, 0, NULL );

    /* ここには戻ってこない */

    return 0;
}
