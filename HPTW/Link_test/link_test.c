#include <stdio.h>
#include <stdlib.h>
typedef   struct    node{
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
      while (ch<100){
           p=(listnode*)malloc(sizeof(listnode));/*分配空间*/
           p->next=head;/*指定后继指针*/
           head=p;/*head指针指定到新插入的结点上*/
           ch++;
      }
      return (head);
}
void printlist(linklist head)
{
     listnode * p;
     p=head;
     while(p)
     {
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

