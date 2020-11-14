#ifndef __KERNEL_TTY_TTY_H__
#define __KERNEL_TTY_TTY_H__

#include <stdint.h>

namespace kernel {
    class TTY {
        private:
        int w;
        int h;
        int x;
        int y;
        uint8_t *buf;
        uint8_t attr;
        virtual void scrollup(const int n);

        public:
        TTY(void *buf, int w, int h);
        TTY(void *buf, int w, int h, int x, int y);
        virtual void cls();
        virtual void putc(const char c);
        virtual void puts(const char *s);
        virtual void setAttr(const uint8_t attr);
    };
}

#endif
