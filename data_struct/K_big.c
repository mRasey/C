#include <stdio.h>
//#include <Windows.h>

int n, k;
int num[1000] = {0};
void QSort(int begin, int end);
void change(int a, int b);

int main(){
    int i;
    scanf("%d %d", &n, &k);
    for(i = 0; i < n; i++){
        scanf("%d", &num[i]);
    }
    QSort(0, n - 1);
    //system("pause");
    return 0;
}

void QSort(int begin, int end){
    int i, j, temp;
    /*if(begin == n - k){
        printf("%d\n", num[begin]);
    }*/
    if(begin < end){
        i = begin;
        j = end + 1;
        while(1){
            //for(i = begin + 1; !(num[begin] <= num[i] || i == end); i++);
            do{
                i++;
            }while(!(num[begin] <= num[i] || i == end)); 
            //for(j = end; !(num[j] <= num[begin] || j == begin); j--);
            do{
                j--;
            }while(!(num[j] <= num[begin] || j == begin));
            if(i < j){
                //change(num[i], num[j]);
                temp = num[i];
                num[i] = num[j];
                num[j] = temp;
            }
            else{
                break;
            }
        }
        //change(num[begin], num[j]);
        temp = num[begin];
        num[begin] = num[j];
        num[j] = temp;
        if(begin == n - k){
            printf("%d\n", num[begin]);
        }
        else if(j == n - k){
            printf("%d\n", num[j]);
        }
        else if(j > n - k)
            QSort(begin, j - 1);
        else if(j < n - k)
            QSort(j + 1, end);
        
    }
}

void change(int a, int b){
    int temp;
    temp = a;
    a = b;
    b = temp;
}