
#ifndef __JPU_IO_H__
#define __JPU_IO_H__

typedef struct {
    unsigned int  size;
    unsigned long long base;
    unsigned long long phys_addr;
    unsigned long long virt_addr;
    void*         pb;
    int  device_index;
} jpu_mem_desc;

int IOGetPhyMem(jpu_mem_desc* pmd, int flags);
int IOFreePhyMem(jpu_mem_desc* pmd);
int IOGetVirtMem(jpu_mem_desc* pmd, int map_flags);
int IOFreeVirtMem(jpu_mem_desc* pmd);
int IOInvalidatePhyMem(jpu_mem_desc* pmd, unsigned long long phys_addr, unsigned int size);
int IOFlushPhyMem(jpu_mem_desc* pmd, unsigned long long phys_addr, unsigned int size);

#ifdef BM_PCIE_MODE
int IOPcieReadMem(jpu_mem_desc* pmd, unsigned long long dst, uint8_t *src, size_t size);
int IOPcieWriteMem(jpu_mem_desc* pmd, uint8_t *dst, unsigned long long src, size_t size);
#endif
#endif /*  __JPU_IO_H__ */

