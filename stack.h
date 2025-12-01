//stack.h
#ifndef STACK_H
#define STACK_H
#include <stdio.h>
#include <stdlib.h>

struct node {
    long double data;
     struct node* link;
}*top=NULL;//this pointer always point the top of the node and currently it empty
//my push fuction which is similar add_begi function in link list 
int isEmpty(){
    if (top==NULL)
    return 1;
    else 
    return 0;
}
void push(long double data)
{
struct node* newnode;
newnode= (struct node*)malloc(sizeof(struct node));//dynamic memmory and typecast from void pointer 
if (newnode == NULL)
{
    printf("stack overflow");
    exit(1);}
newnode->data=data;
newnode->link=NULL;

newnode->link=top;
top=newnode;
}


long double pop()
{ struct node* temp;
    temp=top;
    if (isEmpty())
    {
        printf("stack underflow");
        exit(1);}
     long double val=temp->data;
      top=top->link;// 2nd node become top
        free(temp);
         temp=NULL;
          return val;
}
long double peek()
{
    if (isEmpty())
    {
        printf("stack underflow");
        exit(1);}
    return top->data;
}



#endif