struct vmarea {
    uint64 start;
    uint64 end;
    struct vmarea* next;
};

struct memset {
    pagetable_t pg;
    struct vmarea* vmlist;
};

#define VMMAX (NPROC * 16)