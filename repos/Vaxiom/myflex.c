#include <stdio.h>
#include <stdbool.h>
#include <string.h>
int main(){
    FILE* fptr;
    FILE* fptr2;
    fptr = fopen("input.txt","r");
    fptr2 = fopen("output.txt","w");
    char ch;
    int state;
    state = 0;
    bool flag = false;
    char ch2[100] = "";
    if (fptr != NULL && fptr2 != NULL){
        while (flag == false){
            ch = fgetc(fptr);
            if (state == 1){
                if ((ch>= 'A' && ch<='Z') || (ch>= '0' && ch<= '9') || (ch == '_') || (ch>= 'a' && ch<= 'z'))
                {
                    strncat(ch2,&ch,1);
                }else{
                    state = 0;
                    if (strcmp(ch2,"IF") ==0 || strcmp(ch2,"ENDIF") ==0 || strcmp(ch2,"ELSE") ==0 || strcmp(ch2,"DECLARE") ==0 || strcmp(ch2,"ARRAY") ==0 || strcmp(ch2,"STRING") ==0 || strcmp(ch2,"CHAR") ==0|| strcmp(ch2,"INTEGER") ==0 || strcmp(ch2,"STRING") ==0 || strcmp(ch2,"FOR") ==0 || strcmp(ch2,"NEXT") ==0 || strcmp(ch2,"OF") ==0 || strcmp(ch2,"TO") ==0 || strcmp(ch2,"BOOLEAN") ==0 || strcmp(ch2,"WHILE") ==0 || strcmp(ch2,"ENDWHILE") ==0 || strcmp(ch2,"FUNCTION") ==0 || strcmp(ch2,"ENDFUNCTION") ==0 || strcmp(ch2,"CALL") ==0)
                    {
                       fputs("KEY WORD, ",fptr2);
                       fputs(ch2,fptr2);
                       fputc('\n',fptr2);
                       ungetc(ch,fptr);
                    }else{
                    if (ch != EOF) {ungetc(ch,fptr);}
                    fputs("IDENTIFIER, ",fptr2);
                    fputs(ch2,fptr2);
                    fputc('\n',fptr2);
                    }
                }  
            }
            else if (state == 2)
            {
                if(ch>='0' && ch<='9'){
                    strncat(ch2,&ch,1);
                }else{
                    state = 0;
                    fputs("NUMBER, ",fptr2);
                    fputs(ch2,fptr2);
                    fputc('\n',fptr2);
                    if (ch != EOF) ungetc(ch,fptr);
                }
            }
            else if (state == 3){
                if (ch=='\n' || ch=='\t' || ch=='\r' || ch==' '){

                }else{
                    state = 0;
                    if (ch != EOF) ungetc(ch,fptr);
                }
            }
            else if (state == 4){
                if (ch == '-' || ch == '>'){
                    strncat(ch2,&ch,1);
                    if (strcmp(ch2,"<--") ==0 || strcmp(ch2,"<>") ==0){
                        state =0;
                        fputs("OPPERATOR, ",fptr2);
                        fputs(ch2,fptr2);
                        fputc('\n',fptr2);
                    }
                }
                else{ 
                    state = 0;
                    if (ch != EOF) {ungetc(ch,fptr);}
                    if(strcmp(ch2,"<-") ==0){
                        fputs("OPPERATOR, ",fptr2);
                        fputc('<',fptr2);
                        fputs("OPPERATOR, ",fptr2);
                        fputc('-',fptr2);
                    }else{
                    fputs("OPPERATOR, ",fptr2);
                    fputs(ch2,fptr2);
                    fputc('\n',fptr2);
                    }
            }
        }
            else if(state == 5){
                if (ch == '\\')
                {
                    ch = fgetc(fptr);
                    strncat(ch2,&ch,1);
                }
                else if (ch !='"')
                {
                    strncat(ch2,&ch,1);

                }else{
                    state = 0;
                    fputs("StringLiteral, ",fptr2);
                    fputs(ch2,fptr2);
                    fputc('\n',fptr2);
                }
                
                
            }
            else if (state == 0){
                ch2[0] = '\0';
                if (ch>='A' && ch<='Z'){
                    state = 1;
                    ch2[0] = ch;
                    ch2[1] = '\0';
                }
                else if (ch>='0' && ch<='9'){
                    state = 2;
                    ch2[0] = ch;
                    ch2[1] ='\0';
                    
                }
                else if (ch=='\n' || ch=='\t' || ch=='\r' || ch==' ' )
                {
                    state = 3;
                }
                else if (ch==':' || ch==';'|| ch=='+'|| ch=='-' || ch=='<' || ch=='>' || ch=='=' || ch=='(' || ch==')' || ch==']' || ch=='[' || ch=='\'')
                {
                    state = 4;
                    ch2[0] = ch;
                    ch2[1] = '\0';
                }  
                else if (ch=='"'){
                    state =5;
                
                }
                else if(ch != EOF){
                    fprintf(fptr2,"ERROR, %c",ch);
                    fprintf(fptr2,"\n");
                }
            }
            if(ch== EOF){
                flag = true;
            }
            
        }
    }
}
/* [A-Z][a-zA_Z]
"IF"|"ENDIF"|"FOR"|"NEXT"|"INPUT"|"DECLARE"|"ARRAY"|"INTEGER"|"OF"|"TO"   {fprintf(yyout, "KEYWORD, %s\n",yytext);}
":"|"+"|"-"|"<>"|"<"|">"|"="|"<--"                                        {fprintf(yyout, "OPPERATOR, %s\n",yytext);}
[0-9]+                                                                    {fprintf(yyout, "NUMBER, %s\n",yytext);}
[A-Z][A-Za-z0-9_]*                                                        {fprintf(yyout, "IDENTIFIER, %s\n",yytext);}
[\n\t\r ]+  
IfIfif
*/
