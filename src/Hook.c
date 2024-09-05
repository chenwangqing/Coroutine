
#include "Hook.h"
#include <link.h>
#include <string.h>
#include <stdlib.h>
#include <fnmatch.h>
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <setjmp.h>
#include <signal.h>

#define PAGE_SHIFT       12
#define PAGE_SIZE        (1UL << PAGE_SHIFT)
#define PAGE_MASK        (~(PAGE_SIZE - 1))
#define PAGE_START(addr) ((addr)&PAGE_MASK)
#define PAGE_END(addr)   (PAGE_START(addr + sizeof(uintptr_t) - 1) + PAGE_SIZE)
#define PAGE_COVER(addr) (PAGE_END(addr) - PAGE_START(addr))
const int               POINTER_LENGTH       = sizeof(void *);
static volatile int     xh_core_sigsegv_flag = 0;
static sigjmp_buf       xh_core_sigsegv_env;
static struct sigaction xh_core_sigsegv_act_old;

typedef struct Symbol
{
    char *         name;
    size_t         offset;
    struct Symbol *next;
} Symbol_t;

typedef struct DLLNode
{
    char *          name;
    size_t          address;
    Symbol_t *      symbol;
    struct DLLNode *next;
} DLLNode_t;

static DLLNode_t *dll_list = NULL;

static void _sigsegv_handler(int sig)
{
    (void)sig;
    if (xh_core_sigsegv_flag)
        siglongjmp(xh_core_sigsegv_env, 1);
    else
        sigaction(SIGSEGV, &xh_core_sigsegv_act_old, NULL);
}

static int _add_sigsegv_handler()
{
    struct sigaction act;
    if (0 != sigemptyset(&act.sa_mask))
        return (0 == errno ? -1 : errno);
    act.sa_handler = _sigsegv_handler;
    if (0 != sigaction(SIGSEGV, &act, &xh_core_sigsegv_act_old))
        return (0 == errno ? -1 : errno);
    return 0;
}

static void _del_sigsegv_handler()
{
    sigaction(SIGSEGV, &xh_core_sigsegv_act_old, NULL);
    return;
}

static int callback(struct dl_phdr_info *info, size_t size, void *data)
{
    int s = strlen(info->dlpi_name);
    if (s && info->dlpi_name[0] == '/') {
        printf("Name: %s Addr: %llX\n", info->dlpi_name, (uint64_t)info->dlpi_addr);
        DLLNode_t *n = (DLLNode_t *)malloc(sizeof(DLLNode_t) + s + 1);
        n->name      = (char *)(n + 1);
        n->address   = info->dlpi_addr;
        n->next      = dll_list;
        dll_list     = n;
        memcpy(n->name, info->dlpi_name, s);
        n->name[s] = '\0';
    }
    return 0;
}

void Hook_Init(void)
{
    dl_iterate_phdr(callback, NULL);
    // 忽略一些不能替换的动态库
    Hook_Ignore("*libpthread*");
    Hook_Ignore("*libdl*");
    Hook_Ignore("*libstdc++*");
    Hook_Ignore("*libm*");
    Hook_Ignore("*libc*");
    Hook_Ignore("*libgcc*");
    Hook_Ignore("*ld-linux*");
    Hook_Ignore("linux-vdso.so*");
    return;
}

void Hook_Ignore(const char *exp)
{
    if (exp == NULL || *exp == '\0') return;
    DLLNode_t *p  = dll_list;
    DLLNode_t *up = p;
    while (p) {
        int ret = fnmatch(exp, p->name, 0);
        if (ret == 0) {
            DLLNode_t *t = p;
            if (p == up)
                dll_list = t->next;
            p        = p->next;
            up->next = t->next;
            free(t);
            continue;
        }
        up = p;
        p  = p->next;
    }
    return;
}

static bool LoadPLTInfo(DLLNode_t *n)
{
    printf("File Name: %s\n", n->name);
    int fd = open(n->name, O_RDONLY);
    if (fd == -1) return false;
#if POINTER_LENGTH == 4
    typedef Elf32_Ehdr Elf_Ehdr;
    typedef Elf32_Shdr Elf_Shdr;
    typedef Elf32_Rel  Elf_Rel;
    typedef Elf32_Sym  Elf_Sym;
#else
    typedef Elf64_Ehdr Elf_Ehdr;
    typedef Elf64_Shdr Elf_Shdr;
    typedef Elf64_Rel  Elf_Rel;
    typedef Elf64_Sym  Elf_Sym;
#endif
    Elf_Shdr *shdr     = NULL;
    char *    str_rela = NULL;
    char *    shstrtab = NULL;
    bool      isOk     = false;
    do {
        // 读取 ELF 文件头
        Elf_Ehdr ehdr;
        if (read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr))
            break;
        // 解析section 分配内存 section * 数量
        shdr = (Elf64_Shdr *)malloc(sizeof(Elf_Shdr) * ehdr.e_shnum);
        // 设置fp偏移量 offset，e_shoff含义
        lseek(fd, ehdr.e_shoff, SEEK_SET);
        // 读取section 到 shdr, 大小为shdr * 数量
        if (read(fd, shdr, sizeof(Elf_Shdr) * ehdr.e_shnum) != sizeof(Elf_Shdr) * ehdr.e_shnum)
            break;
        // 第e_shstrndx项是字符串表 定义 字节 长度 char类型 数组
        str_rela           = (char *)malloc(shdr[ehdr.e_shstrndx].sh_size);
        uint32_t sh_offset = shdr[ehdr.e_shstrndx].sh_offset;
        lseek(fd, sh_offset, SEEK_SET);
        if (read(fd, str_rela, shdr[ehdr.e_shstrndx].sh_size) != shdr[ehdr.e_shstrndx].sh_size)
            break;
        int IDX_SHT_STRTAB = -1;
        int IDX_SHT_RELA   = -1;
        int IDX_SHT_SYMTAB = -1;
        for (int i = 0; i < ehdr.e_shnum; i++) {
            char *name = str_rela + shdr[i].sh_name;
            if (shdr[i].sh_type == SHT_STRTAB && strcmp(name, ".dynstr") == 0 && IDX_SHT_STRTAB < 0) {
                IDX_SHT_STRTAB = i;   // 符号表
                shstrtab       = (char *)malloc(shdr[i].sh_size);
                lseek(fd, shdr[i].sh_offset, SEEK_SET);
                if (read(fd, shstrtab, shdr[i].sh_size) != shdr[i].sh_size)
                    break;
            } else if (shdr[i].sh_type == SHT_RELA && strcmp(name, ".rela.plt") == 0 && IDX_SHT_RELA < 0)
                IDX_SHT_RELA = i;   // .rela.plt
            else if (shdr[i].sh_type == SHT_DYNSYM && IDX_SHT_SYMTAB < 0)
                IDX_SHT_SYMTAB = i;
        }
        if (IDX_SHT_SYMTAB < 0 || IDX_SHT_STRTAB < 0 || IDX_SHT_RELA < 0)
            break;
        size_t addr_rel = shdr[IDX_SHT_RELA].sh_offset;
        size_t addr_sym = shdr[IDX_SHT_SYMTAB].sh_offset;
        int    cnt_sym  = shdr[IDX_SHT_SYMTAB].sh_size / shdr[IDX_SHT_SYMTAB].sh_entsize;
        int    cnt_rel  = shdr[IDX_SHT_RELA].sh_size / shdr[IDX_SHT_RELA].sh_entsize;
        int    idx_rel  = 0;
        for (int i = 0; i < cnt_sym && idx_rel < cnt_rel; i++) {
            Elf_Sym sym;
            lseek(fd, addr_sym + i * shdr[IDX_SHT_SYMTAB].sh_entsize, SEEK_SET);
            if (read(fd, &sym, sizeof(Elf_Sym)) != sizeof(Elf_Sym))
                break;
            if (ELF32_ST_TYPE(sym.st_info) != STT_FUNC) continue;
            Elf_Rel rel;
            lseek(fd, addr_rel + idx_rel * shdr[IDX_SHT_RELA].sh_entsize, SEEK_SET);
            if (read(fd, &rel, sizeof(Elf_Rel)) != sizeof(Elf_Rel))
                break;
            char *symbol = shstrtab + sym.st_name;
            printf("Relocation entry %d:", i);
            printf("  Offset: 0x%x", rel.r_offset);
            printf("  Symbol: %s\n", symbol);
            int   len = strlen(symbol);
            char *ptr = strstr(symbol, "@");
            if (ptr) len = ptr - symbol;
            Symbol_t *s = (Symbol_t *)malloc(sizeof(Symbol_t) + len + 1);
            s->name     = (char *)(s + 1);
            s->next     = n->symbol;
            n->symbol   = s;
            s->offset   = rel.r_offset;
            memcpy(s->name, symbol, len);
            s->name[len] = '\0';
            idx_rel++;
        }
        isOk = true;
    } while (false);
    close(fd);
    if (shdr) free(shdr);
    if (shstrtab) free(shstrtab);
    if (str_rela) free(str_rela);
    return isOk;
}

void Hook_ReadyRegister(void)
{
    DLLNode_t *p  = dll_list;
    DLLNode_t *up = p;
    while (p) {
        if (!LoadPLTInfo(p)) {
            DLLNode_t *t = p;
            if (p == up)
                dll_list = t->next;
            p        = p->next;
            up->next = t->next;
            free(t);
            continue;
        }
        up = p;
        p  = p->next;
    }
    return;
}

static bool _Hook_Register(DLLNode_t *n, const char *name, void *func)
{
    Symbol_t *s      = n->symbol;
    size_t    offset = 0;
    while (s) {
        if (strcmp(s->name, name) == 0) {
            offset = s->offset;
            break;
        }
        s = s->next;
    }
    if (!offset)
        return true;
    uintptr_t addr = n->address + offset;
    // add write permission
    uintptr_t page_start = PAGE_START(addr);
    size_t    page_cover = PAGE_COVER(addr);
    uintptr_t page_end   = PAGE_END(addr);
    if (mprotect((void *)page_start, page_cover, PROT_READ | PROT_WRITE))
        return false;
    // replace the function address
    *(void **)addr = func;
    // clear instruction cache
    __builtin___clear_cache(((void *)page_start), ((void *)page_end));
    return true;
}

bool Hook_Register(const char *exp, const char *name, void *func)
{
    if (exp == NULL || *exp == '\0' || name == NULL || *name == '\0' || func == NULL)
        return false;
    bool       isOk = true;
    DLLNode_t *p    = dll_list;
    while (p && isOk) {
        int ret = fnmatch(exp, p->name, 0);
        if (ret == 0)
            isOk = _Hook_Register(p, name, func);
        p = p->next;
    }
    return isOk;
}

void Hook_Finish(void)
{
    while (dll_list) {
        DLLNode_t *p = dll_list;
        dll_list     = dll_list->next;
        while (p->symbol) {
            Symbol_t *s = p->symbol;
            p->symbol   = s->next;
            free(s);
        }
        free(p);
    }
    return;
}
