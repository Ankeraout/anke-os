#ifndef __KERNEL_DRIVER_ETH_HPP__
#define __KERNEL_DRIVER_ETH_HPP__

#include <stdint.h>

#include "driver/net.hpp"

namespace kernel {
    class EthernetDevice : public NetworkDevice {
        public:
        EthernetDevice();
        virtual ~EthernetDevice();
        virtual networkAddress_t *getMACAddress(networkAddress_t *networkAddress) = 0;
    };
}

#endif
