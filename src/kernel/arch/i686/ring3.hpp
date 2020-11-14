#ifndef __RING3_H__
#define __RING3_H__

namespace kernel {
    extern "C" void callUsermode(const void *ptr);
}

#endif
