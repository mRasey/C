#include<stdio.h>

typedef struct edge{
    int start;
    int end;
    int weight;
    int if_need;
    //int if_sel;
}Elink;

Elink point[100];
int n, count;           
//int load[100] = {0}; //��¼·��
int weight_count = 0; //��Ȩֵ
int sel_edge = 0; //�ѱ�ѡ��ı���
int sel_count = 0;
int seled[100] = {0}; //�ѱ�ѡ��ߵļ���
//int unseled[100] = {0}; //δ��ѡ��ߵļ���
int flag = 0; //0 none 1 ��start 2 ��end 3 ��both
int first_edge = 1;
void sort();

int main(){   //kruskal�㷨
    int i;
    int con;
    scanf("%d%d", &n, &count);
    for(i = 0; i < count; i++){
        scanf("%d%d%d", &point[i].start, &point[i].end, &point[i].weight);
    }
    /*for(i = 1; i <= count; i++){
        point[i].if_need = 0;
    }*/
    //reset_load();
    while(sel_count < n){  //����δѡ��ĵ�
        sort();
        con = if_circle();
        if(flag == 1 || flag == 2){
            weight_count += point[con].weight;
            //printf("%d\n", weight_count);/////////////
            if(flag == 1){ //�����β��
                seled[sel_count++] = point[con].end;
            }
            else if(flag == 2){ //���뿪ʼ��
                seled[sel_count++] = point[con].start;
            }
        }
        else if(flag == 0 && first_edge){ //�����ʼ������
            weight_count += point[con].weight;
            seled[sel_count++] = point[con].start;
            seled[sel_count++] = point[con].end;
            //point[0].weight = 1000000000; //�����������ĩβ
            first_edge = 0;
        }
        //out();
        point[con].weight = 1000000000; //�����������ĩβ
        //printf("\n");
    }
    printf("%d\n", weight_count);
    return 0;
}

/*void reset_load(){
    int i;
    for(i = 1; i <= count; i++){
        count[i] = i;
    }
}*/

int if_circle(){
    int i, con;
    flag = 0;
    con = -1;

    do{
        con++; //con = 0;
        for(i = 0; i < sel_count; i++){
            if(point[con].start == seled[i]){
                flag = 1;
                break;
            }
        }
        for(i = 0; i < sel_count; i++){
            if(point[con].end == seled[i]){
                if(flag == 1)
                    flag = 3;
                else
                    flag = 2;
                break;
            }
        }
    }while((flag == 0) && !(flag == 0 && first_edge));

    return con;
}

/*void join_load(int a, int b){
    load[a] = b;
}*/

void sort(){
    int i, j;
    Elink temp;
    for(i = 0; i < count; i++){
        for(j = i; j < count; j++){
            if(point[i].weight > point[j].weight){
                temp = point[i];
                point[i] = point[j];
                point[j] = temp;
            }
        }
    }
}

void out(){
    int i;
    for(i = 0; i < sel_count; i++){
        printf("%d ", seled[i]);
    }
}
