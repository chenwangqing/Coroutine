
#include "Print.h"

#ifdef __linux__
#include <sys/types.h>
#endif

static const char *TO_STR_UPPER = "0123456789ABCDEF";
static const char *TO_STR_LOWER = "0123456789abcdef";

static size_t uint2str(uint64_t val, char *buf, size_t size, uint8_t radix, const char *addition)
{
    char *  p;        /* pointer to traverse string */
    char *  firstdig; /* pointer to first digit */
    char    temp;     /* temp char */
    uint8_t digval;   /* value of digit */

    p        = buf;
    firstdig = p; /* save pointer to first digit */
    size--;

    do {
        digval = (uint8_t)(val % radix);
        val /= radix; /* get next digit */

        /* convert to ascii and store */
        *p++ = addition[digval & 0x0F];
        size--;
    } while (val > 0 && size > 0);

    /* We now have the digit of the number in the buffer, but in reverse  
order. Thus we reverse them now. */

    size_t len = p - buf;
    *p--       = '\0'; /* terminate string; p points to last digit */

    do {
        temp      = *p;
        *p        = *firstdig;
        *firstdig = temp; /* swap *p and *firstdig */
        --p;
        ++firstdig;         /* advance to next two digits */
    } while (firstdig < p); /* repeat until halfway */
    return len;
}

static void Pointer2str(void *val, char *buf, const char *addition)
{
    uint8_t          num  = sizeof(void *);
    static const int test = 0x1122;
    const uint8_t *  p    = (const uint8_t *)&val;
    if (*(uint8_t *)&test == 0x22) {
        // 小端
        for (int i = num - 1; i >= 0; i--) {
            *buf++ = addition[(p[i] >> 4) & 0x0F];
            *buf++ = addition[p[i] & 0x0F];
        }
    } else {
        // 大端
        for (int i = 0; i < num; i++) {
            *buf++ = addition[(p[i] >> 4) & 0x0F];
            *buf++ = addition[p[i] & 0x0F];
        }
    }
    *buf++ = '\0';
    return;
}

#if CFG_PRINT_FLOAT_SUPPORT
static size_t double_integer_2str(double integer, char *buf, size_t size)
{
    size_t idx = 0;
    while (integer >= 1.0 && idx < size) {
        int n      = (int)fmod(integer, 10.0);
        buf[idx++] = '0' + n;
        integer /= 10.0;
    }
    // 反转
    for (int i = 0; i < (idx >> 1); i++) {
        char ch          = buf[i];
        buf[i]           = buf[idx - 1 - i];
        buf[idx - 1 - i] = ch;
    }
    return idx;
}

static size_t double2str(double num, int decimal, char *buf, size_t size)
{
    size--;
    size_t idx        = 0;
    double integer    = 0;
    double fractional = modf(num, &integer);   // 获取小数
    // 整数部分
    idx += double_integer_2str(integer, buf, size);
    buf[idx] = '\0';
    if (idx >= size)
        return idx;
    // 小数部分
    fractional = round(fractional * pow(10.0, decimal));
    if (fractional > 0) {
        // 小数点
        buf[idx++] = '.';
        idx += double_integer_2str(fractional, buf + idx, size - idx);
        for (int i = idx - 1; i > 0 && buf[i] == '0'; i--)
            idx--;
    }
    buf[idx] = '\0';
    return idx;
}

static size_t double_e_2str(double num, int decimal, char *buf, size_t size, bool is_upper)
{
    size--;
    int idx      = 0;
    int exponent = log10(num);
    num /= pow(10.0, exponent);
    int k      = (int)num;
    buf[idx++] = k + '0';
    buf[idx++] = '.';
    num -= k;
    num = round(num * pow(10.0, decimal));
    idx += double_integer_2str(num, buf + idx, size - idx);
    if (size - idx >= 4) {
        buf[idx++] = is_upper ? 'E' : 'e';
        if (exponent < 0) {
            buf[idx++] = '-';
            exponent   = -exponent;
        } else {
            buf[idx++] = '+';
        }
        if (exponent < 10) {
            buf[idx++] = '0';
            buf[idx++] = exponent + '0';
        } else
            idx += uint2str((uint64_t)exponent, buf + idx, size - idx + 1, 10, TO_STR_UPPER);
    }
    buf[idx] = '\0';
    return idx;
}
static size_t double_g_2str(double num, int decimal, char *buf, size_t size)
{
    if (num > 1e6)
        return double_e_2str(num, decimal, buf, size, false);
    return double2str(num, decimal, buf, size);
}
#endif

#if CFG_PRINT_LONG_FLOAT_SUPPORT
static size_t longdouble_integer_2str(long double integer, char *buf, size_t size)
{
    size_t idx = 0;
    while (integer >= 1.0 && idx < size) {
        int n      = (int)fmodl(integer, 10.0);
        buf[idx++] = '0' + n;
        integer /= 10.0;
    }
    // 反转
    for (int i = 0; i < (idx >> 1); i++) {
        char ch          = buf[i];
        buf[i]           = buf[idx - 1 - i];
        buf[idx - 1 - i] = ch;
    }
    return idx;
}

static size_t longdouble2str(long double num, int decimal, char *buf, size_t size)
{
    size--;
    size_t      idx        = 0;
    long double integer    = 0;
    long double fractional = modfl(num, &integer);   // 获取小数
    // 整数部分
    idx += longdouble_integer_2str(integer, buf, size);
    buf[idx] = '\0';
    if (idx >= size)
        return idx;
    // 小数部分
    fractional = roundl(fractional * pow(10.0, decimal));
    if (fractional > 0) {
        // 小数点
        buf[idx++] = '.';
        idx += longdouble_integer_2str(fractional, buf + idx, size - idx);
        for (int i = idx - 1; i > 0 && buf[i] == '0'; i--)
            idx--;
    }
    buf[idx] = '\0';
    return idx;
}

static size_t longdouble_e_2str(long double num, int decimal, char *buf, size_t size, bool is_upper)
{
    size--;
    int idx      = 0;
    int exponent = log10l(num);
    num /= pow(10.0, exponent);
    int k      = (int)num;
    buf[idx++] = k + '0';
    buf[idx++] = '.';
    num -= k;
    num = roundl(num * pow(10.0, decimal));
    idx += longdouble_integer_2str(num, buf + idx, size - idx);
    if (size - idx >= 4) {
        buf[idx++] = is_upper ? 'E' : 'e';
        if (exponent < 0) {
            buf[idx++] = '-';
            exponent   = -exponent;
        } else {
            buf[idx++] = '+';
        }
        if (exponent < 10) {
            buf[idx++] = '0';
            buf[idx++] = exponent + '0';
        } else
            idx += uint2str((uint64_t)exponent, buf + idx, size - idx + 1, 10, TO_STR_UPPER);
    }
    buf[idx] = '\0';
    return idx;
}

static size_t longdouble_g_2str(double num, int decimal, char *buf, size_t size)
{
    if (num > 1e6)
        return longdouble_e_2str(num, decimal, buf, size, false);
    return longdouble2str(num, decimal, buf, size);
}
#endif

// 输出有符号
#define PRINT_INT(Type)                                                             \
    do {                                                                            \
        Type val = va_arg(vArgList, Type);                                          \
        char tmp[64];                                                               \
        if (val >= 0) {                                                             \
            int s = 0;                                                              \
            if (flag[0] == '+' && val > 0) tmp[s++] = '+';                          \
            uint2str((uint64_t)val, tmp + s, sizeof(tmp) - s, 10, TO_STR_LOWER);    \
        } else {                                                                    \
            tmp[0] = '-';                                                           \
            uint2str((uint64_t)(-val), tmp + 1, sizeof(tmp) - 1, 10, TO_STR_LOWER); \
        }                                                                           \
        idx += write(buf, size, tmp, strlen(tmp), flag, flag_num);                  \
    } while (false)
// 输出无符号
#define PRINT_UINT(Type, Radix)                                         \
    do {                                                                \
        Type val = va_arg(vArgList, Type);                              \
        char tmp[64];                                                   \
        uint2str((uint64_t)val, tmp, sizeof(tmp), Radix, TO_STR_LOWER); \
        idx += write(buf, size, tmp, strlen(tmp), flag, flag_num);      \
    } while (false)

#define PROCESS_INTEGER(x)                                           \
    case 'd': {                                                      \
        PRINT_INT(x int);                                            \
        break;                                                       \
    }                                                                \
    case 'o': {                                                      \
        PRINT_UINT(unsigned x int, 8);                               \
        break;                                                       \
    }                                                                \
    case 'u': {                                                      \
        PRINT_UINT(unsigned x int, 10);                              \
        break;                                                       \
    }                                                                \
    case 'x': {                                                      \
        PRINT_UINT(unsigned x int, 16);                              \
        break;                                                       \
    }                                                                \
    case 'X': {                                                      \
        unsigned x int val = va_arg(vArgList, unsigned x int);       \
        char           tmp[64];                                      \
        uint2str((uint64_t)val, tmp, sizeof(tmp), 16, TO_STR_UPPER); \
        idx += write(buf, size, tmp, strlen(tmp), flag, flag_num);   \
        break;                                                       \
    }

#define PROCESS_FLOAT(x, size, ex, Decimal)                              \
    case 'f':                                                            \
    case 'g':                                                            \
    case 'e':                                                            \
    case 'E': {                                                          \
        char     tmp[size];                                              \
        x double val     = va_arg(vArgList, x double);                   \
        int      s       = 0;                                            \
        int      decimal = Decimal;                                      \
        if (flag[0] == '+' && val > 0)                                   \
            tmp[s++] = '+';                                              \
        if (val < 0) {                                                   \
            tmp[s++] = '-';                                              \
            val      = -val;                                             \
        }                                                                \
        if (flag_num_len) {                                              \
            int n = flag_num_len;                                        \
            int i = 0;                                                   \
            while (flag_num[i] != '.')                                   \
                i++;                                                     \
            if (i < n)                                                   \
                decimal = atoi(flag_num + i + 1);                        \
        }                                                                \
        if (val >= ex) ch = 'e';                                         \
        decimal &= 0x0F;                                                 \
        if (isinf(val))                                                  \
            strncpy(tmp, (val > 0 ? "(inf)" : "-(inf)"), sizeof(tmp));   \
        else if (isnan(val))                                             \
            strncpy(tmp, "(nan)", sizeof(tmp));                          \
        else if (ch == 'f')                                              \
            x##double2str(val, decimal, tmp, sizeof(tmp));               \
        else if (ch == 'e' || ch == 'E')                                 \
            x##double_e_2str(val, decimal, tmp, sizeof(tmp), ch == 'E'); \
        else                                                             \
            x##double_g_2str(val, decimal, tmp, sizeof(tmp));            \
        idx += write(buf, size, tmp, strlen(tmp), flag, flag_num);       \
        break;                                                           \
    }

#if CFG_PRINT_FLOAT_SUPPORT
#define PROCESS_DOUBLE() PROCESS_FLOAT(, 64, 1e16, 6)
#else
#define PROCESS_DOUBLE()
#endif

#if CFG_PRINT_LONG_FLOAT_SUPPORT
#define PROCESS_LONG_DOUBLE() PROCESS_FLOAT(long, 80, 1e16, 12)
#elif CFG_PRINT_FLOAT_SUPPORT
#define PROCESS_LONG_DOUBLE() PROCESS_DOUBLE()
#else
#define PROCESS_LONG_DOUBLE()
#endif

#define UNDEFINED(x, ch)                    \
    default: {                              \
        const char str[] = "%" #x;          \
        int        n     = sizeof(str) - 1; \
        memcpy(buf + idx, str, n);          \
        idx += n;                           \
        buf[idx++] = ch;                    \
        buf[idx++] = '\0';                  \
    } break;

// 填充字符
#define PADDING(buf, idx, ch, num)    \
    do {                              \
        for (int i = 0; i < num; i++) \
            buf[idx++] = ch;          \
    } while (false)

static size_t write(char *      buf,
                    size_t      size,
                    const char *value,
                    size_t      len,
                    const char *flag,
                    const char *flag_num)
{
    size_t idx            = 0;
    size_t fmt_min_len    = flag_num ? atoi(flag_num) : 0;                                      // 格式化最小长度
    bool   is_align_right = flag[0] != '-';                                                     // 是否右对齐
    char   pad_char       = flag[0] == '0' || (flag[0] == '+' && flag[1] == '0') ? '0' : ' ';   // 填充字符
    if (len > size) len = size;
    if (fmt_min_len > size) fmt_min_len = size;
    int offset_left  = is_align_right ? fmt_min_len - len : 0;   // 左偏移
    int offset_right = is_align_right ? 0 : fmt_min_len - len;   // 右偏移
    PADDING(buf, idx, pad_char, offset_left);
    memcpy(buf + idx, value, len);
    idx += len;
    PADDING(buf, idx, ' ', offset_right);
    return idx;
}

static size_t print(char *buf, size_t size, const char **format, va_list vArgList)
{
    size_t      idx = 0;
    const char *fmt = *format;
    // 跳过数值
    const char *flag = fmt;
    char        flag_num[32];
    int         flag_num_len     = 0;
    int         dynamic_flag_num = 0;
    while (*fmt && flag_num_len < sizeof(flag_num) - 1) {
        char ch = *fmt;
        if (ch == '-' || ch == '+') {
            fmt++;
            continue;
        }
        if ((ch >= '0' && ch <= '9') || ch == '.') {
            flag_num[flag_num_len++] = ch;
            fmt++;
            continue;
        }
        if (ch == '*') {
            dynamic_flag_num = va_arg(vArgList, int);
            if (dynamic_flag_num > 0)
                flag_num_len += uint2str((uint64_t)dynamic_flag_num,
                                         flag_num + flag_num_len,
                                         sizeof(flag_num) - flag_num_len,
                                         10,
                                         TO_STR_LOWER);
            else
                dynamic_flag_num = 0;
            fmt++;
            continue;
        }
        break;
    }
    flag_num[flag_num_len] = '\0';
    if (*fmt) {
        char ch = *fmt++;
        // 获取值字符串
        switch (ch) {
            PROCESS_INTEGER()
            PROCESS_DOUBLE()
            case '%':
                buf[idx++] = '%';
                break;
            case 'c': {
                char val   = (char)va_arg(vArgList, int);
                buf[idx++] = val;
                break;
            }
            case 'p': {
                char  tmp[32];
                void *val = va_arg(vArgList, void *);
                Pointer2str(val, tmp, TO_STR_LOWER);
                int  len         = strlen(tmp);
                bool is_overflow = len > size - idx;
                if (is_overflow) len = size - idx;
                for (int i = 0; i < len; i++)
                    buf[idx++] = tmp[i];
                break;
            }
            case 'z': {
                ch = *fmt;
                if (ch == '0')
                    break;
                fmt++;
                switch (ch) {
                    case 'u':
                        PRINT_UINT(size_t, 10);
                        break;
                    case 'd':
                        PRINT_INT(ssize_t);
                        break;
                    case 'x':
                        PRINT_UINT(size_t, 16);
                        break;
                        UNDEFINED(z, ch)
                };
                break;
            }
            case 'l': {
                ch = *fmt;
                if (ch == '0')
                    break;
                fmt++;
                switch (ch) {
                    PROCESS_INTEGER(long)
                    PROCESS_LONG_DOUBLE()
                    case 'l': {
                        ch = *fmt;
                        if (ch == '0')
                            break;
                        fmt++;
                        switch (ch) {
                            PROCESS_INTEGER(long long)
                            PROCESS_LONG_DOUBLE()
                            UNDEFINED(ll, ch)
                        };
                        break;
                    }
                        UNDEFINED(l, ch)
                }
                break;
            }
            case 's': {
                size_t      s   = dynamic_flag_num;
                const char *val = va_arg(vArgList, const char *);
                if (s == 0 && val)
                    s = strlen(val);
                idx += write(buf, size, val, s, flag, flag_num);
                break;
            }
                UNDEFINED(, ch)
        }
    }
    *format = fmt;
    return idx;
}

size_t vPrint(char *buf, size_t size, const char *format, va_list vArgList)
{
    if (buf == NULL || size == 0 || format == NULL)
        return 0;
    size--;
    size_t idx = 0;
    while (idx < size) {
        char ch = *format++;
        if (ch == '\0')
            break;
        if (ch == '%')
            idx += print(buf + idx, size - idx, &format, vArgList);
        else
            buf[idx++] = ch;
    }
    buf[idx] = '\0';
    return idx;
}

size_t Print(char *buf, size_t size, const char *format, ...)
{
    va_list vArgList;
    va_start(vArgList, format);
    size_t len = vPrint(buf, size, format, vArgList);
    va_end(vArgList);
    return len;
}
