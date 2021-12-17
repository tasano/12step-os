#include "defines.h"
#include "elf.h"
#include "interrupt.h"
#include "lib.h"
#include "serial.h"
#include "xmodem.h"

static int init( void )
{
    /* 以下はリンカ・スクリプトで定義してあるシンボル */
    extern int erodata, data_start, edata, bss_start, ebss;

    /*
     * データ領域とBSS領域を初期化する。この処理以降でないと、
     * グローバル変数が初期化されていないので注意。
     */
    memcpy( &data_start, &erodata, ( long )&edata - ( long )&data_start );
    memset( &bss_start, 0, ( long )&ebss - ( long )&bss_start );

    /* ソフトウェア・割込みベクタを初期化する */
    softvec_init();

    /* シリアルの初期化 */
    serial_init( SERIAL_DEFAULT_DEVICE );

    return 0;
}

/* メモリの16進ダンプ出力 */
static int dump( char* buf, long size )
{
    long i;

    if ( size < 0 ) {
        puts( "no data.\n" );
        return -1;
    }

    for ( i = 0; i < size; i++ ) {
        putxval( buf[i], 2 );
        if ( ( i & 0x0F ) == 15 ) {
            puts( "\n" );
        } else {
            if ( ( i & 0x0F ) == 7 ) {
                puts( " " );
            }
            puts( " " );
        }
    }
    puts( "\n" );

    return 0;
}

static void wait( void )
{
    volatile long i;
    for ( i = 0; i < 300000; i++ )
        ;
}

int main( void )
{
    static char           buf[16];
    static long           size    = -1;
    static unsigned char* loadbuf = NULL;
    char*                 entry_point;
    void ( *f )( void );
    extern int buffer_start; /* リンカ・スクリプトで定義されているバッファ */

    /* 割込み無効にする */
    INTR_DISABLE;

    init();
    puts( "kzload (kozos boot loader) started.\n" );

    while ( 1 ) {
        puts( "kzload> " ); /* プロンプト表示 */
        gets( buf );        /* シリアルからのコマンド受信 */

        /* XMODEMでのファイルのダウンロード */
        if ( !strcmp( buf, "load" ) ) {
            loadbuf = ( char* )&buffer_start;
            size    = xmodem_recv( loadbuf );
            /* 転送アプリが終了し端末アプリに制御が戻るまで待ち合わせる */
            wait();
            if ( size < 0 ) {
                puts( "\nXMODEM receive error!\n" );
            } else {
                puts( "\nXMODEM receive succeeded.\n" );
            }
        }
        /* メモリの16進ダンプ出力 */
        else if ( !strcmp( buf, "dump" ) ) {
            puts( "size: " );
            putxval( size, 0 );
            puts( "\n" );
            dump( loadbuf, size );
        }
        /* ELF形式ファイルの実行 */
        else if ( !strcmp( buf, "run" ) ) {
            /* メモリ上に展開(ロード) */
            entry_point = elf_load( loadbuf );
            if ( !entry_point ) {
                puts( "run error!\n" );
            } else {
                puts( "starting from entry point: " );
                putxval( ( unsigned long )entry_point, 0 );
                puts( "\n" );
                /* ここで、ロードしたプログラムに処理を渡す */
                f = ( void ( * )( void ) )entry_point;
                f();
                /* ここには返ってこない */
            }
        }
        /* 不明 */
        else {
            puts( "unknown.\n" );
        }
    }

    return 0;
}
