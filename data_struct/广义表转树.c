#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct NUM{
    int data;
    struct NUM *lchild;
    struct NUM *rchild;
}*Tree, Node;

typedef struct OUT{
    Tree T;
    int floor;
};
char input[100] = {'\0'};
int num[100] = {0};
int length;
char in[100][10]= {{'\0'}};
int len;
int int_in[100] = {0};
Tree queue[100];
int qtop = 0, qend = 0;
int floor[100] = {0};
struct OUT out[100];

int main()
{
    Tree p = NULL;
    int i = 0;
    gets(input);
    //puts(input);
    length = strlen(input);
    get_in();
    get_num();
    //get_in(len);
    /*for(i = 0; i < len; i++){
        printf("%d\n", num[i]);
    }*/
    p = (Tree)malloc(sizeof(Node));
    build(0, len-1, p);
    /*printf("%d\n", p->data);
    printf("%d\n", p->lchild->data);
    printf("%d\n", p->rchild->data);
    printf("%d", p->lchild->lchild->data);*/
    visit(p);


    return 0;
}

void build(int start, int end, Tree p){
    //p = (Tree)malloc(sizeof(Node));
    p -> lchild = (Tree)malloc(sizeof(Node));
    p -> rchild = (Tree)malloc(sizeof(Node));
    //p -> lchild = NULL;
    //p -> rchild = NULL;
    p -> data = num[start];
    if(start == end){
        p -> lchild = NULL;
        p -> rchild = NULL;
        //return 0;
    }
    else{
        int j;
        int cut_first = 0;
        int kuo_first = 0;
        for(j = start+2; j < end; j++){
            if(num[j] == -3){
                cut_first = 1;               //先出现逗号6(7,8(9,10))
                break;
            }
            else if(num[j] == -1){
                kuo_first = 1;               //先出现左括号2(3(4,5))
                break;
            }
        }

        if(kuo_first == 1){
            int count = 1;
            int i = start + 3;
            do{
                if(num[i] == -1){
                    count++;
                }
                else if(num[i] == -2){
                    count--;
                }
                i++;
            }while(count != 1 && count != 0);
            /*if(count == 0){                           //3(4,5)
                p -> lchild -> data = input[start+2];
                if(input[start + 3] == ','){
                    p -> rchild -> data = input[start+4];
                }
                return 0;
            }*/
            //else if(count == 1){                     //2(3(4,5))
            build(start+2, i-1, p->lchild);
            if(num[i] == -3){
                build(i+1, end-1, p->rchild);
            }
            else{
                p -> rchild = NULL;
            }
            /*else{
                p -> rchild = NULL;
            }*/

        }
        else if(cut_first == 1){                      //1(2,3(4,5))
            //p -> lchild -> data = num[start+2];
            build(start+2, start+2, p->lchild);
            build(start+4, end-1, p -> rchild);
        }
        else{
            build(start+2, start+2, p->lchild);
            p -> rchild = NULL;
        }
    }
}

void visit(Tree p){
    /*printf("%d ", p->data);
    if(p -> lchild != NULL){
        visit(p -> lchild);
    }
    if(p -> rchild != NULL){
        visit(p -> rchild);
    }*/

    /*queue[qend++] = p;
    while(qtop != qend){
        printf("%d ", queue[qtop]->data);
        if(queue[qtop] -> lchild != NULL){
            queue[qend++] = queue[qtop] -> lchild;
        }
        if(queue[qtop] -> rchild != NULL){
            queue[qend++] = queue[qtop] -> rchild;
        }
        qtop++;
    }*/

    int i = 0, j = 0;
    out[qend++].T = p;
    out[qtop].floor = 1;
    while(qtop != qend){
        //printf("%d ", queue[qtop]->data);
        if(out[qtop].T -> lchild != NULL){
            //queue[qend++] = queue[qtop] -> lchild;
            out[qend].T = out[qtop].T -> lchild;
            out[qend].floor = out[qtop].floor + 1;
            qend++;
        }
        if(out[qtop].T -> rchild != NULL){
            //queue[qend++] = queue[qtop] -> rchild;
            out[qend].T = out[qtop].T -> rchild;
            out[qend].floor = out[qtop].floor + 1;
            qend++;
        }
        qtop++;
    }

    for(i = out[qend-1].floor; i > 0; i--){
        for(j = 0; j < qend; j++){
            if(out[j].floor == i){
                printf("%d ", out[j].T->data);
            }
        }
        printf("\n");
    }
}











void get_in(){
    int i, j, k;
    for(i = 0, k = 0; i < length; k++){
        /*for(j = 0; input[j] != '(' && input[j] != ')' && input[j] != ',' && j < length; j++){
            in[k][j] = input[i];
        }*/
        j = 0;
        do{
            in[k][j++] = input[i++];
            if(in[k][0] == '(' || in[k][0] == ')' || in[k][0] == ','){
                break;
            }
        }while(input[i] != '(' && input[i] != ')' && input[i] != ',' && i < length);
        //i = i + j;
    }
    len = k;
}

void get_num(){
    int i;
    for(i = 0; i < len; i++){
        if(in[i][0] == '('){
            num[i] = -1;
        }
        else if(in[i][0] == ')'){
            num[i] = -2;
        }
        else if(in[i][0] == ','){
            num[i] = -3;
        }
        else{
            num[i] = atoi(in[i]);
        }
    }
}




