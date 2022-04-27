#include "mem.h"
#include "alloc.h"
#include "string.h"

pte_t *walk(pagetable_t pagetable, uint64_t va, int alloc) {
  for (int level = 3; level > 0; level--) {
    pte_t *pte = &pagetable[PX(level, va)];
    if (*pte & PT_PAGE || *pte & PT_BLOCK) {
      pagetable = (pagetable_t)PA2KA(PTE2PA(*pte));
    } else {
      if (!alloc ||
          (pagetable = (pagetable_t)get_page_addr(page_alloc(0))) == 0)
        return 0;
      memset(pagetable, 0, PGSIZE);
      *pte = KA2PA((uint64_t)pagetable) | PT_PAGE;
    }
  }
  return &pagetable[PX(0, va)];
}

void mappages(pagetable_t pagetable, uint64_t va, uint64_t size, uint64_t pa,
              int perm) {
  uint64_t a, last;
  pte_t *pte;
  a = PGROUNDDOWN(va);
  last = PGROUNDDOWN(va + size - 1);
  for (; a != last + PGSIZE; a += PGSIZE, pa += PGSIZE) {
    pte = walk(pagetable, a, 1);
    *pte = KA2PA(pa) | perm | PT_PAGE;
  }
}
