#include <stdio.h>

int main(){
    FILE* fptr;
    FILE* fptr2;
    fptr = fopen("input.txt","r");
    fptr2 = fopen("input2.txt","w");
    char ch;
    if (fptr != NULL && fptr2 != NULL){
        while((ch = fgetc(fptr)) != EOF){
            fputc(ch,fptr2);
        }
    fclose(fptr);
    fclose(fptr2);
    }

    
    return 0;
}