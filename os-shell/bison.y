%{
	#include"global.h"
    int yylex ();
    void yyerror ();
	SimpleCmd* newSimpleCmd();
    int k,k2,done;
	SimpleCmd *cmd,*cmd2;
    int commandDone,error;
%}

%token STRING END

%%
line            :   /* empty */	END	{error=0;return;}
                    |command  	END	{if(error==0) {cmd->args[k]=NULL; if(isPipe){cmd2->args[k2]=NULL;	pipeCmd(cmd,cmd2);}
														else{	execSimpleCmd(cmd); }
														commandDone = 1;
														} 
											error = 0; return; }
;

command         :   fgCommand 
                    |fgCommand '&' {cmd->isBack=1;}
;

fgCommand       :   simpleCmd 
					|simpleCmd '|' simpleCmd 
;

simpleCmd       :   progInvocation inputRedirect outputRedirect{done = 1;}
;

progInvocation  :   STRING args{	if(isPipe){cmd2 = newSimpleCmd();k2=1;
											cmd2->args[0] = (char*)malloc(sizeof(char) * strlen($1));
										strcpy(cmd2->args[0],$1);
										}
									if(!isPipe | !done){
										cmd->args[0] = (char*)malloc(sizeof(char) * strlen($1));
										strcpy(cmd->args[0],$1);
										}
							   }
;

inputRedirect   :   /* empty */
                    |'<' STRING {	if(isPipe){
									cmd2->input = (char*)malloc(sizeof(char) * strlen($2));
									strcpy(cmd2->input,$2);}
									if(!isPipe | !done){
									cmd->input = (char*)malloc(sizeof(char) * strlen($2));
									strcpy(cmd->input,$2);
									}
								}
;

outputRedirect  :   /* empty */
                    |'>' STRING {if(isPipe){
									cmd2->output = (char*)malloc(sizeof(char) * strlen($2));
									strcpy(cmd2->output,$2);}
									if(!isPipe | !done){
									cmd->output = (char*)malloc(sizeof(char) * strlen($2));
									strcpy(cmd->output,$2);
									}
								}
;

args            :   /* empty */
                    |args STRING {	if(isPipe){cmd2->args[k2] = (char*)malloc(sizeof(char) * strlen($2));
										strcpy(cmd2->args[k2],$2);k2++;}
								  	if(!isPipe | !done){cmd->args[k] = (char*)malloc(sizeof(char) * strlen($2));
										strcpy(cmd->args[k],$2);k++;
											}

								}
;

%%


/****************************************************************
                  错误信息执行函数
****************************************************************/
void yyerror()
{	
	if(error==0){
   		 printf("你输入的命令不正确，请重新输入！\n");
	}
    error = 1;
}

/****************************************************************
                  main主函数
****************************************************************/
SimpleCmd* newSimpleCmd(){
	SimpleCmd *cmd;
	cmd = (SimpleCmd*)malloc(sizeof(SimpleCmd));
	cmd->input = cmd->output = NULL;
	cmd->args = (char**)malloc(sizeof(char*) * 101);
	return cmd;
}
void freeCmd(SimpleCmd* cmd){
	int i;
	for(i = 0; cmd->args[i] != NULL; i++){
		free(cmd->args[i]);
		free(cmd->input);
		free(cmd->output);
	    }
		free(cmd);
}

int main(int argc, char** argv) {
     int i;
	 char c;

    init(); //初始化环境
    commandDone = 0;
	
    
    printf("yourname@computer:%s$ ", get_current_dir_name()); //打印提示符信息


    while(1){	
		
		cmd = newSimpleCmd();
		cmd->isBack = 0;
		isPipe = 0;
		k=1;	
		done = 0;	
		
        yyparse(); //调用语法分析函数，该函数由yylex()提供当前输入的单词符号
	
        if(commandDone == 1){ //命令已经执行完成后，添加历史记录信息
            commandDone = 0;
            addHistory(inputBuff);
        }

	freeCmd(cmd);
	if(isPipe){freeCmd(cmd2);}
	usleep(100);
    if(error==0){
        	printf("yourname@computer:%s$ ", get_current_dir_name()); //打印提示符信息
			if((c=getchar())!=-1);{
			ungetc(c,stdin);
			}
	}
		inputBuff[0]=0;
     }

    return 0;
}
