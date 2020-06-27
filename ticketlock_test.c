#include "types.h"

#include "user.h"
#include "ticketlock.h"

#define NCHID 10

int main() {
    int pid;
    struct ticketlock tl;
    ticketlockinit(&tl);

    pid = fork();

    for (int i = 1; i < NCHID; ++i){
        if (pid < 0) {
            printf(1, "fork failed\n");
            exit();
        } else if (pid > 0)
            pid = fork();
    }

    if (pid < 0){
        printf(1, "fork failed\n");
        exit();
    }
    else if(pid == 0){
        printf(1, "child adding to shared counter\n");
        printf(2, "ticketlock value for child %d\n", ticketlocktest(&tl) );
    }
    else
    {
        for (int i = 0; i < NCHID; ++i) {
            wait();
        }
        printf(1, "user program finished\n");
        printf(2, "ticketlock value %d\n", ticketlocktest(&tl) );
    }

    exit();

    return 0;
}