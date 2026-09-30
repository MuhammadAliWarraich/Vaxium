#include <stdio.h>
#include <stdbool.h>
#include <string.h>


 struct token{
        char Type[20];
        char value[100];
    };
int main(){
    FILE* fptr;
    fptr = fopen("output.txt","r");
    char ch2[1000];
    bool flag = false;
    /*1. populate the array of tokan type */
    if (fptr != NULL){
        fgets(ch2,1000,fptr);
        int count = 1;
        while (ch2 != EOF)
        {
            fgets(ch2,1000,fptr);
            count = count + 1;
        }
        struct token MyTokens[count+1];
        int tokencount = 0;
        rewind(fptr);
        fgets(ch2,sizeof(ch2),fptr);
        while(ch2 != NULL){
            for (int i =0;i<strlen(ch2);i++){
                char ch = ch2[i];
                if (ch == ','){
                    strncpy(MyTokens[tokencount].value, ch2 + (i+2),(strlen(ch2)-(i+1)));
                    strncpy(MyTokens[tokencount].Type,ch2, (i));
                    MyTokens[tokencount].value[i+1] = '\0';
                    MyTokens[tokencount].value[strlen(ch2)-(i+1)] = '\0';
                }
            }
            tokencount = tokencount+1;
            fgets(ch2,sizeof(ch2),fptr);
        }
        int count1 = 0;
        struct Node* Program = malloc(sizeof(struct Node));
        Program->children = malloc(sizeof(struct Node));
        int count = 0;
        while(count != (tokencount+1)){
            Program->children = realloc(Program->children,sizeof(struct Node)*(count1+1));
            Program->children[count1]  = stmt(MyTokens,count);
            count1++;
        }
    return 0;
    }
}
struct Node{
    struct Node** children;
    char* ctypeptr;
};
char *getToken(int tokencount, struct token* MyTokens){
    return MyTokens[tokencount].value;
}
struct Program{
    struct Node** Node;
    char* ctypeptr;
    int child_count;
};
struct Node* Declarstmt(struct token* MyTokens, int *tokencount){
    struct Node* decNode = malloc(sizeof(struct Node));
    tokencount++;
    decNode->ctypeptr = "DECLARE";
    decNode->children = malloc(sizeof(struct Node*)*2);
    bool flag = false;
    if(strcmp(MyTokens[*tokencount].Type,"IDENTIFIER")==0){
        struct Node* idNode = malloc(sizeof(struct Node));
        idNode->ctypeptr = MyTokens[*tokencount].value;
        idNode->children = NULL;
        decNode->children[0] = idNode;
        tokencount++;
        if(MyTokens[*tokencount].value == ':'){
            tokencount++;
           if (strcmp(MyTokens[*tokencount].value,"STRING") == 0 || strcmp(MyTokens[*tokencount].value,"INTEGER") == 0 || strcmp(MyTokens[*tokencount].value,"BOOLEAN") ==0 || strcmp(MyTokens[*tokencount].value,"CHAR") ==0) 
        {
            struct Node* typeNode = malloc(sizeof(struct Node));
            typeNode->ctypeptr = MyTokens[*tokencount].value;
            decNode->children[1] = typeNode;
            tokencount++;
            if(MyTokens[*tokencount].value == ';'){
                return decNode;
                flag = true;
            }
        } 
            else if (strcmp(MyTokens[*tokencount].value,"ARRAY") ==0)
            {
                decNode--->children[1] = arraydec(MyTokens,*tokencount);
                if(MyTokens[*tokencount].value == ';'){
                return decNode;
                flag = true;
            }
            } 
    }           
}
if (flag == false){
    printf("error in declaration statement");
    exit();
    return 0;
}
}
struct Node* arraydec(struct token* MyTokens, int *tokencount){
    /*arraydec <-- ARRAY [ NUMBER : NUMBER ] OF (STRING || INTEGER || CHAR );*/
    struct Node* arrdnode = malloc(sizeof(struct Node));
    arrdnode->ctypeptr = "ARRAY DECLERATION";
    arrdnode->children = malloc(sizeof(struct Node) *3);
    tokencount++;
    bool flag = false;
    if (MyTokens[*tokencount].value == '[')
    {
        tokencount++;
       if (strcmp(MyTokens[*tokencount].Type,"NUMBER")==0 )
       {
        struct Node* lowerbound = malloc(sizeof(struct Node));
        lowerbound->children = NULL;
        lowerbound->ctypeptr = MyTokens[*tokencount].value;
        arrdnode->children[0] = lowerbound;
        tokencount++;
        if(MyTokens[*tokencount].value == ':'){
            if(strcmp(MyTokens[*tokencount].Type,"NUMBER")==0){
            struct Node* upperbound = malloc(sizeof(struct Node));
            upperbound->children = NULL;
            upperbound->ctypeptr = MyTokens[*tokencount].value;
            arrdnode->children[1] = upperbound;
            tokencount++;
                if(MyTokens[*tokencount].value == ']'){
                    tokencount++;
                    if(strcmp(MyTokens[*tokencount].value,"OF")==0){
                        tokencount++;
                        if(strcmp(MyTokens[*tokencount].value,"CHAR")==0 || strcmp(MyTokens[*tokencount].value,"INTEGER")==0 || strcmp(MyTokens[*tokencount].value,"STRING")==0 || strcmp(MyTokens[*tokencount].value,"BOOLEAN")==0){
                            struct Node* typenode = malloc(sizeof(struct Node));
                            flag = true;
                            typenode->children = NULL;
                            typenode->ctypeptr = MyTokens[*tokencount].value;
                            arrdnode->children[2] = typenode;
                            return arrdnode;
                        }
                    }
                }
            }
        }
       } 
    }
    if (flag == false){
        printf("error in array declaration");
        exit();
        return 0;
    }
}
struct Node* arrayaccess(struct token* MyTokens, int *tokencount){
    struct Node* arracc = malloc(sizeof(struct Node));
    bool flag = false;
    arracc->ctypeptr = MyTokens[*tokencount -1].value;
    arracc->children = malloc(sizeof(struct Node));
    if(MyTokens[*tokencount].value == '[' && strcmp(MyTokens[*tokencount + 1].Type,"NUMBER")==0)
    {
    struct Node* subscript = malloc(sizeof(struct Node));
    subscript->ctypeptr = MyTokens[*tokencount +1].value;
    arracc->children = subscript;
    tokencount = tokencount + 2;
    if(MyTokens[*tokencount].value == ']'){
        return arracc;
        flag = true;
    }
    }
}
struct Node* parseidentifier(struct token* MyTokens, int *tokencount){
    struct Node* piden = malloc(sizeof(struct Node));
    piden->children = malloc(sizeof(struct Node*));
    if (strcmp(MyTokens[*tokencount].Type,"IDENTIFIER")==0 && strcpy(MyTokens[*tokencount+1].Type,"[") ==0)
        {
            return arrayaccess(MyTokens,tokencount+1);
        }
        else if (strcmp(MyTokens[*tokencount].Type,"IDENTIFIER")==0)
        {
            piden->ctypeptr = MyTokens[*tokencount].value;
        }
        else if (strcmp(MyTokens[*tokencount].value,"CALL")==0)
        {
            return funcCALL(MyTokens,*tokencount);
        }
        else{
            printf("ERROR");
            return NULL;

        }
        return piden;
}
struct Node* parse_term(struct token* MyTokens, int *tokencount){
    struct Node* term = malloc(sizeof(struct Node));
    term->children=malloc(sizeof(struct Node)*2);
    term->children[0] = parseidentifier( MyTokens,*tokencount);
    tokencount++;
    while(strcmp(MyTokens[*tokencount].value,"*") == 0 ||strcmp(MyTokens[*tokencount].value,"/") == 0){
        struct Node* opnode = malloc(sizeof(struct Node));
        opnode->children = malloc(sizeof(struct Node*)*2);
        if(strcmp(MyTokens[*tokencount].value,'*') == 0){
            opnode->ctypeptr = "*";
        }else{
            opnode->ctypeptr = "/";
        }
       opnode->children[0] = term;
        struct Node* term2 = malloc(sizeof(struct Node));
        term2->children = malloc(sizeof(struct Node*));
        (*tokencount)++;
        term2->children[0] = parseidentifier( MyTokens,*tokencount);
        opnode->children[1] =  term2;
        term = opnode;
        *tokencount++;
    }
    return term;
}
struct Node* parse_arithmatic(struct token* MyTokens, int *tokencount){
   struct Node* term = malloc(sizeof(struct Node));
    term->children=malloc(sizeof(struct Node)*2);
    term->children[0] = parse_term( MyTokens,*tokencount);
    tokencount++;
    while(strcmp(MyTokens[*tokencount].value,"+") == 0 ||strcmp(MyTokens[*tokencount].value,"-") == 0){
        struct Node* opnode = malloc(sizeof(struct Node));
        opnode->children = malloc(sizeof(struct Node*)*2);
        if(strcmp(MyTokens[*tokencount].value,"+") == 0){
            opnode->ctypeptr = "+";
        }else{
            opnode->ctypeptr = "-";
        }
       opnode->children[0] = term;
        struct Node* term2 = malloc(sizeof(struct Node));
        term2->children = malloc(sizeof(struct Node*));
        (*tokencount)++;
        term2->children[0] = parse_term( MyTokens,*tokencount);
        opnode->children[1] =  term2;
        term = opnode;
        *tokencount++;
    }
    return term;
}
/*expression <-- TRUE || FALSE ||" StringLiteral " || (IDENTIFIER || arrayaccess || FunctionCALL ((+ || - || * || /) IDENTIFIER || arrayaccess || FunctionCALL)+);*/
struct Node* expression(struct token* MyTokens, int *tokencount){
    struct Node* expre = malloc(sizeof(struct Node));
    bool flag = false;
    expre->ctypeptr = "Expression Start";
    if ((strcmp(MyTokens[*tokencount].Type,"IDENTIFIER")==0 && MyTokens[*tokencount+1].Type =='[') || strcmp(MyTokens[*tokencount].Type,"IDENTIFIER")==0 || strcmp(MyTokens[*tokencount].value,"CALL")==0)
    {
        expre->children = malloc(sizeof(struct Node));
        expre->children[0] = parse_arithmatic(MyTokens, *tokencount);
    }
    else if (strcmp(MyTokens[*tokencount].value,"TRUE")==0 || strcmp(MyTokens[*tokencount].value,"FALSE")==0 || MyTokens[*tokencount].value == '"')
    {
        if(MyTokens[*tokencount].value == '"' && strcmp(MyTokens[*tokencount+1].Type,"STRING LITERAL")==0 ){
            expre->children = malloc(sizeof(struct Node));
            struct Node* Strlit = malloc(sizeof(struct Node));
            Strlit->ctypeptr = MyTokens[*tokencount+1].value;
            expre->children[0] = Strlit;
        }else{
            expre->children = malloc(sizeof(struct Node));
            struct Node* boolval = malloc(sizeof(struct Node));
            boolval->ctypeptr = MyTokens[*tokencount].value;
            expre->children[0] = boolval;

        }
        
    }
    else{
        exit();
    }
    return expre;
}
/*Statement <- DECLARE stmt|| assignment stmt || print stmt || input stmt || expression stmt || Loop stmt || IF stmt || Function stmt ;*/
struct Node* Inputstmt(struct token* MyTokens, int *tokencount){
    /*struct Node* instmt = malloc(sizeof(struct Node));
    instmt->ctypeptr = "INPUT";
    instmt->children = malloc(sizeof(struct Node));
    instmt->children[0] = expression(MyTokens,*tokencount);*/
    return expression(MyTokens,*tokencount);
}
struct Node* OUTstmt(struct token* MyTokens, int *tokencount){
    return expression(MyTokens,*tokencount);
}
struct Node* ASignstmt(struct token* MyTokens, int *tokencount){
    struct Node* ASstmt = malloc(sizeof(struct Node));
    ASstmt->children = malloc(sizeof(struct Node));
    ASstmt->ctypeptr = MyTokens[*tokencount].value;
    (*tokencount)++;
    if(strcmp(MyTokens[*tokencount].value,"<--")){
        (*tokencount)++;
        ASstmt->children = expression(MyTokens,*tokencount);
        if(strcmp(MyTokens[*tokencount+1].value,";")){
            (*tokencount)++;
            return ASstmt;
        }else{
            exit();
        }
    }else{
        exit();
    }

}
struct Node* IFstmt(struct token* MyTokens, int *tokencount){
    struct Node* IFstmt = malloc(sizeof(struct Node));
    IFstmt->children = malloc(sizeof(struct Node));
    /*IF stmt <-- IF CONDITION  THEN Statement* ENDIF;*/
    (*tokencount)++;
    int count = 1;
    int state = 0;
    IFstmt->children[0] = condition(MyTokens,tokencount);
    if(strcmp(MyTokens[*tokencount].value,"THEN") ==0){
        while (strcmp(MyTokens[*tokencount].value,"ENDIF") !=0)
        {
            IFstmt->children = realloc(IFstmt->children,sizeof(struct Node)*(count+1));
            IFstmt->children[count] = stmt(MyTokens,tokencount);
            count++;
        }
        
    }else{
        exit();
    }

    return IFstmt;
}
struct Node* ELSEstmt(struct token* MyTokens, int *tokencount){
    struct Node* ELSEstmt = malloc(sizeof(struct Node));
    ELSEstmt->children = malloc(sizeof(struct Node));
    /*IF stmt <-- IF CONDITION  THEN Statement* ENDIF;*/
    (*tokencount)++;
    int count = 1;
        while (strcmp(MyTokens[*tokencount].value,"ENDIF") !=0)
        {
            ELSEstmt->children = realloc(ELSEstmt->children,sizeof(struct Node)*(count+1));
            ELSEstmt->children[count] = stmt(MyTokens,tokencount);
            count++;
        }
    return ELSEstmt;
}
struct Node* Callfunc(struct token* MyTokens, int *tokencount){
    // FunctionCALL <-- CALL IDENTIFIER((IDENTIFIER||arrayaccess (, IDENTIFIER||arrayaccess)+)+);



    
}
struct Node* FORLstmt(struct token* MyTokens, int *tokencount){
/*For loop <-- FOR IDENTIFIER "<--" NUMBER TO NUMBER statement* NEXT*/
    (*tokencount)++;
    int count = 1;
    struct Node* ForStmt = malloc(sizeof(struct Node));
    ForStmt->children = malloc(sizeof(struct Node));
    if(strcmp(MyTokens[*tokencount].Type,"IDENTIFIER")==0){
        ForStmt->children = malloc(sizeof(struct Node));
        struct Node* idenf = malloc(sizeof(struct Node));
        idenf->children = malloc(sizeof(struct Node)*2);
        idenf->ctypeptr = MyTokens[*tokencount].value;
        (*tokencount)++;
        if(strcmp(MyTokens[*tokencount].value,"<--")==0)
        {
            (*tokencount)++;
            if(strcmp(MyTokens[*tokencount].Type,"NUMBER")==0){
                struct Node* Llimit = malloc(sizeof(struct Node));
                Llimit->children = NULL;
                Llimit->ctypeptr = MyTokens[*tokencount].value;
                (*tokencount)++;
                if(strcmp(MyTokens[*tokencount].value,"TO")==0){
                    (*tokencount)++;
                    if(strcmp(MyTokens[*tokencount].Type,"NUMBER")==0){
                        struct Node* Ulimt = malloc(sizeof(struct Node));
                        Ulimt->children = NULL;
                        Ulimt->ctypeptr = MyTokens[*tokencount].value;
                        ForStmt->children[0] = idenf;
                        (*tokencount)++;
                        while (strcmp(MyTokens[*tokencount].value,"NEXT") !=0)
                        {
                            ForStmt->children = realloc(ForStmt->children,sizeof(struct Node)*(count+1));
                            ForStmt->children[count] = stmt(MyTokens,tokencount);
                            count++;
                        }
                        return ForStmt;
                    }else{
                        exit();
                    }
                }else{
                    exit();
                }
            }else{exit();}
        }else{exit();}
    }
    else{
        exit();
    }
}
struct Node* WHILstmt(struct token* MyTokens, int *tokencount){
    /*WHILE loop <-- WHILE CONDITION statement* ENDWHILE*/
    struct Node* whiloop = malloc(sizeof(struct Node));
    whiloop->children = malloc(sizeof(struct Node));
    int count = 1;
    whiloop->children[0] = condition(MyTokens,tokencount);
    while(stcmp(MyTokens[*tokencount].value,"ENDWHILE")!=0){
        whiloop->children = realloc(whiloop->children,sizeof(struct Node)*(count+1));
        whiloop->children[count] = stmt(MyTokens,tokencount);
        count++;
    }
    return whiloop;
}
struct Node* condition(struct token* MyTokens, int *tokencount){
    /*CONDITION <-- expression ((AND || OR || <> || NOT || == || <= || >= || < || > ) expression)+*/
    struct Node* condi1 = malloc(sizeof(struct Node));
    condi1->children = malloc(sizeof(struct Node));
    condi1->children[0] = brackcond(MyTokens,tokencount);
    while(strcmp(MyTokens[*tokencount].value,"AND")==0 || strcmp(MyTokens[*tokencount].value,"OR")==0 ||
     strcmp(MyTokens[*tokencount].value,"<>")==0 || strcmp(MyTokens[*tokencount].value,"NOT")==0 ||
      strcmp(MyTokens[*tokencount].value,"<")==0 || strcmp(MyTokens[*tokencount].value,">")==0 || 
      (strcmp(MyTokens[*tokencount].value,"=")==0 && strcmp(MyTokens[*tokencount+1].value,"=")==0) ||
      (strcmp(MyTokens[*tokencount].value,"<")==0 && strcmp(MyTokens[*tokencount+1].value,"=")==0) ||
      (strcmp(MyTokens[*tokencount].value,">")==0 && strcmp(MyTokens[*tokencount+1].value,"=")==0)
    ){
        struct Node* node1 = malloc(sizeof(struct Node));
        node1->children = malloc(sizeof(struct Node)*2);
        if((strcmp(MyTokens[*tokencount].value,"=")==0 && strcmp(MyTokens[*tokencount+1].value,"=")==0) ||
      (strcmp(MyTokens[*tokencount].value,"<")==0 && strcmp(MyTokens[*tokencount+1].value,"=")==0) ||
      (strcmp(MyTokens[*tokencount].value,">")==0 && strcmp(MyTokens[*tokencount+1].value,"=")==0))
        {
            node1->ctypeptr = strcat(MyTokens[*tokencount].value,MyTokens[*tokencount+1].value);
            (*tokencount)++;
            (*tokencount)++;
        }
        else if(strcmp(MyTokens[*tokencount].value,"AND")==0 || strcmp(MyTokens[*tokencount].value,"OR")==0 ||
     strcmp(MyTokens[*tokencount].value,"<>")==0 || strcmp(MyTokens[*tokencount].value,"NOT")==0 ||
      strcmp(MyTokens[*tokencount].value,"<")==0 || strcmp(MyTokens[*tokencount].value,">")==0 )
        {
             node1->ctypeptr = MyTokens[*tokencount].value;
            (*tokencount)++;

        }
        node1->children[0] = condi1;
        node1->children[1] = brackcond(MyTokens,*tokencount);
        condi1 = node1;
     }
}
struct Node* brackcond(struct token* MyTokens, int *tokencount){
    struct Node* brac = malloc(sizeof(struct Node));
    brac->children = malloc(sizeof(struct Node));
    brac->ctypeptr = "BRACKET";
    if(strcmp(MyTokens[*tokencount].value,"(")==0){
        (*tokencount)++;
        while(strcmp(MyTokens[*tokencount].value,")")!=0){
            brac->children[0] = condition(MyTokens,tokencount);
        }
        (*tokencount)++;
    }else{
        return expression(MyTokens,tokencount);
    }
}
struct Node* FUNCDstmt(struct token* MyTokens, int *tokencount){
    /*Functiondec stmt <-- FUNCTION IDENTIFIER (IDENTIFIER : <TYPE>, ...) statement* ENDFUNCTION*/
    // arraydec <-- ARRAY [ NUMBER : NUMBER ] OF (STRING || INTEGER || CHAR );

    tokencount++;
    struct Node* Func = malloc(sizeof(struct Node));
    Func->children = malloc(sizeof(struct Node));
    if(strcmp(MyTokens[*tokencount].Type,"IDENTIFIER")==0){
        struct Node* FDecstmt = malloc(sizeof(struct Node));
        FDecstmt->ctypeptr = MyTokens[*tokencount].value;
        tokencount++;
        if(strcmp(MyTokens[*tokencount].value,"(")==0 && strcmp(MyTokens[*tokencount+1].value,")")==0){
        int count = 1;
        while(stcmp(MyTokens[*tokencount].value,"ENDFUNCTION")!=0){
        Func->children = realloc(Func->children,sizeof(struct Node)*(count+1));
        Func->children[count] = stmt(MyTokens,tokencount);
        count++;
        }
        
    }
    else if(strcmp(MyTokens[*tokencount].value,"(")==0){
        tokencount++;
        
}
}
}
struct Node* stmt(struct token* MyTokens, int *tokencount){
    struct Node* stmt = malloc(sizeof(struct Node));
    if (strcmp(MyTokens[*tokencount].value,"DECLARE")==0)
    {
        stmt->ctypeptr = "Declare Statement";
        stmt->children =malloc(sizeof(struct Node));
        stmt->children[0] = Declarstmt(MyTokens, *tokencount);
    }
    else if(strcmp(MyTokens[*tokencount].value,"INPUT")==0){
        stmt->ctypeptr = "INPUT Statement";
        stmt->children =malloc(sizeof(struct Node));
        stmt->children[0] = Inputstmt(MyTokens, *tokencount);

    }
    else if(strcmp(MyTokens[*tokencount].value,"OUTPUT")==0){
        stmt->ctypeptr = "OUTPUT Statement";
        stmt->children =malloc(sizeof(struct Node));
        stmt->children[0] = OUTstmt(MyTokens, *tokencount);
    }
    else if(strcmp(MyTokens[*tokencount].value,"IF")==0){
        stmt->ctypeptr = "IF statment";
        stmt->children =malloc(sizeof(struct Node));
        stmt->children[0] = IFstmt(MyTokens, *tokencount);
    }
    else if(strcmp(MyTokens[*tokencount].value,"FOR")==0){
        stmt->ctypeptr = "For Loop";
        stmt->children =malloc(sizeof(struct Node));
        stmt->children[0] = FORLstmt(MyTokens, *tokencount);
    }
    else if(strcmp(MyTokens[*tokencount].value,"WHILE")==0){
        stmt->ctypeptr = "WHILE LOOP";
        stmt->children =malloc(sizeof(struct Node));
        stmt->children[0] = WHILstmt(MyTokens, *tokencount);
    }
    else if(strcmp(MyTokens[*tokencount].value,"FUNCTION")==0){
        stmt->ctypeptr = "FUNCTION DECLARATION Statement";
        stmt->children =malloc(sizeof(struct Node));
        stmt->children[0] = FUNCDstmt(MyTokens, *tokencount);
    }
    else if(strcmp(MyTokens[*tokencount].value,"ELSE") ==0){
        stmt->ctypeptr = "ELSE STATEMENT START";
        stmt->children = malloc(sizeof(struct Node));
        stmt->children[0] = ELSEstmt(MyTokens,*tokencount);
    }
    else if(strcmp(MyTokens[*tokencount].Type,"IDENTIFIER")){
        stmt->ctypeptr = "ASSIGNMENT Statement";
        stmt->children =malloc(sizeof(struct Node));
        stmt->children[0] = ASignstmt(MyTokens, *tokencount);  

    }
    else{
        return 1;
    }
    return stmt;

    

}



