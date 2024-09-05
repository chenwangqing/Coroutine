#include <stdio.h>
#include <stdlib.h>
#include "libTest.h"

void say_hello()
{
    char *buf = malloc(1024);
    if (NULL != buf) {
        snprintf(buf, 1024, "hello.\n");
        printf("%s", buf);
    }
    free(buf);
    return;
}