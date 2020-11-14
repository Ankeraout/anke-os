#include <stdint.h>
#include <stddef.h>

#include "debug.hpp"
#include "libk/libk.hpp"
#include "syscall.hpp"

namespace kernel {
    extern "C" size_t syscall(size_t function, size_t argument) {
        char buffer[9];
        debug("syscall func=0x");
        debug(std::hex32(function, buffer));
        debug(" arg=0x");
        debug(std::hex32(argument, buffer));
        debug("\n");

        size_t result = 0;

        switch(function) {
            case 0: // malloc
                result = (size_t)std::malloc(argument, false);
                break;
        }

        return result;
    }
}
