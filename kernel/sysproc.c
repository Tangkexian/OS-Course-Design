#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  backtrace();
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64 sys_sigalarm(void)
{
  struct proc* myProc = myproc(); // 定义一个指向 proc 结构体的指针 myProc，并将其初始化为当前进程
  int n;
  uint64 handler;
  if (argint(0, &n) < 0) // 使用 argint 函数获取第一个参数的值并将其存储在变量 n 中
  {
    return -1; // 如果获取失败，则返回 -1
  }
  myProc->interval = n; // 将当前进程的 interval 成员设置为 n
  if (argaddr(1, &handler) < 0) // 使用 argaddr 函数获取第二个参数的值并将其存储在变量 handler 中
  {
    return -1; // 如果获取失败，则返回 -1
  }
  myProc->handler = (void(*)())handler; // 将当前进程的 handler 成员设置为指向函数的指针，该函数由变量 handler 指定
  return 0; // 函数返回 0

}


uint64 sys_sigreturn(void)
{
  struct proc* myProc = myproc();
  switchTrapframe(myProc->trapframe,myProc->trapframeSave);
  myProc->waitReturn = 0;
  return 0;
}
