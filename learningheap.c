
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
int main(){
    int *ptr = malloc(5*sizeof(int));
    
    return 0;
}

char* stringcac(char *ch2, char ch, int size){
    bool flag = false;
    int count = 1;
    int i =0;
    while (flag == false){
        char ch3 = ch2[i];
        if(ch3 == '\0'){
            flag=true;
        }else{
            i++;
            count++;
        }
    }
    count = count + 1;
    if(count<= size){
        flag = false;
    }else{
        flag = true;
    }
    for(int i = 0;i>(count-1);i++){
        if (flag == false){
            
        }else{

        }
    }
    return 's';
}
