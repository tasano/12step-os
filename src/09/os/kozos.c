#include "kozos.h"
#include "defines.h"
#include "interrupt.h"
#include "intr.h"
#include "lib.h"
#include "syscall.h"

#define THREAD_NUM           6
#define PRIORITY_NUM         16
#define THREAD_NAME_SIZE     15
#define KZ_THREAD_FLAG_READY ( 1 << 0 )

/* スレッド・コンテキスト */
typedef struct _kz_context {
    uint32 sp; /* スタック・ポインタ */
} kz_context;

/* タスク・コントロール・ブロック(TCB) */
typedef struct _kz_thread {
    struct _kz_thread* next;
    char               name[THREAD_NAME_SIZE + 1]; /* スレッド名 */
    int                priority;                   /* 優先度 */
    char*              stack;                      /* スタック */
    uint32             flags;                      /* 各種フラグ */

    /* スレッドのスタート・アップ(thread_init())に渡すパラメータ */
    struct {
        kz_func_t func; /* スレッドのメイン関数 */
        int       argc; /* スレッドのメイン関数に渡す argc */
        char**    argv; /* スレッドのメイン関数に渡す argv */
    } init;

    /* システム・コール用バッファ */
    struct {
        kz_syscall_type_t   type;
        kz_syscall_param_t* param;
    } syscall;

    /* コンテキスト情報 */
    kz_context context;
} kz_thread;

/* スレッドのレディー・キュー */
static struct {
    kz_thread* head;
    kz_thread* tail;
} readyque[PRIORITY_NUM];

static kz_thread* current;            /* カレント・スレッド */
static kz_thread threads[THREAD_NUM]; /* タスク・コントロール・ブロック */
static kz_handler_t handlers[SOFTVEC_TYPE_NUM]; /* 割込みハンドラ */

void dispatch( kz_context* context );

/* カレント・スレッドをレディー・キューから抜き出す */
static int getcurrent( void )
{
    if ( current == NULL ) {
        return -1;
    }

    /* 既に無い場合は無視 */
    if ( !( current->flags & KZ_THREAD_FLAG_READY ) ) {
        return 1;
    }

    /* カレント・スレッドは必ず先頭にあるはずなので、先頭から抜き出す */
    readyque[current->priority].head = current->next;
    if ( readyque[current->priority].head == NULL ) {
        readyque[current->priority].tail = NULL;
    }
    current->flags &= ~KZ_THREAD_FLAG_READY;
    current->next = NULL;

    return 0;
}

/* カレント・スレッドをレディー・キューに繋げる */
static int putcurrent( void )
{
    if ( current == NULL ) {
        return -1;
    }

    /* 既に有る場合は無視 */
    if ( current->flags & KZ_THREAD_FLAG_READY ) {
        return 1;
    }

    /* レディー・キューの末尾に接続する */
    if ( readyque[current->priority].tail ) {
        readyque[current->priority].tail->next = current;
    } else {
        readyque[current->priority].head = current;
    }
    readyque[current->priority].tail = current;
    current->flags |= KZ_THREAD_FLAG_READY;

    return 0;
}

static void thread_end( void )
{
    kz_exit();
}

/* スレッドのスタート・アップ */
static void thread_init( kz_thread* thp )
{
    /* スレッドのメイン関数を呼び出す */
    thp->init.func( thp->init.argc, thp->init.argv );
    thread_end();
}

/* システム・コールの処理(kz_run():スレッドの起動) */
static kz_thread_id_t thread_run( kz_func_t func, char* name, int priority,
                                  int stacksize, int argc, char* argv[] )
{
    int        i;
    kz_thread* thp;
    uint32*    sp;
    extern char userstack; /* リンカ・スクリプトで定義されるスタック領域 */
    static char* thread_stack = &userstack;

    /* 空いているタスク・コントロール・ブロックを検索 */
    for ( i = 0; i < THREAD_NUM; i++ ) {
        thp = &threads[i];
        if ( !thp->init.func ) {
            /* 見つかった */
            break;
        }
    }
    /* 見つからなかった */
    if ( i == THREAD_NUM ) {
        return -1;
    }

    memset( thp, 0, sizeof( *thp ) );

    /* タスク・コントロール・ブロック(TCB)の設定 */
    strcpy( thp->name, name );
    thp->next      = NULL;
    thp->priority  = priority;
    thp->flags     = 0;
    thp->init.func = func;
    thp->init.argc = argc;
    thp->init.argv = argv;

    /* スタック領域を獲得 */
    memset( thread_stack, 0, stacksize );
    thread_stack += stacksize;

    /* スタックを設定 */
    thp->stack = thread_stack;

    /* スタックの初期化 */
    sp        = ( uint32* )thp->stack;
    *( --sp ) = ( uint32 )thread_end;

    /*
     * プログラム・カウンタを設定する。
     * スレッドの優先度がゼロの場合には、割込み禁止スレッドとする。
     */
    *( --sp ) =
        ( uint32 )thread_init | ( ( uint32 )( priority ? 0 : 0xC0 ) << 24 );
    *( --sp ) = 0; /* ER6 */
    *( --sp ) = 0; /* ER5 */
    *( --sp ) = 0; /* ER4 */
    *( --sp ) = 0; /* ER3 */
    *( --sp ) = 0; /* ER2 */
    *( --sp ) = 0; /* ER1 */

    /* スレッドのスタート・アップ(thread_init())に渡す引数 */
    *( --sp ) = ( uint32 )thp; /* ER0 */

    /* スレッドのコンテキストを設定 */
    thp->context.sp = ( uint32 )sp;

    /* システム・コールを呼び出したスレッドをレディー・キューに戻す */
    putcurrent();

    /* 新規作成したスレッドを、レディー・キューに接続する */
    current = thp;
    putcurrent();

    return ( kz_thread_id_t )current;
}

/* システム・コールの処理(kz_exit():スレッドの終了) */
static int thread_exit( void )
{
    /*
     * 本来ならスタックも開放して再利用できるようにすべきだが省略。
     * このため、スレッドを頻繁に生成・消去するようなことは現状できない。
     */
    puts( current->name );
    puts( " EXIT.\n" );
    memset( current, 0, sizeof( *current ) );
    return 0;
}

/* システム・コールの処理(kz_wait():スレッドの実行権放棄) */
static int thread_wait( void )
{
    putcurrent();
    return 0;
}

/* システム・コールの処理(kz_sleep():スレッドのスリープ) */
static int thread_sleep( void )
{
    return 0;
}

/* システム・コールの処理(kz_wakeup():スレッドのウェイク・アップ) */
static int thread_wakeup( kz_thread_id_t id )
{
    /* ウェイク・アップを呼び出したスレッドをレディー・キューに戻す */
    putcurrent();

    /* 指定されたスレッドをレディー・キューに接続してウェイク・アップする */
    current = ( kz_thread* )id;
    putcurrent();

    return 0;
}

/* システム・コールの処理(kz_getid():スレッドID取得) */
static kz_thread_id_t thread_getid( void )
{
    putcurrent();
    return ( kz_thread_id_t )current;
}

/* システム・コールの処理(kz_chpri():スレッドの優先度変更) */
static int thread_chpri( int priority )
{
    int old = current->priority;
    /* 優先度変更 */
    if ( 0 <= priority ) {
        current->priority = priority;
    }
    /* 新しい優先度のレディー・キューに繋ぎ直す */
    putcurrent();
    return old;
}

/* 割込みハンドラの登録 */
static int setintr( softvec_type_t type, kz_handler_t handler )
{
    static void thread_intr( softvec_type_t type, unsigned long sp );

    /*
     * 割込みを受け付けるために、ソフトウェア・割込みベクタに
     * OSの割込み処理の入口となる関数を登録する。
     */
    softvec_setintr( type, thread_intr );

    /* OS側から呼び出す割込みハンドラを登録 */
    handlers[type] = handler;

    return 0;
}

static void call_functions( kz_syscall_type_t type, kz_syscall_param_t* p )
{
    /* システム・コールの実行中にcurrentが書き換わるので注意 */
    switch ( type ) {
    case KZ_SYSCALL_TYPE_RUN: /* kz_run() */
        p->un.run.ret =
            thread_run( p->un.run.func, p->un.run.name, p->un.run.priority,
                        p->un.run.stacksize, p->un.run.argc, p->un.run.argv );
        break;
    case KZ_SYSCALL_TYPE_EXIT: /* kz_exit() */
        /* TCBが消去されるので、戻り値を書き込んではいけない */
        thread_exit();
        break;
    case KZ_SYSCALL_TYPE_WAIT: /* kz_wait() */
        p->un.wait.ret = thread_wait();
        break;
    case KZ_SYSCALL_TYPE_SLEEP: /* kz_sleep() */
        p->un.sleep.ret = thread_sleep();
        break;
    case KZ_SYSCALL_TYPE_WAKEUP: /* kz_wakeup() */
        p->un.wakeup.ret = thread_wakeup( p->un.wakeup.id );
        break;
    case KZ_SYSCALL_TYPE_GETID: /* kz_getid() */
        p->un.getid.ret = thread_getid();
        break;
    case KZ_SYSCALL_TYPE_CHPRI: /* kz_chpri() */
        p->un.chpri.ret = thread_chpri( p->un.chpri.priority );
        break;
    default:
        break;
    }
}

/* システム・コールの処理 */
static void syscall_proc( kz_syscall_type_t type, kz_syscall_param_t* p )
{
    /*
     * システム・コールを呼び出したスレッドをレディー・キューから
     * 外した状態で処理関数を呼び出す。このためシステム・コールを
     * 呼び出したスレッドをそのまま動作継続させたい場合には、
     * 処理関数の内部で putcurrent() を行う必要がある。
     */
    getcurrent();
    call_functions( type, p );
}

/* スレッドのスケジューリング */
static void schedule( void )
{
    int i;

    /*
     * 優先順位の高い順(優先度の数値の小さい順)にレディー・キューを見て、
     * 動作可能なスレッドを検索する。
     */
    for ( i = 0; i < PRIORITY_NUM; i++ ) {
        /* 見つかった */
        if ( readyque[i].head ) {
            break;
        }
    }
    /* 見つからなかった */
    if ( i == PRIORITY_NUM ) {
        kz_sysdown();
    }

    /* カレント・スレッドに設定する */
    current = readyque[i].head;
}

static void syscall_intr( void )
{
    syscall_proc( current->syscall.type, current->syscall.param );
}

static void softerr_intr( void )
{
    puts( current->name );
    puts( " DOWN.\n" );
    /* レディー・キューから外す */
    getcurrent();
    /* スレッドを終了する */
    thread_exit();
}

/* 割込み処理の入口関数 */
static void thread_intr( softvec_type_t type, unsigned long sp )
{
    /* カレント・スレッドのコンテキストを保存する */
    current->context.sp = sp;

    /*
     * 割込み毎の処理を実行する。
     * SOFTVEC_TYPE_SYSCALL、SOFTVEC_TYPE_SOFTERR の場合は
     * syscall_intr()、softerr_intr() がハンドラに登録されているので、
     * それらが実行される。
     */
    if ( handlers[type] ) {
        handlers[type]();
    }

    /* スレッドのスケジューリング */
    schedule();

    /*
     * スレッドのディスパッチ
     * (dispatch()関数の本体はstartup.sにあり、アセンブラで記述されている)
     */
    dispatch( &current->context );

    /* ここには返ってこない */
}

void kz_start( kz_func_t func, char* name, int priority, int stacksize,
               int argc, char* argv[] )
{
    /*
     * 以降で呼び出すスレッド関連のライブラリ関数の内部で current を
     * 見ている場合があるので、current を NULL に初期化しておく。
     */
    current = NULL;

    memset( readyque, 0, sizeof( readyque ) );
    memset( threads, 0, sizeof( threads ) );
    memset( handlers, 0, sizeof( handlers ) );

    /* 割込みハンドラの登録 */
    setintr( SOFTVEC_TYPE_SYSCALL, syscall_intr ); /* システム・コール */
    setintr( SOFTVEC_TYPE_SOFTERR, softerr_intr ); /* ダウン要因発生 */

    /* システム・コール発行不可なので直接関数を呼び出してスレッドを作成する */
    current =
        ( kz_thread* )thread_run( func, name, priority, stacksize, argc, argv );

    /* 最初のスレッドを起動 */
    dispatch( &current->context );

    /* ここには返ってこない */
}

void kz_sysdown( void )
{
    puts( "system error!\n" );
    while ( 1 )
        ;
}

/* システム・コール呼び出し用ライブラリ関数 */
void kz_syscall( kz_syscall_type_t type, kz_syscall_param_t* param )
{
    current->syscall.type  = type;
    current->syscall.param = param;
    /* トラップ割込み発行 */
    asm volatile( "trapa #0" );
}
