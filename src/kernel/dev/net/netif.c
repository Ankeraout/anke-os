#include "dev/net/netif.h"
#include "libk/stdio.h"

void netif_register(netif_t *netif);

void netif_register(netif_t *netif) {
    if(netif->type == NETIF_ETHERNET) {
        printf("netif: new Ethernet card with MAC address ");

        for(int i = 0; i < 6; i++) {
            printf("%02x", netif->link_addr.mac[i]);

            if(i < 5) {
                printf(":");
            }
        }

        printf("\n");
    }
}