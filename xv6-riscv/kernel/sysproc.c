#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
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
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
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

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
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

int setpriority(int num){
  if (num<1 || num>20)
    return -1; // Μη έγκυρη προτεραιότητα

  myproc()->priority=num;
  return 0;
}

struct pstat{
  int num_processes;  //number of processes
  struct proc processes[100]; //array for each process info , max_process==100
};

int getpinfo(struct pstat *stats){
  if (stats == 0 || !is_valid_ptr(stats, sizeof(struct pstat)))
    return -1;

  acquire(&ptable->lock);  //locking the table

  stats->num_processes = 0;
  for (struct proc *p=ptable; p<&ptable[NPROC]; p++) {
    if (p->state!=UNUSED){
      stats->processes[stats->num_processes].pid=p->pid;
      //no parent ppid=0, else ppid=pid of parent
      stats->processes[stats->num_processes].ppid=p->parent ? p->parent->pid : 0;
      //safe copy of p.name to stats with max size p.name
      safestrcpy(stats->processes[stats->num_processes].name, p->name, sizeof(p->name));
      stats->processes[stats->num_processes].priority=p->priority;
      stats->processes[stats->num_processes].state=p->state;
      stats->processes[stats->num_processes].size=p->sz;
      //if we add we raised 
      stats->num_processes++;
    }
  }
  release(&ptable->lock);  //unlock the ptable
  return 0;
}