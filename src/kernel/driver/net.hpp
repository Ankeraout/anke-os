#ifndef __KERNEL_DRIVER_NET_HPP__
#define __KERNEL_DRIVER_NET_HPP__

#include <stddef.h>
#include <stdint.h>

namespace kernel {
    typedef enum {
        NETADDR_MAC,
        NETADDR_IPV4,
        NETADDR_IPV6
    } networkAddressType_t;

    typedef struct {
        networkAddressType_t type;
        union {
            uint8_t mac[6];
            uint8_t ipv4[4];
            uint8_t ipv6[16];
        } addr;
    } networkAddress_t;

    class NetworkDevice {
        public:
        virtual void sendFrame(const void *buffer, const size_t length) = 0;
        virtual void receiveFrame(const void *buffer, const size_t length) = 0;
    };
}

#endif
