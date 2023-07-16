// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU]; // 为每个 CPU 分配独立的 freelist，并用独立的锁保护它。lab8-1

//lab8-1(begin)
char* kmem_lock_names[] = {
  "kmem_cpu_0",
  "kmem_cpu_1",
  "kmem_cpu_2",
  "kmem_cpu_3",
  "kmem_cpu_4",
  "kmem_cpu_5",
  "kmem_cpu_6",
  "kmem_cpu_7",
};//lab8-1(end)



void
kinit()
{
  for (int i = 0; i < NCPU; i++) { // 初始化所有锁 lab8-1
    initlock(&kmem[i].lock, kmem_lock_names[i]);
  }
  freerange(end, (void*)PHYSTOP);
}


void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();//lab8-1

  int cpu = cpuid();//lab8-1

  acquire(&kmem[cpu].lock);//lab8-1
  r->next = kmem[cpu].freelist;//lab8-1
  kmem[cpu].freelist = r;//lab8-1
  release(&kmem[cpu].lock);//lab8-1

  pop_off();//lab8-1
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();//lab8-1

  int cpu = cpuid();//lab8-1

  acquire(&kmem[cpu].lock);//lab8-1

  //lab8-1(begin)
  if (!kmem[cpu].freelist) { // no page left for this cpu
    int steal_left = 64; // steal 64 pages from other cpu(s)
    for (int i = 0; i < NCPU; i++) {
      if (i == cpu) continue; // no self-robbery
      acquire(&kmem[i].lock);
      struct run* rr = kmem[i].freelist;
      while (rr && steal_left) {
        kmem[i].freelist = rr->next;
        rr->next = kmem[cpu].freelist;
        kmem[cpu].freelist = rr;
        rr = kmem[i].freelist;
        steal_left--;
      }
      release(&kmem[i].lock);
      if (steal_left == 0) break; // done stealing
    }
  }
//lab8-1(end)


  r = kmem[cpu].freelist;//lab8-1
  if (r)
    kmem[cpu].freelist = r->next;//lab8-1
  release(&kmem[cpu].lock);//lab8-1

  pop_off();//lab8-1

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
