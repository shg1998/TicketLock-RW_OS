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

struct ticketlock* reader_mutex;
struct ticketlock* writer_mutex;
struct ticketlock* writer_permission;
struct ticketlock* reader_permission;
int reader_count;
int writer_count;
int shared_value;

void
pushcli_rw()
{
    int eflags;

    eflags = readeflags();
    cli();
    if(mycpu()->ncli == 0)
        mycpu()->intena = eflags & FL_IF;
    mycpu()->ncli += 1;
}

void
popcli_rw()
{
    if(readeflags()&FL_IF)
        panic("popcli - interruptible");
    if(--mycpu()->ncli < 0)
        panic("popcli");
    if(mycpu()->ncli == 0 && mycpu()->intena)
        sti();
}

void
rwinit(void)
{
    reader_mutex = (struct ticketlock*) kalloc();
    reader_mutex->turn = 0;
    reader_mutex->total = 0;
    reader_mutex->name = "mr";

    writer_mutex = (struct ticketlock*) kalloc();
    writer_mutex->turn = 0;
    writer_mutex->total = 0;
    writer_mutex->name = "mw";

    reader_permission = (struct ticketlock*) kalloc();
    reader_permission->turn = 0;
    reader_permission->total = 0;
    reader_permission->name = "rp";

    writer_permission = (struct ticketlock*) kalloc();
    writer_permission->turn = 0;
    writer_permission->total = 0;
    writer_permission->name = "wp";

    reader_count = 0;
    writer_count = 0;
    shared_value = 0;
}

void
acquirerwlock(struct ticketlock* tl)
{
    pushcli_rw();
    myproc()->rw_turn = fetch_and_add(&tl->total, 1);
    while(myproc()->rw_turn != tl->turn) {
//        cprintf("%s\n", tl->name);
//        cprintf("T:%d\n", tl->total);
//        cprintf("t:%d\n", tl->turn);
//        cprintf("r:%d\n", reader_count);
//        cprintf("w:%d\n", writer_count);
//        cprintf("%d\n", myproc()->rw_turn);
    }
    popcli_rw();
}

// Release the lock.
void
releaserwlock(struct ticketlock* tl)
{
    pushcli_rw();
    fetch_and_add(&tl->turn, 1);
    popcli_rw();
}

void
reader()
{
    shared_value++;
    acquirerwlock(reader_permission);
    acquirerwlock(reader_mutex);
    {
        reader_count++;
        if(reader_count == 1) {
            acquirerwlock(writer_permission);
        }
    }
    releaserwlock(reader_mutex);

    releaserwlock(reader_permission);

    while(myproc()->work){
        myproc()->work--;
    }

    acquirerwlock(reader_mutex);
    {
        reader_count--;
        if(reader_count == 0)
            releaserwlock(writer_permission);
    }
    releaserwlock(reader_mutex);
}

void
writer()
{
    shared_value++;
    acquirerwlock(writer_mutex);
    {
        writer_count++;
        if(writer_count == 1) {
            acquirerwlock(reader_permission);
        }
    }
    releaserwlock(writer_mutex);

    acquirerwlock(writer_permission);
    while(myproc()->work){
        myproc()->work--;
    }
    releaserwlock(writer_permission);

    acquirerwlock(writer_mutex);
    {
        writer_count--;
        if(writer_count == 0)
            releaserwlock(reader_permission);
    }
    releaserwlock(writer_mutex);
}

int
rwtest(int pattern)
{
    if (pattern == 0) {
        reader();
        return shared_value;
    }else{
        writer();
        return shared_value;
    }
}