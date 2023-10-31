#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<unistd.h>
#include<sys/times.h>
#include<sys/types.h>
#include<sys/wait.h>
#define HZ 100
const int n = 5;

void cpuio_bound(int last, int cpu_time, int io_time);

int main(int argc, char *argv[])
{
    int i = 0;
    pid_t cpid[n];
    pid_t par = getpid();
    int status = -1;
    printf("now parent.pid=%d\n", par);
    for(i = 0; i < n; i++){
        cpid[i] = fork();
        if(cpid[i] < 0){
            printf("error while creating %dth process\n", i);
        }else if(cpid[i] == 0){
            //child
            cpuio_bound(n, i, n-i);
            //cpuio_bound(n, n-i, i)
            printf("now child(%d), pid=%d ppid=%d\n", i, getpid(), getppid());
            exit(i);
        }
        //parent process move on
    }
    for(int i=0; i<n; ++i){
        pid_t a=wait(&status);
    }
    for(int i=0; i<n; ++i){
        printf("child(%d):pid=%d\n",i,cpid[i]);
    }

    printf("the end\n");
    return 0;
}

void cpuio_bound(int last, int cpu_time, int io_time){
    struct tms start_time, current_time;
    clock_t utime, stime;
    int sleep_time;

    while (last > 0)
    {
        /* code */
        times(&start_time);
        do{
            times(&current_time);
            utime = current_time.tms_utime - start_time.tms_utime;
            stime = current_time.tms_stime - start_time.tms_stime;
        }while(((utime+stime) / HZ) < cpu_time);
        last -= cpu_time;

        if(last <= 0)
            break;
        
        sleep_time = 0;
        while (sleep_time < io_time)
        {
            sleep(1);
            sleep_time++;
        }
        last -= sleep_time;
    }
    
    
}