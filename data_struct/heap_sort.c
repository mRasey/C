#include <stdio.h>
//#include <Windows.h>

int n;
int num[1000] = {0};
void adjust(int i, int n);
void HSort(int n);
void print_array();

int main(){
    int i;
    scanf("%d", &n);
    for(i = 0; i < n; i++){
        scanf("%d", &num[i]);
    }
    HSort(n);
    //HSort(n-1);
    /*for(i = n; i >= 0; i--){
        HSort(i);
    }*/
    print_array();

    //system("pause");
    return 0;
}

void print_array(){
    int i;
    for(i = 0; i < n; i++){
        printf("%d ", num[i]);
    }
    printf("\n");
}

void HSort(int n){
    int i;
    int temp;
    for(i = (n - 1) / 2; i >= 0; i--)//建立初始堆积
        adjust(i, n - 1);
    //print_array();
    for(i = n - 1; i > 0; i--){
        temp = num[i];//交换首尾数字
        num[i] = num[0];
        num[0] = temp;
        adjust(0, i - 1);
        //print_array();
    }
}

void adjust(int i, int n){
    int temp, j;
    temp = num[i];//防止交换过程中父亲节点的值被覆盖掉
    for(j = 2 * i + 1; j <= n; j = 2 * j + 1){
        if(j < n && num[j] < num[j+1])
            j++;//取较大的孩子
        if(temp >= num[j])//大于任何一个孩子
            break;
        num[i] = num[j]; //把某孩子放在父节点上
        i = j;
    }
    num[i] = temp;
}