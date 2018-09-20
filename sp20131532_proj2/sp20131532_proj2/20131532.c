#include "20131532.h"

int main()
{
	DIR *dirp;	//for directory files open
	FILE *fp;	//for open text File
	struct dirent *direntp;	//	searching directory
	char ins[32];	//get command and trim command
	char history[30];
	char mnemonic[10],format[4];		//get text string
	char *ptr[20];	// to use strtok functions
	int i=0;		//searching index
	int num=0;	//check the word numbers
	int op_hex;	//show opcode

	HashTable *tmp;
	for(i=0;i<20;i++)
		hash[i]=NULL;

	if((fp=fopen("opcode.txt","r"))==NULL)	//check the file is exist
	{
		printf("There is no File\n");
		return 0;
	}
	while(fscanf(fp,"%x %s %s",&op_hex,mnemonic,format)!=EOF){	//read text and write to variable
		Insert_Hash(op_hex,mnemonic,format[0]-'0');
		for(i=0;i<10;i++)
			mnemonic[i]=0;
	}
	fclose(fp);

	dirp=opendir(".");
	direntp =NULL;
	for(;;)
	{


		printf("sicsim>");

		fgets(ins,30,stdin);		

		if(strlen(ins)>=29)		// if input is more than 30 clear buffer
			while(getchar()!='\n');

		ins[strlen(ins)-1]='\0';	//last string input must be \n so exchange to \0

		strcpy(history,ins);

		ptr[0]=strtok(ins," \t");		//tokenize string use ' ' and '\t' as delimeter
		while(ptr[num]!=NULL)		
		{
			num++;
			ptr[num]=strtok(NULL," \t");

		}
		if(ptr[0]==NULL)
			continue;		//ptr store string wiht tokenized

		/* From here execute command that has each functions*/



		if(((strcmp(ptr[0],"q")==0 && ptr[1]==NULL) ||( strcmp(ptr[0],"quit")==0 && ptr[1]==NULL)))
			break;
		else if(((strcmp(ptr[0],"h")==0 && ptr[1]==NULL) ||( strcmp(ptr[0],"help")==0 && ptr[1]==NULL)))
		{
			Helpfunc();
			Insert_His(history);
		}
		else if(((strcmp(ptr[0],"d")==0 && ptr[1]==NULL) ||( strcmp(ptr[0],"dir")==0 && ptr[1]==NULL)))
		{

			Dirfunc(dirp,direntp);
			Insert_His(history);
		}
		else if(((strcmp(ptr[0],"hi")==0 && ptr[1]==NULL)|| (strcmp(ptr[0],"history")==0 && ptr[1]==NULL)))
		{
			Insert_His(history);
			PrintHis();
		}
		else if((strcmp(ptr[0],"du")==0) || strcmp(ptr[0],"du ")==0 || (strcmp(ptr[0],"dump")==0))
		{
			if(Dumpfunc(history,ptr,num)==0)		//if dump is wrong command, not put into history list
				Insert_His(history);
		}
		else if(strcmp(ptr[0],"e")==0 || strcmp(ptr[0],"edit")==0)
		{
			if(!Editfunc(history,ptr,num))			//same as dump
				Insert_His(history);
		}
		else if(strcmp(ptr[0],"f")==0 || strcmp(ptr[0],"fill")==0)
		{
			if(!Fillfunc(history,ptr,num))
				Insert_His(history);
		}
		else if(strcmp(ptr[0],"reset")==0 && ptr[1]==NULL)
		{
			resetfunc();
			Insert_His(history);
		}
		else if(strcmp(ptr[0],"opcodelist")==0 && ptr[1]==NULL)
		{
			Print_Hash();
			Insert_His(history);
		}
		else if(strcmp(ptr[0],"opcode")==0)
		{
			tmp=Find_opcode(ptr[1]);
			if(tmp!=NULL){
				printf("opcode is %X\n",tmp->opcode);
				Insert_His(history);
			}
		}
		else if(strcmp(ptr[0],"type")==0)
		{
			Type_function(ptr[1]);
			Insert_His(history);
		}
		else if(strcmp(ptr[0],"assemble")==0)
		{
			if(Pass1(ptr[1])==0)
				Insert_His(history);
		}
		else if(strcmp(ptr[0],"symbol")==0)
		{
			Print_Symbol();
			Insert_His(history);
		}
			
					
		for(i=0;i<num;i++)		//intialized all parameters
				ptr[i]=NULL;
		num=0;
		for(i=0;i<20;i++)
			history[i]='\0';
		for(i=0;i<32;i++)
			ins[i]='\0';		
	}	
	Free_All();
	
}
