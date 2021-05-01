#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel/libk/string.h"

#include "kernel/sys/dev/tty.h"

int puts(const char *str);

int puts(const char *str) {
    tty_driver_t *tty_driver = tty_getDefault();

    if(tty_driver != NULL) {
        tty_driver->write(tty_driver, str, strlen(str));
    }

    return 0;
}

#define IS_IDENTIFIER(c) (c == '%' || c == 'd' || c == 'i' || c == 'o' || c == 'x' || c == 'X' || c == 'u' || c == 'c' || c == 's' || c == 'f' || c == 'e' || c == 'E' || c == 'g' || c == 'G' || c == 'p' || c == 'n')

typedef enum {
    STATE_NORMAL,
    STATE_PERCENT,
    STATE_MINFW,
    STATE_PERIOD,
    STATE_MAXFW,
    STATE_IDENTIFIER_L,
    STATE_IDENTIFIER_LL
} sprintf_parser_state_t;

int sprintf(char *s, const char *format, ...);
int vsprintf(char *s, const char *format, va_list arguments);
int printf(const char *format, ...);

int sprintf(char *s, const char *format, ...) {
    va_list arguments;
    va_start(arguments, format);

    int returnValue = vsprintf(s, format, arguments);

    va_end(arguments);

    return returnValue;
}

int vsprintf(char *s, const char *format, va_list arguments) {
    char c = *format;
    bool flag_llu;
    bool flag_identifier = false;
    bool flag_leftJustify;
    bool flag_pad0;
    bool flag_alwaysSign;
    bool flag_blank;
    bool flag_sharp;
    bool flag_error;
    int minfw;
    int maxfw;
    int index = 0;
    sprintf_parser_state_t parserState = STATE_NORMAL;

    // Temporary: removes warnings
    (void)flag_alwaysSign;
    (void)flag_leftJustify;

    while(c) {
        switch(parserState) {
            case STATE_NORMAL:
                if(c == '%') {
                    parserState = STATE_PERCENT;
                    flag_llu = false;
                    flag_identifier = false;
                    flag_leftJustify = false;
                    flag_pad0 = false;
                    flag_alwaysSign = false;
                    flag_blank = false;
                    flag_sharp = false;
                    flag_error = false;
                    minfw = -1;
                    maxfw = -1;
                } else {
                    s[index++] = c;
                }

                break;

            case STATE_PERCENT:
                if(c == '-') {
                    flag_leftJustify = true;
                } else if(c == '0') {
                    flag_pad0 = true;
                } else if(c == '+') {
                    flag_alwaysSign = true;
                } else if(c == ' ') {
                    flag_blank = true;
                } else if(c == '#') {
                    flag_sharp = true;
                } else if((c >= '1') && (c <= '9')) {
                    parserState = STATE_MINFW;
                } else if(c == 'l') {
                    parserState = STATE_IDENTIFIER_L;
                } else if(IS_IDENTIFIER(c)) {
                    flag_identifier = true;
                    parserState = STATE_NORMAL;
                } else if(c == '.') {
                    parserState = STATE_PERIOD;
                } else {
                    flag_error = true;
                }

                break;

            case STATE_MINFW:
                if((c >= '0') && (c <= '9')) {

                } else if(c == '.') {
                    parserState = STATE_PERIOD;
                } else if(c == 'l') {
                    parserState = STATE_IDENTIFIER_L;
                } else if(IS_IDENTIFIER(c)) {
                    flag_identifier = true;
                    parserState = STATE_NORMAL;
                } else {
                    flag_error = true;
                }

                break;

            case STATE_PERIOD:
                if((c >= '0') && (c <= '9')) {
                    parserState = STATE_MAXFW;
                } else if(c == 'l') {
                    parserState = STATE_IDENTIFIER_L;
                } else if(IS_IDENTIFIER(c)) {
                    flag_identifier = true;
                    parserState = STATE_NORMAL;
                } else {
                    flag_error = true;
                }

                break;

            case STATE_MAXFW:
                if((c >= '0') && (c <= '9')) {

                } else if(c == 'l') {
                    parserState = STATE_IDENTIFIER_L;
                } else if(IS_IDENTIFIER(c)) {
                    flag_identifier = true;
                    parserState = STATE_NORMAL;
                } else {
                    flag_error = true;
                }

                break;

            case STATE_IDENTIFIER_L:
                if(c == 'l') {
                    parserState = STATE_IDENTIFIER_LL;
                } else {
                    flag_error = true;
                }

                break;

            case STATE_IDENTIFIER_LL:
                if(c == 'u') {
                    flag_identifier = true;
                    flag_llu = true;
                    parserState = STATE_IDENTIFIER_LL;
                } else {
                    flag_error = true;
                }

                break;
        }

        if(flag_error) {
            // TODO: what to do here?
        }

        if(parserState == STATE_MINFW) {
            if(minfw == -1) {
                minfw = 0;
            }

            minfw *= 10;
            minfw += c - '0';
        }

        if(parserState == STATE_MAXFW) {
            if(maxfw == -1) {
                maxfw = 0;
            }

            maxfw *= 10;
            maxfw += c - '0';
        }

        if(flag_identifier) {
            char identifier = c;

            if(flag_llu) {

            } else if(identifier == 'c') {
                s[index++] = (char)va_arg(arguments, int);
            } else if(identifier == 's') {
                const char *string = va_arg(arguments, const char *);
                size_t stringLength = strlen(string);

                if((int)stringLength < minfw) {
                    for(size_t i = 0; i < (minfw - stringLength); i++) {
                        if(flag_pad0) {
                            s[index++] = '0';
                        } else {
                            s[index++] = ' ';
                        }
                    }

                    strcpy(&s[index], string);
                } else {
                    size_t bytesToCopy;

                    if(maxfw == -1) {
                        bytesToCopy = stringLength;
                    } else {
                        bytesToCopy = ((int)stringLength > maxfw) ? (size_t)maxfw : stringLength;
                    }

                    memcpy(&s[index], string, bytesToCopy);
                    index += bytesToCopy;
                }
            } else if(identifier == 'x' || identifier == 'X') {
                if(flag_sharp) {
                    s[index++] = '0';

                    if(identifier == 'x') {
                        s[index++] = 'x';
                    } else {
                        s[index++] = 'X';
                    }
                }

                unsigned int value = va_arg(arguments, unsigned int);
                char buffer[9];

                for(int i = 0; i < 8; i++) {
                    uint32_t digit = value >> 28;
                    value <<= 4;

                    if(digit < 10) {
                        buffer[i] = '0' + digit;
                    } else {
                        if(identifier == 'x') {
                            buffer[i] = 'a' + digit - 10;
                        } else {
                            buffer[i] = 'A' + digit - 10;
                        }
                    }
                }

                buffer[8] = '\0';

                int trailingZeros = 0;

                while(buffer[trailingZeros] == '0') {
                    trailingZeros++;
                }

                if(trailingZeros == 8) {
                    trailingZeros--;
                }

                size_t stringLength = 8 - trailingZeros;
                
                if((int)stringLength < minfw) {
                    for(size_t i = 0; i < (minfw - stringLength); i++) {
                        if(flag_pad0) {
                            s[index++] = '0';
                        } else {
                            s[index++] = ' ';
                        }
                    }

                    strcpy(&s[index], buffer + trailingZeros);

                    index += stringLength;
                } else {
                    size_t bytesToCopy;

                    if(maxfw == -1) {
                        bytesToCopy = stringLength;
                    } else {
                        bytesToCopy = ((int)stringLength > maxfw) ? (size_t)maxfw : stringLength;
                    }
                    
                    memcpy(&s[index], buffer + trailingZeros, bytesToCopy);
                    index += bytesToCopy;
                }
            } else if(identifier == 'd' || identifier == 'i') {
                int arg = va_arg(arguments, int);

                char buffer[11];
                int bufferLength = 0;

                bool negative = arg < 0;

                if(negative) {
                    arg = -arg;
                }

                if(arg == 0) {
                    buffer[0] = '0';
                    bufferLength = 1;
                } else if(arg == INT32_MIN) {
                    memcpy(buffer, "8463847412", 10);
                    bufferLength = 10;
                } else {
                    while(arg > 0) {
                        buffer[bufferLength++] = '0' + (arg % 10);
                        arg /= 10;
                    }
                }

                if(flag_pad0) {
                    while(bufferLength < minfw) {
                        buffer[bufferLength++] = '0';
                    }

                    if(negative) {
                        buffer[bufferLength - 1] = '-';
                    } else if(flag_blank) {
                        buffer[bufferLength - 1] = ' ';
                    }
                } else {
                    if(negative) {
                        buffer[bufferLength++] = '-';
                    }

                    while(bufferLength < minfw) {
                        buffer[bufferLength++] = ' ';
                    }
                }

                buffer[bufferLength] = '\0';

                strrev(buffer);

                memcpy(&s[index], buffer, bufferLength);

                index += bufferLength;
            } else if(identifier == '%') {
                s[index++] = '%';
            }

            flag_identifier = false;
        }

        format++;
        c = *format;
    }

    s[index] = '\0';

    return index;
}

int printf(const char *format, ...) {
    char buffer[4096];

    va_list arguments;
    va_start(arguments, format);

    int returnValue = vsprintf(buffer, format, arguments);

    va_end(arguments);

    puts(buffer);

    return returnValue;
}
