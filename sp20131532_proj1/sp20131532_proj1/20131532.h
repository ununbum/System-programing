#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

typedef struct HISTORY{
	struct HISTORY *next;
	char data[40];
}HISTORY;

typedef struct HashTable{
	struct HashTable *next;
	char data[10];
	int opcode;
	int form;
}HashTable;
//this two structure is for opcode and history

HISTORY *head=NULL;
HashTable *hash[20];

unsigned char memory[1048576];		//unsigned char is 1byte so this memory variable is 1mbyte

int address=0;		//save start point of dump


void Helpfunc(void)			//this function print all command of this program
{
	printf("h[elp]\n");
	printf("d[ir]\n");
	printf("q[uit]\n");
	printf("hi[story]\n");
	printf("du[mp] [start, end]\n");
	printf("e[dit] address, value\n");
	printf("f[ill] start, end, value\n");
	printf("reset\n");
	printf("opcode memonic\n");
	printf("opcodelist\n");		
}

void Dirfunc(DIR *dirp,struct dirent *direntp)		//this function print all files in current director
{
	struct stat Check_Stat;
	dirp = opendir(".");
	while((direntp = readdir(dirp)) != NULL)
	{
		stat(direntp->d_name,&Check_Stat);
		if(S_ISDIR(Check_Stat.st_mode))
		{
			if(strcmp(direntp->d_name,".")!=0 && strcmp(direntp->d_name,"..")!=0)
				printf("%s/   ",direntp->d_name);
		}
		else if(Check_Stat.st_mode & S_IXUSR == S_IXUSR)
			printf("%s*   ",direntp->d_name);
		else
			printf("%s    ",direntp->d_name);
	}
	printf("\n");
}
void PrintHis(void)		//this function print history linked list
{
	int i=1;
	HISTORY *tmp;
	tmp=head;
	while(tmp!=NULL)
	{
		printf("  %d   %-10s\n",i,tmp->data);
		tmp=tmp->next;
		i++;
	}
}
void Insert_His(char *ins)		//this fuction link command to history linked list
{
	HISTORY *node,*tmp;
	node = (HISTORY *) malloc(sizeof(HISTORY));
	strcpy(node->data,ins);
	node->next=NULL;
	tmp=head;
	if(head==NULL)
		head=node;
	else
	{
		tmp= head;
		while(tmp->next!=NULL)
			tmp=tmp->next;
		tmp->next=node;
	}

}
int Hashing(char* key)		//this fuction calculate hask key by the following format : (1st word * 3rd word + 2nd word) % 20
{
	return (key[0]*key[2]+key[1])%20;
}
void Insert_Hash(int Op_hex,char * mnemonic,int Op_form)		// this function input hash tabel by using Hashing function's result for index
{
	HashTable *node,*tmp;
	int key;
	node=(HashTable*)malloc(sizeof(HashTable));

	node->opcode=Op_hex;
	strcpy(node->data,mnemonic);
	node->form=Op_form;
	key = Hashing(mnemonic);
	node->next=NULL;
	
	if(hash[key]==NULL)
		hash[key]=node;
	else
	{
		tmp= hash[key];
		while(tmp->next!=NULL)
			tmp=tmp->next;
		tmp->next=node;
	}
}

void Free_All(void)		//clear all lists
{
	HISTORY *tmp1;
	HashTable *tmp2;
	int i;
	
	tmp1=head;
	while(head!=NULL)
	{
		head=head->next;
		free(tmp1);
		tmp1=head;
	}

	for(i=0;i<20;i++)
	{
		tmp2=hash[i];
		while(hash[i]!=NULL)
		{
			hash[i]=hash[i]->next;
			free(tmp2);
			tmp2=hash[i];
		}
	}
}
HashTable * Find_opcode(char *opcode)		//this function find the command is in the hash or not.
{
	int i,key;
	HashTable * tmp=NULL;
	
	
	if(opcode==NULL)
		return tmp;
	key=Hashing(opcode);

	tmp=hash[key];

	while(tmp!=NULL)
	{
		if(strcmp(tmp->data,opcode)==0)
			return tmp;
		else
			tmp=tmp->next;
	}
	
	return tmp;
}
void Print_Hash(void)		//this function print hash table
{
	HashTable  *tmp;
	int i;

	for(i=0;i<20;i++)
	{
		printf("%d : ",i);
		tmp=hash[i];

		if(tmp!=NULL)
		{
			while(tmp->next!=NULL)
			{
				printf("[%s,%x] -> ",tmp->data,tmp->opcode);
				tmp=tmp->next;
			}
			printf("[%s,%x]",tmp->data,tmp->opcode);
		}
		printf("\n");
	}
}
void Print_dump(int start,int end)		//this function print dump by using parameter start and end, it print ' 'and '.' if it is out of range 
{
	int i,j;

	for(i=start/16;i<end/16+1;i++)
	{
		if(i>0xffff)
		{
			address=0;
			return ;
		}
		printf("%05X ",i*16);
		for(j=0;j<16;j++)
		{
			if(16*i+j<start)
				printf("   ");
			else if(end<16*i+j)
				printf("   ");
			else 
				printf(" %02X",memory[16*i+j]);
		}
		printf(" ; ");
		for(j=0;j<16;j++)
		{
			if(16*i+j<start)
				printf(".");
			else if(end<16*i+j)
				printf(".");
			else {
				if(0x20<=memory[16*i+j] && memory[16*i+j]<=0x7e)
					printf("%c",memory[16*i+j]);
				else
					printf(".");
			}
		}
		printf("\n");
	}
	address=end+1;
	if(address>0xfffff)
		address=0;
}
int Dumpfunc(char* history,char ** ptr,int num)		//this function judge all dump string using ssacanf. it depends on  sscanf's return value.
{
	int oper_num;
	int i,j;
	int start,end;
	char *buf=NULL,err_check='\0';
	
	int comma_flag=0;

	oper_num=sscanf(history,"%*s %5x %*[,c] %5x %c",&start,&end,&err_check);

	for(i=0;i<20;i++)		//if it has two or more comma in string, it is wrong command
	{
		if(history[i]==',')
			comma_flag++;
		if(comma_flag==2)
			return 1;
	}

	if(err_check!='\0')		//if some charater is behind end value, it is wrong command
		return 1;
	if(oper_num==EOF && ptr[1]=='\0')		//if sscanf's return value is not exist, it means dump command
	{
		Print_dump(address,address+159);
		return 0;
	}

	else if( oper_num==1 && ptr[2]=='\0')		//if return value is only one, it means dump start command but if start value string is not change into hex number, it is wrong command
	{
		strtol(ptr[1],&buf,16);
		if(*buf!='\0')
		{
			printf("ERROR!\n");
			return 1;
		}
		if(start>0xfffff || start<0 || strlen(ptr[1])>5)
		{
			printf("ERROR!\n");
			return 1;
		}
		Print_dump(start,start+159);
		return 0;
	}

	else if(oper_num==2)		//if return value is 2, it means dump start, end command
	{
		if(start>end || start > 0xfffff || end > 0xfffff || start<0 || end <0)
		{

			printf("ERROR!\n");
			return 1;
		}
		Print_dump(start,end);
		return 0;
	}
	printf("ERROR!\n");
	return 1;

}
int Editfunc(char * history,char **ptr,int num)		//same as control string dump start end command but function for edit only one memory
{
	int oper_num;
	int edit_address,value;
	int comma_flag=0,i;
	char err_check='\0';
	for(i=0;i<20;i++)
	{
		if(history[i]==',')
			comma_flag++;
		if(comma_flag==2)
			return 1;
	}
	oper_num=sscanf(history,"%*s %5x %*[,c] %x %c",&edit_address,&value,&err_check);

	if(err_check!='\0')
		return 1;
	if(oper_num==2)
	{
		if(edit_address < 0 || value > 0xff || value < 0)
		{
			printf("ERROR!\n");
			return 1;
		}
		memory[edit_address]=value;
		return 0;
	}
	printf("ERROR!\n");
	return 1;
}
int Fillfunc(char * history,char **ptr,int num)		// it is more one character than dump start end command, but same as controling it. function fill all memory in range of start, end by input value
{
	int i,j;
	int oper_num;
	int start,end,value;
	int comma_flag=0;
	char err_check='\0';
	oper_num=sscanf(history,"%*s %5x %*[,c] %5x %*[,c] %x %c",&start,&end,&value,&err_check);
	for(i=0;i<20;i++)
	{
		if(history[i]==',')
			comma_flag++;
		if(comma_flag==3)
			return 1;
	}
	if(err_check!='\0')
		return 1;
	if(oper_num==3)
	{
		if(start>end || value >0xff || value < 0)
		{
			printf("ERROR!\n");
			return 1;
		}
		for(i=start;i<=end;i++)
			memory[i]=value;
		return 0;
	}
 	printf("ERROR!\n");
	return 1;
}
void resetfunc(void)		//reset all memory into 0;
{
	int i;
	for(i=0;i<=0xfffff;i++)
		memory[i]=0;
}
