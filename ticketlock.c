// Ticket locks.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "ticketlock.h"



// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off.

void
pushcli_tl()
{
    int eflags;

    eflags = readeflags();
    cli();
    if(mycpu()->ncli == 0)
        mycpu()->intena = eflags & FL_IF;
    mycpu()->ncli += 1;
}

void
popcli_tl()
{
    if(readeflags()&FL_IF)
        panic("popcli - interruptible");
    if(--mycpu()->ncli < 0)
        panic("popcli");
    if(mycpu()->ncli == 0 && mycpu()->intena)
        sti();
}

int turn = 0;
int total = 0;

void
initticketlock(struct ticketlock* tl, char* name)
{
    turn = 0;
    total = 0;
}


void
acquireticketlock(struct ticketlock *tl)
{
    pushcli_tl(); // disable interrupts to avoid deadlock.

    myproc()->turn = fetch_and_add(&total,1);
    while(myproc()->rw_turn != turn);

    popcli_tl();
}

// Release the lock.
void
releaseticketlock(struct ticketlock *tl)
{
  pushcli_tl();

  turn++;

  popcli_tl();
}

int
ticketlocktest(struct ticketlock* tl){
    int tl_value;
    acquireticketlock(tl);
    tl_value = total;
    releaseticketlock(tl);
    return tl_value;
}

