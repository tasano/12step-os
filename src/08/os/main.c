#include "defines.h"
#include "interrupt.h"
#include "kozos.h"
#include "lib.h"

/* システム・タスクとユーザ・スレッドの起動 */
static int start_threads( int argc, char* argv[] )
{
    kz_run( test08_1_main, "command", 0x100, 0, NULL );
    return 0;
}

int main( void )
{
    /* 割込み無効にする */
    INTR_DISABLE;

    puts( "kozos boot succeed!\n" );

    /* OSの動作開始 */
    kz_start( start_threads, "start", 0x100, 0, NULL );

    /* ここには戻ってこない */

    return 0;
}
