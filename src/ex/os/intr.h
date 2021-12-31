#ifndef _INTR_H_INCLUDE_
#define _INTR_H_INCLUDE_

/* ソフトウェア・割込みベクタの定義 */

#define SOFTVEC_TYPE_NUM 4

#define SOFTVEC_TYPE_SOFTERR 0
#define SOFTVEC_TYPE_SYSCALL 1
#define SOFTVEC_TYPE_SERINTR 2
#define SOFTVEC_TYPE_TIMINTR 3

#endif
