#include "ethernet.h"
#include "kozos.h"
#include "lib.h"
#include "netdrv.h"

#define ETHERNET_HEADER_SIZE 14

struct ethernet_header {
    uint8  dst_addr[MACADDR_SIZE];
    uint8  src_addr[MACADDR_SIZE];
    uint16 type;
};

static unsigned char my_macaddr[MACADDR_SIZE];
static int           initialize = 0;

static int ethernet_recv( struct netbuf* pkt )
{
    struct ethernet_header* ethhdr = ( struct ethernet_header* )pkt->top;

    if ( !initialize ) {
        return 0;
    }

    if ( !( ethhdr->dst_addr[0] & 0x01 ) &&
         memcmp( ethhdr->dst_addr, my_macaddr, MACADDR_SIZE ) ) {
        return 0;
    }

    return 0;
}

static int ethernet_send( struct netbuf* pkt )
{
    return 0;
}

static int ethernet_proc( struct netbuf* buf )
{
    int ret = 0;

    switch ( buf->cmd ) {
    case ETHERNET_CMD_MACADDR:
        memcpy( my_macaddr, buf->option.common.macaddr.addr, MACADDR_SIZE );
        initialize++;
        break;
    case ETHERNET_CMD_RECV:
        ret = ethernet_recv( buf );
        break;
    case ETHERNET_CMD_SEND:
        ret = ethernet_send( buf );
        break;
    default:
        break;
    }

    return ret;
}

int ethernet_main( int argc, char* argv[] )
{
    struct netbuf* buf;
    int            ret;

    buf      = kz_kmalloc( sizeof( *buf ) );
    buf->cmd = NETDRV_CMD_USE;
    kz_send( MSGBOX_ID_NETPROC, 0, ( char* )buf );

    while ( 1 ) {
        kz_recv( MSGBOX_ID_ETHPROC, NULL, ( void* )&buf );
        ret = ethernet_proc( buf );
        if ( !ret ) {
            kz_kmfree( buf );
        }
    }

    return 0;
}
