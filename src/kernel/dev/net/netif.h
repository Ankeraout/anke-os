#ifndef __KERNEL_DEV_NET_NETIF_H__
#define __KERNEL_DEV_NET_NETIF_H__

#include <stddef.h>
#include <stdint.h>

#include "libk/list.h"

typedef enum {
    NETIF_ETHERNET
} netif_type_t;

typedef union {
    uint8_t mac[6];
    
    struct {
        uint8_t addr[4];
        uint8_t mask[4];
    } ipv4;
} netif_addr_t;

struct netif_s;

typedef struct {
    int (*send)(struct netif_s *netif, const void *buffer, size_t size);
    int (*getLinkAddress)(struct netif_s *netif, netif_addr_t *linkAddress);
    int (*setLinkAddress)(struct netif_s *netif, const netif_addr_t *linkAddress);
} netif_api_t;

typedef struct netif_s {
    netif_api_t api;
    netif_type_t type;
    netif_addr_t link_addr;
    netif_addr_t net_addr;
} netif_t;

void netif_register(netif_t *netif);

#endif
