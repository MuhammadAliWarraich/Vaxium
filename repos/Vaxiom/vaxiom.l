%{

    #include <stdio.h>


%}

%option noyywrap
%%
"IF"|"ENDIF"|"FOR"|"NEXT"|"INPUT"|"DECLARE"|"ARRAY"|"INTEGER"|"OF"|"TO"   {fprintf(yyout, "KEYWORD, %s\n",yytext);}
":"|"+"|"-"|"<>"|"<"|">"|"="|"<--"                                        {fprintf(yyout, "OPPERATOR, %s\n",yytext);}
[0-9]+                                                                    {fprintf(yyout, "NUMBER, %s\n",yytext);}
[A-Z][A-Za-z0-9_]*                                                        {fprintf(yyout, "IDENTIFIER, %s\n",yytext);}
[\n\t\r ]+                                                                 {}
.                                                                         {fprintf(yyout, "UNKNOWN, %s\n",yytext);}
%%

int main() {
    // Open input.txt for reading
    yyin = fopen("input.txt", "r");
    if (yyin == NULL) {
        printf("Error: Could not open input.txt\n");
        return 1;
    }

    // Open output.txt for writing
    yyout = fopen("output.txt", "w");
    if (yyout == NULL) {
        printf("Error: Could not create output.txt\n");
        fclose(yyin);
        return 1;
    }

    // Run the lexer
    yylex();

    // Clean up and close the files smoothly
    fclose(yyin);
    fclose(yyout);
    
    printf("Tokenization complete! Check output.txt for results.\n");
    return 0;
}