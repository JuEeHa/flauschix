#include <console.h>
#include <stdint.h>
#include <stdarg.h>

static unsigned char *vram = (unsigned char *)0xb8000;
static uint32_t ret = 0;
static uint8_t x    = 0;
static uint8_t y    = 0;


static void
kputc(const char c)
{
    if ((c == '\n') || (x > 79)) {
        x = 0;
        y++;
    }

    if (y > 24) {
        int i = 0;

        for (; i < (2 * 24 * 80); i++)
            vram[i] = vram[i + 160];
        for (; i < (2 * 25 * 80); i++)
            vram[i] = 0;
    }

    vram[2 * (y * 80 + x)]     = c;
    vram[2 * (y * 80 + x) + 1] = 0x07;

    x++;
    ret++;
}

static void
kputs(const char *s)
{
    while (*s)
        kputc(*s++);
}

static void
kputn(size_t x, const int base)
{
    const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    char buf[65];
    char *p;

    if (base > 36)
        return;

    p  = buf + 64;
    *p = '\0';

    do {
        *--p = digits[x % base];
        x /= base;
    } while (x);

    kputs(p);
}

void
kclrscr(void)
{
    for (int i = 0; i < (2 * 25 * 80); i++)
        vram[i] = 0;

    x = y = 0;
}

uint32_t
kprintf(const char *fmt, ...)
{
    const char *s;
    size_t n;
    va_list ap;

    va_start(ap, fmt);
    ret = 0;

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;

            switch (*fmt) {
                case 's':
                    s = va_arg(ap, char*);
                    kputs(s);
                    break;
                case 'd':
                case 'u':
                    n = va_arg(ap, unsigned long int);
                    kputn(n, 10);
                    break;
                case 'x':
                case 'p':
                    n = va_arg(ap, unsigned long int);
                    kputn(n, 16);
                    break;
                case '%':
                    kputc('%');
                    break;
                case '\0':
                    goto out;
                default:
                    kputc('%');
                    kputc(*fmt);
                    break;
            }
        }
        else
            kputc(*fmt);
        fmt++;
    }

out:
    va_end(ap);

    return ret;
}