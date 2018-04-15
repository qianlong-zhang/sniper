#include <stdio.h>
#include <stdlib.h>

typedef   struct    pat{
    int data;
}pat;

typedef   struct    node{
    //int temp;
    pat *pat_p;
    struct  node  *next;
} listnode;

typedef  listnode  *linklist;
listnode  *p;
void process(int temp, int data)
{
    printf("temp =%d, data = %d\n",temp, data);
}
void process2(int data)
{
    printf("data = %d\n", data);
}
void main()
{
    printf("creating\n");
    unsigned int ch;
    linklist  head;
    listnode  *p;
    head=NULL;/*初始化为空*/
    ch = 0;
    while (ch<100){
        p=(listnode*)malloc(sizeof(listnode));/*分配空间*/
        p->next=head;/*指定后继指针*/
        //p->temp = ch;
        p->pat_p = (pat*)malloc(sizeof(pat));
        p->pat_p->data = ch;
        head=p;/*head指针指定到新插入的结点上*/
        ch++;
    }


    printf("geting\n");
    p=head;
    int temp=0;
    void *p_temp=NULL;
    for(p=head; p; p=p->next)
    {
        //process(p->temp, p->pat_p->data);
        process2(p->pat_p->data);
    }
    printf("end\n");
}

