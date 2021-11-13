#include "elf.h"
#include "defines.h"
#include "lib.h"

struct elf_header {
    struct {
        uint8 magic[4];    /* マジック・ナンバ */
        uint8 class;       /* 32/64ビットの区別 */
        uint8 format;      /* エンディアン情報 */
        uint8 version;     /* ELFフォーマットのバージョン */
        uint8 abi;         /* OSの種別 */
        uint8 abi_version; /* OSのバージョン */
        uint8 reserve[7];  /* 予約(未使用) */
    } id;
    short type;                 /* ファイルの種別 */
    short arch;                 /* CPUの種類 */
    long  version;              /* ELF形式のバージョン */
    long  entry_point;          /* 実行開始アドレス */
    long program_header_offset; /* プログラム・ヘッダ・テーブルの位置 */
    long section_header_offset; /* セクション・ヘッダ・テーブルの位置 */
    long  flags;                /* 各種フラグ */
    short header_size;          /* ELFヘッダのサイズ */
    short program_header_size; /* プログラム・ヘッダのサイズ */
    short program_header_num;  /* プログラム・ヘッダの個数 */
    short section_header_size; /* セクション・ヘッダのサイズ */
    short section_header_num;  /* セクション・ヘッダの個数 */
    short section_name_index; /* セクション名を格納するセクション */
};

struct elf_program_header {
    long type;          /* セグメントの種別 */
    long offset;        /* ファイル中の位置 */
    long virtual_addr;  /* 論理アドレス(VA) */
    long physical_addr; /* 物理アドレス(PA) */
    long file_size;     /* ファイル中のサイズ */
    long memory_size;   /* メモリ上でのサイズ */
    long flags;         /* 各種フラグ */
    long align;         /* アライメント */
};

/* ELFヘッダのチェック */
static int elf_check( struct elf_header* header )
{
    /* マジック・ナンバのチェック */
    if ( memcmp( header->id.magic,
                 "\x7F"
                 "ELF",
                 4 ) ) {
        return -1;
    }

    /* ELF32 */
    if ( header->id.class != 1 ) {
        return -1;
    }
    /* Big endian */
    if ( header->id.format != 2 ) {
        return -1;
    }
    /* Version 1 */
    if ( header->id.version != 1 ) {
        return -1;
    }
    /* Executable file */
    if ( header->type != 2 ) {
        return -1;
    }
    /* Version 1 */
    if ( header->version != 1 ) {
        return -1;
    }

    /* Hitachi H8/300 or H8/300H */
    if ( ( header->arch != 46 ) && ( header->arch != 47 ) ) {
        return -1;
    }

    return 0;
}

/* セグメント単位でのロード */
static int elf_load_program( struct elf_header* header )
{
    int i;

    for ( i = 0; i < header->program_header_num; i++ ) {
        /* プログラム・ヘッダを取得 */
        struct elf_program_header* phdr =
            ( struct elf_program_header* )( ( char* )header +
                                            header->program_header_offset +
                                            header->program_header_size * i );

        /* ロード可能なセグメントか？ */
        if ( phdr->type != 1 ) {
            continue;
        }

        /* とりあえず実験用に、実際にロードせずにセグメント情報を表示する */
        putxval( phdr->offset, 6 );
        puts( " " );
        putxval( phdr->virtual_addr, 8 );
        puts( " " );
        putxval( phdr->physical_addr, 8 );
        puts( " " );
        putxval( phdr->file_size, 5 );
        puts( " " );
        putxval( phdr->memory_size, 5 );
        puts( " " );
        putxval( phdr->flags, 2 );
        puts( " " );
        putxval( phdr->align, 2 );
        puts( "\n" );
    }

    return 0;
}

int elf_load( char* buf )
{
    struct elf_header* header = ( struct elf_header* )buf;

    /* ELFヘッダのチェック */
    if ( elf_check( header ) < 0 ) {
        return -1;
    }

    /* セグメント単位でのロード */
    if ( elf_load_program( header ) < 0 ) {
        return -1;
    }

    return 0;
}
