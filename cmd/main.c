#include <stdio.h>

#include "rcu.h"

int main(int argc, char *argv[]) {
    printf("Concurrent-c Version : %s\n", share_get_ver());
    return 0;
}
