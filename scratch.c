#include <errno.h>
#include <limits.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char **argv)
{
    //  mq variables
    for (int i = -5; i < 2 * (1000 * 1000 + 10); ++i) {
        for (int j = 0; j < 3; ++j) {
            printf("%d\n", i);
        }
    }
    return 0;
}