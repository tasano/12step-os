#include "lib.h"
#include "defines.h"
#include "serial.h"

void* memset( void* b, int c, long len )
{
    char* p;
    for ( p = b; 0 < len; len-- ) {
        *( p++ ) = c;
    }

    return b;
}

void* memcpy( void* dst, const void* src, long len )
{
    char*       d = dst;
    const char* s = src;
    for ( ; 0 < len; len-- ) {
        *( d++ ) = *( s++ );
    }

    return dst;
}

int memcmp( const void* b1, const void* b2, long len )
{
    const char* p1 = b1;
    const char* p2 = b2;
    for ( ; 0 < len; len-- ) {
        if ( *p1 != *p2 ) {
            return ( *p1 < *p2 ) ? -1 : 1;
        }
        p1++;
        p2++;
    }

    return 0;
}

int strlen( const char* s )
{
    int len;
    for ( len = 0; *s; s++, len++ )
        ;

    return len;
}

char* strcpy( char* dst, const char* src )
{
    char* d = dst;
    while ( *src ) {
        *( dst++ ) = *( src++ );
    }

    return d;
}

int strcmp( const char* s1, const char* s2 )
{
    while ( *s1 || *s2 ) {
        if ( *s1 != *s2 ) {
            return ( *s1 < *s2 ) ? -1 : 1;
        }
        s1++;
        s2++;
    }

    return 0;
}

int strncmp( const char* s1, const char* s2, int len )
{
    while ( ( *s1 || *s2 ) && ( 0 < len ) ) {
        if ( *s1 != *s2 ) {
            return ( *s1 < *s2 ) ? -1 : 1;
        }
        s1++;
        s2++;
        len--;
    }

    return 0;
}

/* 1文字送信 */
int putc( unsigned char c )
{
    if ( c == '\n' ) {
        serial_send_byte( SERIAL_DEFAULT_DEVICE, '\r' );
    }

    return serial_send_byte( SERIAL_DEFAULT_DEVICE, c );
}

/* 文字列送信 */
int puts( unsigned char* str )
{
    while ( *str ) {
        putc( *( str++ ) );
    }

    return 0;
}

/* 数値の16進表示 */
int putxval( unsigned long value, int column )
{
    char  buf[9];
    char* p;

    p        = buf + sizeof( buf ) - 1;
    *( p-- ) = '\0';

    if ( !value && !column ) {
        column++;
    }

    while ( value || column ) {
        *( p-- ) = "0123456789ABCDEF"[value & 0x0F];
        value >>= 4;
        if ( column ) {
            column--;
        }
    }

    puts( p + 1 );

    return 0;
}
