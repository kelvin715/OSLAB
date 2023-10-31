#include <asm/segment.h>
#include <linux/kernel.h>
#include <unistd.h>
#include <string.h>
#include <linux/sched.h>

#define SHM_COUNT 20

typedef int key_t;
struct
{
    key_t key;
    unsigned long addr;
} shm_table[SHM_COUNT];

int len;

/* shmget()会新建/打开一页内存，并返回该页共享内存的shmid（该块共享内存在操作系统内部的id） */
int sys_shmget(key_t key, size_t size)
{
    int i, exist = 0;
    unsigned long page_addr;
    if (size > 1)
    {
        return -1;
    }
    for (i = 0; i < len; ++i)
    {
        if (shm_table[i].key == key)
        {
            exist = 1;
            return i;
        }
    }
    if (!exist)
    {
        page_addr = get_free_page();
        if (!page_addr)
            return -1;
        printk("shmget get memory's address is 0x%08x\n", page_addr);
        shm_table[len].key = key;
        shm_table[len].addr = page_addr;
        len++;
        printk("page_id=%d",len-1);
    }

    return len - 1;
}

/* shmat()会将shmid指定的共享页面映射到[当前进程]的虚拟地址空间中，并将其首地址返回 */
void *sys_shmat(int shmid)
{
    /* 虚拟地址(逻辑地址)=段选择符+段内偏移 */
    /* int data_limit = 0x4000000; 数据段限长64MB */
    /* LDT描述局部于每个程序的段，包括其代码、数据、堆栈等 */
    unsigned long data_base = get_base(current->ldt[2]);
    printk("current's data_base = 0x%08x,", data_base);
    /* 数据段基址=代码段基址 */
    /* set_base(current->ldt[1],code_base); set_limit(current->ldt[1],code_limit);*/
    /* set_base(current->ldt[2],data_base); set_limit(current->ldt[2],data_limit);*/
    /* brk 指针正好指向堆区顶部 */
    if (shmid < len)
    {
        printk("new page = 0x%08x\n",shm_table[shmid].addr);
        if(put_page(shm_table[shmid].addr, current->brk + data_base)==0){
            printk("mirror error\n");
            return -1;
        } /*para1:physical page,para2:logical*/
        return (void *)current->brk;
    }
    else return -1;
}