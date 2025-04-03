#include <unistd.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <iostream>
#include <stdio.h>

int main()
{
    int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if(timerfd < 0)
    {
        perror("timerfd_create error");
        return -1;
    }

    struct itimerspec itime;
    itime.it_value.tv_sec = 1;
    itime.it_value.tv_nsec = 0; //第一次超时时间为1s后
    itime.it_interval.tv_sec = 1;
    itime.it_interval.tv_nsec = 0; //第一次超时后，每次超时的间隔时间
    timerfd_settime(timerfd, 0, &itime, NULL);

    while(1)
    {
        uint64_t times;
        int ret = read(timerfd, &times, 8);
        if(ret < 0)
        {
            perror("read error");
            return -1;
        }
        printf("time out, %ld times\n", times);
    }
    close(timerfd);
    return 0;
}
