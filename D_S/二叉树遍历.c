#include <stdio.h>
#include <malloc.h>

typedef struct TREE{
    int num;
    struct TREE *lchild;
    struct TREE *rchild;
}tree, *Tree;

//void build(Tree T, int sum);
void front(Tree T);
void mid(Tree T);
void behind(Tree T);

int main(){
    int sum;
    Tree T = NULL;
    //build(T, sum);
    int i, input;
    Tree p, q;
    scanf("%d", &sum);
    for(i = 0; i < sum; i++){
        scanf("%d", &input);
        p = (Tree)malloc(sizeof(tree));
        p->num = input;
        p->lchild = NULL;
        p->rchild = NULL;
        if(T == NULL)
            T = p;
        else{
            q = T;
            while(1){
                if(input < q->num){
                    if(q->lchild != NULL)
                        q = q->lchild;
                    else{
                        q->lchild = p;
                        break;
                    }
                }
                else{
                    if(q->rchild != NULL)
                        q = q->rchild;
                    else{
                        q->rchild = p;
                        break;
                    }
                }
            }
        }
    }
    front(T);
    mid(T);
    behind(T);
    return 0;
}

void front(Tree T){
    Tree p = T, stack[100];
    int top = -1;
    stack[++top] = p;
    if(T != NULL){
        while(top != -1){
            p = stack[top--];
            printf("%d ", p->num);
            if(p->rchild != NULL)
                stack[++top] = p->rchild;
            if(p->lchild != NULL)
                stack[++top] = p->lchild;
        }
    }
    printf("\n");
}

void mid(Tree T){
    Tree p, stack[100];
    int top = -1;
    p = T;
    if(T != NULL){
        do{
            while(p != NULL){
                stack[++top] = p;
                p = p->lchild;
            }
            p = stack[top--];
            printf("%d ", p->num);
            p = p->rchild;
        }while(!(p == NULL && top == -1));
    }
    printf("\n");
}

void behind(Tree T){
    Tree p, stack[100];
    int stack2[100] = {0}, top = -1, flag = 0;
    if(T != NULL){
        p = T;
        do{
            while(p != NULL){
                stack[++top] = p;
                stack2[top] = 0;
                p = p->lchild;
            }
            p = stack[top];
            flag = stack2[top--];
            if(flag == 0){
                stack[++top] = p;
                stack2[top] = 1;
                p = p->rchild;
            }
            else{
                printf("%d ", p->num);
                p = NULL;
            }
        }while(!(p == NULL && top == -1));
    }
    printf("\n");
}