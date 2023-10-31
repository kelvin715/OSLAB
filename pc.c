#define __LIBRARY__
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>

#define BUFFSIZE 5
#define N 10
#define M 511
#define K 10

_syscall2(sem_t *, sem_open, const char *, name, unsigned int, value);
_syscall1(int, sem_wait, sem_t *, sem);
_syscall1(int, sem_post, sem_t *, sem);
_syscall1(int, sem_unlink, const char *, name);

pid_t a, c;
pid_t b[N];
int in, out;
sem_t *space, *item, *mutex, *ctrl_space, *ctrl_item;
int fd;

void Producer();
void Controller();
void Consumer();

int main()
{
    int i;
    if ((space = sem_open("/space", K)) == SEM_FAILED)
    {
        printf("space error!");
    }
    if ((item = sem_open("/item", 0)) == SEM_FAILED)
    {
        printf("item error!");
    }
    if ((ctrl_space = sem_open("/ctrl_space", 1)) == SEM_FAILED)
    {
        printf("ctrl_space error!");
    }
    if ((ctrl_item = sem_open("/ctrl_item", 0)) == SEM_FAILED)
    {
        printf("ctrl_item error!");
    }
    if ((mutex = sem_open("/mutex", 1)) == SEM_FAILED)
    {
        printf("mutex error!");
    }
    printf("sem build ok!\n");
    fd = open("/usr/root/buffer", O_CREAT | O_RDWR | O_TRUNC, 0666);

    a = fork();
    if (a < 0)
    {
        printf("error while creating producer process\n");
    }else if (a == 0)
    {
        Producer();
        return 0;
    }

    c = fork();
    if (c < 0)
    {
        printf("error while creating producer process\n");
    }else if (c == 0)
    {
        Controller();
        return 0;
    }

    for (i = 0; i < N; i++)
    {
        b[i] = fork();
        if (b[i] < 0)
        {
            printf("error while creating %dth consumer process\n", i);
        }else if (b[i] == 0)
        {
            Consumer();
            return 0;
        }
    }

    wait(NULL);
    sem_unlink("/space");
    sem_unlink("/item");
    sem_unlink("/ctrl_item");
    sem_unlink("/ctrl_space");
    sem_unlink("/mutex");
    close(fd);

    return 0;
}
void Producer()
{
    int j;
    for (j = 0; j < M; j++)
    {
        sem_wait(space);
        lseek(fd, in * sizeof(int), SEEK_SET);
        write(fd, (char *)&j, sizeof(int));
        in = (in + 1) % K;
        sem_post(item);
    }
    return;
}
void Controller()
{
    int k = 0;
    while (k < M)
    {
        sem_wait(ctrl_space);
        sem_wait(item);
        lseek(fd, out * sizeof(int), SEEK_SET);
        read(fd, (char *)&k, sizeof(int));
        out = (out + 1) % K;
        lseek(fd, K * sizeof(int), SEEK_SET);
        write(fd, (char *)&k, sizeof(int));
        sem_post(ctrl_item);
        sem_post(space);
    }
    return;
}
void Consumer()
{
    int v;
    while (1)
    {
        sem_wait(ctrl_item);
        sem_wait(mutex);

        lseek(fd, K * sizeof(int), SEEK_SET);
        read(fd, (char *)&v, sizeof(int));

        printf("%d:%d\n", getpid(), v);

        sem_post(mutex);
        sem_post(ctrl_space);
    }
    return;
}
