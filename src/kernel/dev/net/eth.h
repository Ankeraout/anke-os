#ifndef __KERNEL_DEV_NET_ETH_H__
#define __KERNEL_DEV_NET_ETH_H__

#include <stddef.h>

#include "dev/net/netif.h"

void eth_register(netif_t *netif);
void eth_recv(netif_t *netif, const void *buffer, size_t size);

#endif
