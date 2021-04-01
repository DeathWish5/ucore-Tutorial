#include "defs.h"
#include "elf.h"
#include "memory_layout.h"
#include "proc.h"
#include "riscv.h"
#include "trap.h"

#define MAXARG 32  // max exec arguments

static int loadseg(pde_t* pgdir,
                   uint64 addr,
                   struct inode* ip,
                   uint offset,
                   uint sz);

int exec(char* path, char** argv) {
    info("exec : %s\n", path);
    int i, off;
    uint64 argc, sz = 0, sp, ustack[MAXARG + 1], stackbase;
    struct elfhdr elf;
    struct inode* ip;
    struct proghdr ph;
    pagetable_t pagetable = 0, oldpagetable;
    struct proc* p = curr_proc();

    if ((ip = namei(path)) == 0) {
        error("invalid file name\n");
        return -1;
    }
    // Check ELF header
    if (readi(ip, 0, (uint64)&elf, 0, sizeof(elf)) != sizeof(elf)) {
        error("invalid elf header length\n");
        goto bad;
    }
    if (elf.magic != ELF_MAGIC) {
        error("invalid elf magic\n");
        goto bad;
    }

    if ((pagetable = proc_pagetable(p)) == 0) {
        error("????\n");
        goto bad;
    }

    // Load program into memory.
    for (i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)) {
        if (readi(ip, 0, (uint64)&ph, off, sizeof(ph)) != sizeof(ph)) {
            error("invalid program ph\n");
            goto bad;
        }
        if (ph.type != ELF_PROG_LOAD)
            continue;
        if (ph.memsz < ph.filesz) {
            error("5\n");
            goto bad;
        }
        if (ph.vaddr + ph.memsz < ph.vaddr) {
            error("6\n");
            goto bad;
        }
        uint64 sz1;
        if ((sz1 = uvmalloc(pagetable, sz, ph.vaddr + ph.memsz)) == 0)
            goto bad;
        sz = sz1;
        if (ph.vaddr % PGSIZE != 0) {
            error("7 v = %p\n", ph.vaddr);
            goto bad;
        }
        if (loadseg(pagetable, ph.vaddr, ip, ph.off, ph.filesz) < 0) {
            error("8\n");
            goto bad;
        }
    }
    iput(ip);
    ip = 0;

    p = curr_proc();
    uint64 oldsz = p->sz;

    // Allocate two pages at the next page boundary.
    // Use the second as the user stack.
    sz = PGROUNDUP(sz);
    uint64 sz1;
    if ((sz1 = uvmalloc(pagetable, sz, sz + 2 * PGSIZE)) == 0) {
        error("4\n");
        goto bad;
    }
    sz = sz1;
    uvmclear(pagetable, sz - 2 * PGSIZE);
    sp = sz;
    stackbase = sp - PGSIZE;

    // Push argument strings, prepare rest of stack in ustack.
    for (argc = 0; argv[argc]; argc++) {
        if (argc >= MAXARG)
            goto bad;
        sp -= strlen(argv[argc]) + 1;
        sp -= sp % 16;  // riscv sp must be 16-byte aligned
        if (sp < stackbase) {
            error("2\n");
            goto bad;
        }
        if (copyout(pagetable, sp, argv[argc], strlen(argv[argc]) + 1) < 0) {
            error("3\n");
            goto bad;
        }
        ustack[argc] = sp;
    }
    ustack[argc] = 0;

    // push the array of argv[] pointers.
    sp -= (argc + 1) * sizeof(uint64);
    sp -= sp % 16;
    if (sp < stackbase) {
        error("0\n");
        goto bad;
    }
    if (copyout(pagetable, sp, (char*)ustack, (argc + 1) * sizeof(uint64)) < 0) {
        error("1\n");
        goto bad;
    }

    // arguments to user main(argc, argv)
    // argc is returned via the system call return
    // value, which goes in a0.
    p->trapframe->a1 = sp;

    // Commit to the user image.
    oldpagetable = p->pagetable;
    p->pagetable = pagetable;
    p->sz = sz;
    p->trapframe->epc = elf.entry;  // initial program counter = main
    p->trapframe->sp = sp;          // initial stack pointer
    proc_freepagetable(oldpagetable, oldsz);
    for (int i = 0; i < FD_MAX; ++i) {
        if (p->files[i] != 0) {
            fileclose(p->files[i]);
            p->files[i] = 0;
        }
    }
    return argc;  // this ends up in a0, the first argument to main(argc, argv)

bad:
    error("invalid elf QAQ\n");
    if (pagetable)
        proc_freepagetable(pagetable, sz);
    if (ip) {
        iput(ip);
    }
    return -1;
}

// Load a program segment into pagetable at virtual address va.
// va must be page-aligned
// and the pages from va to va+sz must already be mapped.
// Returns 0 on success, -1 on failure.
static int loadseg(pagetable_t pagetable,
                   uint64 va,
                   struct inode* ip,
                   uint offset,
                   uint sz) {
    uint i, n;
    uint64 pa;

    if ((va % PGSIZE) != 0)
        panic("loadseg: va must be page aligned");

    for (i = 0; i < sz; i += PGSIZE) {
        pa = walkaddr(pagetable, va + i);
        if (pa == 0)
            panic("loadseg: address should exist");
        if (sz - i < PGSIZE)
            n = sz - i;
        else
            n = PGSIZE;
        if (readi(ip, 0, (uint64)pa, offset + i, n) != n)
            return -1;
    }

    return 0;
}
