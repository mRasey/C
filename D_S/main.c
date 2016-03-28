#include <stdio.h>
#include <malloc.h>

typedef struct TREE{
    int num;
    struct TREE *lchild;
    struct TREE *rchild;
} tree, *Tree;

int N;

void front(Tree T);
void mid(Tree T);
void behind(Tree T);

int main(){
    int i;
    int input;
    Tree p, q, T = NULL;
    scanf("%d", &N);
    for(i = 0; i < N; i++){
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
    printf("\n");
    mid(T);
    printf("\n");
    behind(T);
    return 0;
}

void front(Tree T){
    Tree p = T;
    Tree stack[100];
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
}

void mid(Tree T){
    Tree p;
    Tree stack[100];
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
}

void behind(Tree T){
    Tree p = T;
    Tree stack[100];
    int stack2[100] = {0}, flag = 0, top = -1;
    if(T != NULL){
        do{
            while(p != NULL){
                stack[++top] = p;
                stack2[top] = 0;
                p = p -> lchild;
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
}