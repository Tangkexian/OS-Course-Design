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
  // �� p ��ʼ��Ϊ pa_start ����ȡ����ҳ��С�ı���
  p = (char*)PGROUNDUP((uint64)pa_start);
  // �� p ��ʼ��ÿ������һҳ��С��ֱ�� p ���ڵ��� pa_end
  for (; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
  {
    // �� kmem.rc �����ж�Ӧ��Ԫ����Ϊ 1
    kmem.rc[(uint64)p / PGSIZE] = 1;
    // ���� kfree �����ͷ���һҳ�ڴ�
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

  // ��������ַ�Ƿ�Ϸ�
  if (((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // ��ȡ kmem.lock ��
  acquire(&kmem.lock);
  // �������ü���
  kmem.rc[(uint64)pa / PGSIZE]--;
  // ������ü���Ϊ0���ͷ��ڴ�
  if (kmem.rc[(uint64)pa / PGSIZE] <= 0) {
    // ���ڴ�����
    memset(pa, 1, PGSIZE);
    // ���ڴ�����������
    r = (struct run*)pa;
    r->next = kmem.freelist;
    kmem.freelist = r;
  }
  // �ͷ� kmem.lock ��
  release(&kmem.lock);
}


// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void* kalloc(void)
{
  struct run* r;

  // ��ȡ kmem.lock ��
  acquire(&kmem.lock);
  // ��ȡ���������ͷ��
  r = kmem.freelist;
  if (r) {
    // �����������ͷ��ָ����һ��Ԫ��
    kmem.freelist = r->next;
    // �������ü���Ϊ 1
    kmem.rc[(uint64)r / PGSIZE] = 1;
  }
  // �ͷ� kmem.lock ��
  release(&kmem.lock);

  // �������ɹ������ڴ�����
  if (r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}


void increase_rc(uint64 pa) {
  // ��ȡ kmem.lock ��
  acquire(&kmem.lock);
  // ���� kmem.rc �����ж�ӦԪ�ص�ֵ
  kmem.rc[pa / PGSIZE]++;
  // �ͷ� kmem.lock ��
  release(&kmem.lock);
}


int cow_alloc(pagetable_t pagetable, uint64 va) {
  uint64 pa;
  uint64 mem;
  pte_t* pte;
  // ��������ַ�Ƿ�Ϸ�
  if (va >= MAXVA)
    return -1;
  // ��ȡ�����ַ����ҳ����ʼ��ַ
  va = PGROUNDDOWN(va);
  // ��ȡҳ����
  pte = walk(pagetable, va, 0);
  // ���ҳ�����Ƿ����
  if (pte == 0) {
    return -1;
  }
  // ���ҳ�Ƿ���Ч��Ϊ COW ҳ
  if (!(*pte & PTE_V)) {
    return -2;
  }
  // ��ȡ�����ַ
  pa = PTE2PA(*pte);

  // ������ü���Ϊ1����������Ϊ��д
  acquire(&kmem.lock);
  if (kmem.rc[pa / PGSIZE] == 1) {
    *pte &= ~PTE_COW;
    *pte |= PTE_W;
    release(&kmem.lock);
    return 0;
  }
  release(&kmem.lock);

  // �����µ������ڴ�
  if ((mem = (uint64)kalloc()) == 0) {
    return -3;
  }

  // �����ڴ�����
  memmove((void*)mem, (void*)pa, PGSIZE);

  // ����ҳ����
  *pte = ((PA2PTE(mem) | PTE_FLAGS(*pte) | PTE_W) & (~PTE_COW));

  // �������ü���
  kfree((void*)pa);

  return 0;
}
