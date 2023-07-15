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
  int rc[PHYSTOP / PGSIZE];
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void freerange(void* pa_start, void* pa_end)
{
  char* p;
  // 将 p 初始化为 pa_start 向上取整到页大小的倍数
  p = (char*)PGROUNDUP((uint64)pa_start);
  // 从 p 开始，每次增加一页大小，直到 p 大于等于 pa_end
  for (; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
  {
    // 将 kmem.rc 数组中对应的元素设为 1
    kmem.rc[(uint64)p / PGSIZE] = 1;
    // 调用 kfree 函数释放这一页内存
    kfree(p);
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void* pa)
{
  struct run* r;

  // 检查物理地址是否合法
  if (((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // 获取 kmem.lock 锁
  acquire(&kmem.lock);
  // 减少引用计数
  kmem.rc[(uint64)pa / PGSIZE]--;
  // 如果引用计数为0，释放内存
  if (kmem.rc[(uint64)pa / PGSIZE] <= 0) {
    // 将内存清零
    memset(pa, 1, PGSIZE);
    // 将内存加入空闲链表
    r = (struct run*)pa;
    r->next = kmem.freelist;
    kmem.freelist = r;
  }
  // 释放 kmem.lock 锁
  release(&kmem.lock);
}


// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void* kalloc(void)
{
  struct run* r;

  // 获取 kmem.lock 锁
  acquire(&kmem.lock);
  // 获取空闲链表的头部
  r = kmem.freelist;
  if (r) {
    // 将空闲链表的头部指向下一个元素
    kmem.freelist = r->next;
    // 设置引用计数为 1
    kmem.rc[(uint64)r / PGSIZE] = 1;
  }
  // 释放 kmem.lock 锁
  release(&kmem.lock);

  // 如果分配成功，将内存清零
  if (r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}


void increase_rc(uint64 pa) {
  // 获取 kmem.lock 锁
  acquire(&kmem.lock);
  // 增加 kmem.rc 数组中对应元素的值
  kmem.rc[pa / PGSIZE]++;
  // 释放 kmem.lock 锁
  release(&kmem.lock);
}


int cow_alloc(pagetable_t pagetable, uint64 va) {
  uint64 pa;
  uint64 mem;
  pte_t* pte;
  // 检查虚拟地址是否合法
  if (va >= MAXVA)
    return -1;
  // 获取虚拟地址所在页的起始地址
  va = PGROUNDDOWN(va);
  // 获取页表项
  pte = walk(pagetable, va, 0);
  // 检查页表项是否存在
  if (pte == 0) {
    return -1;
  }
  // 检查页是否有效且为 COW 页
  if (!(*pte & PTE_V)) {
    return -2;
  }
  // 获取物理地址
  pa = PTE2PA(*pte);

  // 如果引用计数为1，将其设置为可写
  acquire(&kmem.lock);
  if (kmem.rc[pa / PGSIZE] == 1) {
    *pte &= ~PTE_COW;
    *pte |= PTE_W;
    release(&kmem.lock);
    return 0;
  }
  release(&kmem.lock);

  // 分配新的物理内存
  if ((mem = (uint64)kalloc()) == 0) {
    return -3;
  }

  // 复制内存内容
  memmove((void*)mem, (void*)pa, PGSIZE);

  // 更新页表项
  *pte = ((PA2PTE(mem) | PTE_FLAGS(*pte) | PTE_W) & (~PTE_COW));

  // 减少引用计数
  kfree((void*)pa);

  return 0;
}
