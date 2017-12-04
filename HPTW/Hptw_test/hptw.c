#include <stdio.h>
#include <stdlib.h>
typedef struct  pat{
    unsigned int   data;
} patnode;
typedef   struct    node{
    struct  node  *next;
    struct  pat  *pat;
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
           p->pat=(struct pat*)malloc(sizeof(patnode));
	   p->pat->data = ch;
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
    for (p = head ;p;p = p->next)
    {
	(p->pat->data)++;
	printf("%d\n",p->pat->data);
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

