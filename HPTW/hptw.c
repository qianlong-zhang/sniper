#include <stdio.h>
#include <stdlib.h>
typedef   struct    node{
    unsigned int      data;
    struct  node  *next;
} listnode;
typedef  listnode  *linklist;
listnode  *p;
linklist  createlist(void)
{
      unsigned int ch;
      linklist  head;
      listnode  *p;
      head=NULL;/*初始化为空*/
      ch = 0;
      while (ch<10){
           p=(listnode*)malloc(sizeof(listnode));/*分配空间*/
           p->data=ch;/*数据域赋值*/
           p->next=head;/*指定后继指针*/
           head=p;/*head指针指定到新插入的结点上*/
           printf("%d, %p\n",ch, p);
           ch++;
      }
      return (head);
}
void printlist(linklist head)
{
     listnode * p;
     p=head;
     while(p->next){/*遍历第i个结点前的所有结点*/
           printf("%d\n",p->data);
           p=p->next;
     }
}
void main()
{
        linklist    list;
         printf("creating\n");
        list=createlist();
         printf("geting\n");
         printlist(list);
         printf("end\n");
}

