#include <stdio.h>

#include "config.h"
#include "rcu.h"

int main(int argc, char *argv[]) {
    printf("%s Version : %d.%d.%d\n", "Concurrent-c",
        Concurrent_VERSION_MAJOR,
        Concurrent_VERSION_MINOR,
        Concurrent_VERSION_PATCH);
    printf("1 + 2 = %d\n", 1+2);
    return 0;
}

