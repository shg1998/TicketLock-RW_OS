#include "types.h"
#include "user.h"
#define NCHILD 10

void testReadersWriters(int* pattern, int pattern_size);

int main(){
    char argv[100];
    int read_size;
    printf(1, "Enter pattern for readers/writers test\n");
    read_size = read(0, argv, sizeof(argv));
    argv[read_size - 1] = '\0';
    int pattern[100], i;
    for (i = 0; argv[i+1] != '\0'; ++i) {
        if (argv[i+1] == '0')
            pattern[i] = 0;
        else if (argv [i + 1] == '1')
            pattern[i] = 1;
        else{
            printf(1, "Pattern must consist of only 0 and 1");
            exit();
        }
    }
    testReadersWriters(pattern, i);
    exit();
    return 0;
}

void testReadersWriters(int* pattern, int pattern_size){
    int pid, i;
    rwinit();
    pid = fork();
    for (i = 1; i < pattern_size; ++i) {
        if(pid < 0){
            printf(1, "fork failed\n");
            exit();
        }
        else if (pid > 0)
            pid = fork();
        else
            break;
    }

    if (pid < 0){
        printf(1, "fork failed\n");
        exit();
    }
    else if (pid == 0){
        printf(1, "Child adding to shared counter\n");
        int res = rwtest(pattern[i-1]);
        if (pattern[i-1] == 0)
            printf(2, "Reader read from shared counter : %d\n", res);
        else
            printf(1, "Writer added to shared counter\n");
    }
    else{
        for (int i = 0; i < pattern_size; ++i) {
            wait();
        }
        printf(1, "User program finished\n");
        int res = rwtest(0);
        printf(1, "Last value of shared counter: %d\n", res);
    }
}