
#include "assmble.h"
struct symbolTable *pSymTab;
int symTabLen;
FILE *assp,*machp,*fopen();
void main(int argc,char **argv){
    int i,j,found,noInsts;
    struct instruction *currInst;
    size_t lineSize=72;
    char *line;
    char *token;
    char *instructions[]={"add","sub","slt","or","nand",
                          "addi","slti","ori","lui","lw","sw","beq","jalr",
                          "j","halt"};
    int instCnt=0;
    char hexTable[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    char lower[5];
    i=0;
    j=0;
    line=(char *)malloc(72);
    currInst=(struct instruction *)malloc(sizeof(struct instruction));
    if(argc < 3){
        printf("***** Please run this program as follows:\n");
        printf("***** %s assprog.as machprog.m\n",argv[0]);
        printf("***** where assprog.as is your assembly program\n");
        printf("***** and machprog.m will be your machine code.\n");
        exit(1);
    }
    if((assp=fopen(argv[1],"r")) == NULL){
        printf("%s cannot be opened\n",argv[1]);
        exit(1);
    }
    if((machp=fopen(argv[2],"w+")) == NULL){
        printf("%s cannot be opened\n",argv[2]);
        exit(1);
    }
    symTabLen=findSymTabLen(assp);
    pSymTab= ( struct symbolTable*)malloc(symTabLen* sizeof(struct symbolTable));
    for(int i=0;i<symTabLen;i++)
        pSymTab[i].symbol=(char *) malloc(6);
       noInsts= fillSymTab(pSymTab,assp);
     while(fgets(line,lineSize,assp) != NULL){
            token= strtok(line,"\t, ,\n");
            int i1=findInstType(token,instructions,currInst);
            int i2=0;
            if(i1==-1){
                token= strtok(NULL,"\t, ,\n");
                i2=findInstType(token,instructions,currInst);
                if(i2==-1){
                    fprintf(machp,"ERROR :Unknown code");
                    exit(1);
                }
            }
    }
    fclose(assp);
    fclose(machp);
}
int findInstType(char * token,char *instructions[],struct instruction *currInst){
    int index=-1;
    char *in=&instructions;
    for(int i=0;i<15;i++){
        if(strcmp(token,instructions[i])==0){
            index=i;
            break;
        }
    }
    if(strcmp(token,".fill")==0){
        fill(token);
        return 0;
    }
    if(strcmp(token,".space")==0){
        space(token);
        return 0;
    }
    if(index==-1)
        return index;
    else if(index<5)
        typeR(token,index,currInst);
    else if(index<13)
        typeI(token,index,currInst);
    else if(index<15){
        typeJ(token,index,currInst);
        instructions=in;
    }
    return index;
}
void fill(char * token){
    token= strtok(NULL,"\t, ,\n");
    if(isdigit(token[0]) || token[0]=='-'){
        fprintf(machp,"%d\n",atoi(token));
    }else{
        int index2= findSym2(token);
        if(index2==-1) {
           fprintf(machp,"ERROR :missing symbol");
            exit(1);
        }
        fprintf(machp,"%d\n",pSymTab[index2].value);
    }
}
void space(char * token){
    token= strtok(NULL,"\t, ,\n");
    if(isdigit(token[0])){
        fprintf(machp,"%d\n", atoi(token));
    }else{
        int index2= findSym2(token);
        if(index2==-1){
            fprintf(machp,"ERROR :missing symbol");
            exit(1);
        }
        fprintf(machp,"%d\n",pSymTab[index2].value);
    }

}
int typeR(char * token,int index,struct instruction *currInst){
    char *lower ,*rt,*rd,*rs;
    currInst->PC++;
    currInst->instType=0;
    currInst->imm=0;
    currInst->mnemonic=token;
    token= strtok(NULL,"\t, \n");
    strcpy(&rd,token);
    currInst->rd= atoi(token);
    token= strtok(NULL,"\t, \n");
    strcpy(&rs,token);
    currInst->rs= atoi(token);
    token= strtok(NULL,"\t, \n");
    strcpy(&rt,token);
    currInst->rt= atoi(token);
    strcpy(&currInst->inst[0],"x");
    strcpy(&currInst->inst[1],"0");
    int2hex16(lower,index);
    strcpy(&currInst->inst[2],&lower[3]);
    strcpy(&currInst->inst[3],&rs);
    strcpy(&currInst->inst[4],&rt);
    strcpy(&currInst->inst[5],&rd);
    strcpy(&currInst->inst[6],"0");
    strcpy(&currInst->inst[7],"0");
    strcpy(&currInst->inst[8],"0");
    currInst->intInst= hex2int(currInst->inst);
    fprintf(machp,"%d\n",currInst->intInst);
}
int typeI(char * token,int index,struct instruction *currInst){
    char *lower ,*rt,*rs="0";
    currInst->instType=1;
    currInst->imm=0;
    currInst->PC++;
    currInst->mnemonic=token;
    currInst->rs=0;
    token= strtok(NULL,"\t, ,\n");
    strcpy(&rt,token);
    currInst->rt= atoi(token);
    currInst->rd= 0;
    if(strcmp(currInst->mnemonic,"beq")==0){
        strcpy(&currInst->inst[0],"x");
        strcpy(&currInst->inst[1],"0");
        int2hex16(lower,index);
        strcpy(&currInst->inst[2],&lower[3]);
        token= strtok(NULL,"\t, ,\n");
        currInst->rs= atoi(token);
        strcpy(&rs,token);
        strcpy(&currInst->inst[3],  &rt);
        strcpy(&currInst->inst[4],  &rs);
        token= strtok(NULL,"\t, ,\n");
        if(isdigit(token[0])){
            int2hex16(&currInst->inst[5], atoi(token)-currInst->PC);
        }else{
            int index2= findSym2(token);
            if(index2==-1){
                fprintf(machp,"ERROR :missing symbol");
                exit(1);
            }
            int2hex16(&currInst->inst[5], pSymTab[index2].value-currInst->PC);
            currInst->intInst= hex2int(currInst->inst);
        }
        currInst->intInst= hex2int(currInst->inst);
        fprintf(machp,"%d\n",currInst->intInst);
        return 0;
    }
    else if(strcmp(currInst->mnemonic,"lui ")==-1){
        strcpy(&currInst->inst[0],"x");
        strcpy(&currInst->inst[1],"0");
        strcpy(&currInst->inst[2],"8");
        strcpy(&currInst->inst[3],  "0");
        strcpy(&currInst->inst[4], &rt);
        token= strtok(NULL,"\t, ,\n");
        if(isdigit(token[0])){
            int2hex16(&currInst->inst[5], atoi(token));
        }else{
            int index2= findSym2(token);
            if(index2==-1){
                fprintf(machp,"ERROR :missing symbol");
                exit(1);
            }
            int2hex16(&currInst->inst[5], pSymTab[index2].value);
        }
        currInst->intInst= hex2int(currInst->inst);
        fprintf(machp,"%d\n",currInst->intInst);
        return 0;
    }
    strcpy(&currInst->inst[0],"x");
    strcpy(&currInst->inst[1],"0");
    int2hex16(lower,index);
    strcpy(&currInst->inst[2],&lower[3]);
    token= strtok(NULL,"\t, ,\n");
    currInst->rs= atoi(token);
    strcpy(&rs,token);
    strcpy(&currInst->inst[3],  &rs);
    strcpy(&currInst->inst[4],  &rt);
    if(strcmp(currInst->mnemonic,"jalr")!=0){
            token= strtok(NULL,"\t, ,\n");
            if(isdigit(token[0])){
                int2hex16(&currInst->inst[5], atoi(token));
            }else{
                int index2= findSym2(token);
                if(index2==-1){
                    fprintf(machp,"ERROR :missing symbol");
                    exit(1);
                }
                int2hex16(&currInst->inst[5], pSymTab[index2].value);
            }
    } else{
        strcpy(&currInst->inst[5],"0");
        strcpy(&currInst->inst[6],"0");
        strcpy(&currInst->inst[7],"0");
        strcpy(&currInst->inst[8],"0");
    }
    currInst->intInst= hex2int(currInst->inst);
    fprintf(machp,"%d\n",currInst->intInst);

}
int typeJ(char * token,int index,struct instruction *currInst){
    char *lower;
    currInst->instType=2;
    currInst->imm=0;
    currInst->rd= 0;
    currInst->rs=0;
    currInst->rt=0;
    currInst->mnemonic=token;
    if(strcmp(token,"j")==0){
        currInst->PC++;
        token= strtok(NULL,"\t, ,\n");
        strcpy(&currInst->inst[0],"x");
        strcpy(&currInst->inst[1],"0");
        strcpy(&currInst->inst[2],"d");
        strcpy(&currInst->inst[3],"0");
        strcpy(&currInst->inst[4],"0");
        if(isdigit(token[0])){
            int2hex16(&currInst->inst[5], atoi(token));
        }else {
            int index2 = findSym2(token);
            if (index2 == -1) {
                fprintf(machp,"ERROR :missing symbol");
                exit(1);
            }
            currInst->imm=pSymTab[index2].value;
            currInst->PC=pSymTab[index2].value;
            int2hex16(&currInst->inst[5], pSymTab[index2].value);
        }
        currInst->intInst= hex2int(currInst->inst);
        fprintf(machp,"%d\n",currInst->intInst);
    }
    else if(strcmp(token,"halt")==0){
        strcpy(currInst->inst,"x0e000000");
        currInst->intInst= hex2int(currInst->inst);
        fprintf(machp,"%d\n",currInst->intInst);
    }

}
int findSymTabLen(FILE *inputFile){
    int count=0;
    size_t lineSize;
    char *line;
    line=(char *)malloc(72);
    while(fgets(line,lineSize,inputFile) !=NULL){
        if((line[0] == ' ') || (line[0] == '\t'));
        else
            count++;
    }
    rewind(inputFile);
    free(line);
    return count;
}
int fillSymTab(struct symbolTable *symT,FILE *inputFile){
    int lineNo=0;
    size_t lineSize=72;
    char *line;
    int i=0;
    char *token;
    line=(char *)malloc(72);
    while(fgets(line,lineSize,inputFile) != NULL){
        if((line[0] == ' ') || (line[0] == '\t'));
        else{
            if(findSym(line)>=0){
                fprintf(machp,"ERROR :duplicate symbol");
                exit(1);
            }
            else {
                token = strtok(line, "\t, ");
                strcpy(symT[i].symbol, token);
                symT[i].value = lineNo;
                i++;
            }
        }
        lineNo++;
    }
    rewind(inputFile);
    free(line);
    return lineNo;
}

int hex2int( char* hex)
{
    int result=0;
    while ((*hex)!='\0')
    {
        if (('0'<=(*hex))&&((*hex)<='9'))
            result = result*16 + (*hex) -'0';
        else if (('a'<=(*hex))&&((*hex)<='f'))
            result = result*16 + (*hex) -'a'+10;
        else if (('A'<=(*hex))&&((*hex)<='F'))
            result = result*16 + (*hex) -'A'+10;
        hex++;
    }
    return(result);
}
void int2hex16(char *lower,int a){
    if(a>65536){
        fprintf(machp,"ERROR:long offset");
        exit(1);
    }
    sprintf(lower,"%X",a);
    if(a <0x10){
        lower[4]='\0';
        lower[3]=lower[0];
        lower[2]='0';
        lower[1]='0';
        lower[0]='0';
    }
    else if(a <0x100){
        lower[4]='\0';
        lower[3]=lower[1];
        lower[2]=lower[0];
        lower[1]='0';
        lower[0]='0';
    }
    else if(a <0x1000){
        lower[4]='\0';
        lower[3]=lower[2];
        lower[2]=lower[1];
        lower[1]=lower[0];
        lower[0]='0';
    }
}
int findSym(char *token){
    char *pointer= strtok(token,"\t, ");
    for(int i=0;i<symTabLen;i++){
        if(strcmp(pointer,pSymTab[i].symbol)==0){
            return i;
        }
    }
    return -1;
}
int findSym2(char *token){
    for(int i=0;i<symTabLen;i++){
        if(strcmp(token,pSymTab[i].symbol)==0){
            return i;
        }
    }
    return -1;
}