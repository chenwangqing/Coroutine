
#include "Coroutine.h"

#include <dlfcn.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/file.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <asm-generic/ioctl.h>
#include <termios.h>
#include <linux/serial.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/tcp.h>
#include <semaphore.h>

typedef ssize_t (*_hook_read_t)(int __fd, void *__buf, size_t __nbytes);
static _hook_read_t _hook_read;

void Coroutine_Hook_Init(void)
{
    _hook_read = (_hook_read_t)dlsym(RTLD_NEXT, "read");
    return;
}
