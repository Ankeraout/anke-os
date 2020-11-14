#include "libk/libk.hpp"
#include "tty/tty.hpp"

namespace kernel {
    TTY::TTY(void *buf, int w, int h) {
        this->buf = (uint8_t *)buf;
        this->w = w;
        this->h = h;
        this->x = 0;
        this->y = 0;
    }

    TTY::TTY(void *buf, int w, int h, int x, int y) {
        this->buf = (uint8_t *)buf;
        this->w = w;
        this->h = h;
        this->x = x;
        this->y = y;
    }

    void TTY::cls() {
        std::memset(this->buf, 0, this->w * this->h * 2);

        this->x = 0;
        this->y = 0;
        this->attr = 0x07;
    }

    void TTY::scrollup(const int n) {
        std::memcpy(this->buf, this->buf + 2 * this->w * n, this->w * (this->h - n) * 2);
        std::memset(this->buf + 2 * this->w * (this->h - n), 0, this->w * n * 2);

        this->y -= n;

        if(this->y < 0) {
            this->y = 0;
        }
    }

    void TTY::putc(const char c) {
        if(c == 9) {
            this->x += 4 - (this->x % 4);
        } else if(c == '\n') {
            this->x = 0;
            this->y++;
        } else if(c == '\r') {
            this->x = 0;
        } else {
            this->buf[(this->y * this->w + this->x) * 2] = c;
            this->buf[(this->y * this->w + this->x) * 2 + 1] = this->attr;
            this->x++;
        }

        if(this->x >= this->w) {
            this->x = 0;
            this->y++;
        }

        if(this->y >= this->h) {
            this->scrollup(this->y - this->h + 1);
        }
    }

    void TTY::puts(const char *s) {
        while(*s) {
            this->putc(*s++);
        }
    }
    
    void TTY::setAttr(const uint8_t attr) {
        this->attr = attr;
    }
}
