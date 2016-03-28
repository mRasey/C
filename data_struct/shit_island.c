#include<stdio.h>
#include<math.h>

typedef struct Island{
    int x;
    int y;
    int m;
    float time; //��¼�������ӵ�ʱ��
    //int flag;
    //float length;
}Land;

struct Island island[100];
int queue[1000] = {0};
int top = 0;
int end = 0;
int load[1000] = {-1}; //�洢������������·��
float dis[100][100] = {{0}}; //����������֮�������̾���
int sum; //��������
//float time[100] = {0}; //��¼�������ӵ�ʱ��
float load_edge[100] = {0}; //����ĳһȷ��·����¼���еı�
int linked_count = 0;
float answer;
int linked[100] = {0};
float min = 0;
float time;
float final_time[100] = {0};

float distance(int i, int j); //����������֮���ľ���

int main(){
    int i, j;
    scanf("%d", &sum);
    for(i = 0; i < sum; i++){
        scanf("%d%d%d", &island[i].x, &island[i].y, &island[i].m);
    }
    sort(0);
    linked[1] = 1;
    linked_count = 2;
    load[0] = 0;
    load[1] = 0;
    for(i= 2; i < sum; i++){
        min = distance(i, 0);
        load[i] = 0;
        for(j = 1; j < linked_count; j++){
            if(distance(i, j) < min){
                min = distance(i, j);
                load[i] = j; //��¼����
                //printf("%d %d\n", j, load[i]);
            }
        }
        linked_count++;
    }


    final_time[0] = 0;
    for(i = 0; i < sum; i++){  //����
        for(j = 1; j < sum; j++){ //load[j]ǰ�ڵ�
            if(load[j] == i && load[j] == 0){  //�ڶ���
                island[j].time = distance(j, load[j]);
                //final_time[j] = island[j].time;
                //printf("%f\n", island[j].time);
            }
            else if(load[j] == i){ //�����㼰�Ժ�
                time = distance(j, load[j]);
                if(time > island[load[j]].time){
                    island[j].time = distance(j, load[j]); //����һ��ʱ������һ�㳤ʱ����һ��Ϊ׼
                    //printf("%f\n", island[j].time);
                }
                else{
                    island[j].time = island[load[j]].time; //����һ��ʱ������һ����ʱ����һ��Ϊ׼
                    //final_time =
                    //printf("%f\n", island[j].time);
                }
            }

        }
    }
    /*for(i = 0; i < sum; i++){
        printf("%f\n", island[i].time);
    }*/

    average();
    printf("%.2f", answer);

    return 0;
}

void average(){
    float sum_people = 0;
    float sum_time_mult_people = 0;
    int i;
    for(i = 0; i < sum; i++){
        sum_people += island[i].m;
    }
    for(i = 0; i < sum; i++){
        sum_time_mult_people += island[i].time * island[i].m;
    }
    //printf("%")
    answer = sum_time_mult_people / sum_people;
}


void sort(){ //����������������Զ������
    int i, j;
    struct Island temp;
    int tmp1, tmp2, tmp3;
    for(i = 1; i < sum; i++){
        for(j = i + 1; j < sum; j++){
            if(distance(0, i) > distance(0, j)){
                tmp1 = island[i].x;
                tmp2 = island[i].y;
                tmp3 = island[i].m;
                island[i].x = island[j].x;
                island[i].y = island[j].y;
                island[i].m = island[j].m;
                island[j].x = tmp1;
                island[j].y = tmp2;
                island[j].m = tmp3;
            }
        }
    }

}

float distance(int i, int j){
    float distance;
    float det_x = (island[i].x - island[j].x);
    float det_y = (island[i].y - island[j].y);
    distance = (float)sqrt((det_x * det_x) + (det_y * det_y));
    return distance;
}
