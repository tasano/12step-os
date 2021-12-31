#ifndef _MEMORY_H_INCLUDE_
#define _MEMORY_H_INCLUDE_

int   kzmem_init( void );      /* 動的メモリの初期化 */
void* kzmem_alloc( int size ); /* 動的メモリの獲得 */
void  kzmem_free( void* mem ); /* メモリの解放 */

#endif
