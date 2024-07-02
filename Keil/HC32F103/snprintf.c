
#include "snprintf.h"

size_t ff_vsnprintf(char* buf, size_t maxlen, const char* format, va_list val)
{
    if (maxlen == 0 || buf == NULL)
        return 0;
    int rs = vsnprintf(buf, maxlen, format, val);
    if (rs < 0)
        rs = 0;
    maxlen--;
    if ((size_t)rs > maxlen)
        rs = maxlen;
    return rs;
}

size_t ff_snprintf(char* buf, size_t maxlen, const char* format, ...)
{
    va_list vArgList;
    va_start(vArgList, format);
    size_t rs = ff_vsnprintf(buf, maxlen, format, vArgList);
    va_end(vArgList);
    return rs;
}
