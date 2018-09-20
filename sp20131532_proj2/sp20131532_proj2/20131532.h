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

typedef struct Symbol{
	struct Symbol *next;
	char label[10];
	int LOCCTR;
}Symbol;
//this linked list is for symbol table

HISTORY *head=NULL;
HashTable *hash[20];
Symbol *symbol=NULL;


unsigned char memory[1048576];		//unsigned char is 1byte so this memory variable is 1mbyte

int address=0;		//save start point of dump
int BASE;

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
	printf("assmble filename\n");		
	printf("type filename\n");		
	printf("symbol\n");		
}

void Dirfunc(DIR *dirp,struct dirent *direntp)		//this function print all files in current director
{
	struct stat Check_Stat;
	while((direntp = readdir(dirp)) != NULL)
	{
		stat(direntp->d_name,&Check_Stat);
		if(S_ISDIR(Check_Stat.st_mode))
		{
			if(strcmp(direntp->d_name,".")!=0 && strcmp(direntp->d_name,"..")!=0)
				printf("%s/   ",direntp->d_name);
		}
		else if((Check_Stat.st_mode & S_IXUSR) == S_IXUSR)
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
	int key;
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
	int i;
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
	int i;
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
int Insert_Symbol(char * label,int locctr)
{
	Symbol *node,*tmp,*pre;
	node=(Symbol*)malloc(sizeof(Symbol));
	if (label[0] > '0' && label[0] < '9')
		return 1;
	strcpy(node->label,label);
	node->LOCCTR=locctr;
	node->next=NULL;


	tmp=symbol;
	pre=tmp;
	if(tmp==NULL)
		symbol=node;
	else
	{
		while(tmp!=NULL)
		{
			if(strcmp(label,tmp->label)>0){
				if(tmp->next==NULL){
					tmp->next=node;
					break;
				}
				pre=tmp;
				tmp=tmp->next;
			}
			else if(strcmp(label,tmp->label)==0)
				return 1;
			else if(strcmp(label,tmp->label)<0)
			{
				node->next=tmp;
				if(strcmp(pre->label,tmp->label)!=0)
					pre->next = node;
				else
					symbol=node;
				break;
			}
		}
	}
	return 0;
}
void Type_function(char *name)		//This function type files contents
{
	char buf[102];
	FILE *fp;
	fp= fopen(name,"r");
	
	if(fp==NULL)
	{
		printf("THERE IS NO SUCH FILE!!\n");
		return;
	}
	while(fgets(buf,100,fp)!=NULL)
		printf("%s",buf);
}
void Print_Symbol(void)			//This function Print Symbol table
{
	Symbol *tmp;
	tmp=symbol;

	while(tmp!=NULL)
	{
		printf("	%s	%04X\n",tmp->label,tmp->LOCCTR);
		tmp=tmp->next;
	}

}
int Find_Symbol(char * find)		//This function Find sybol in table and return its locctr if not return -1
{
	Symbol *tmp;
	tmp=symbol;

	while(tmp!=NULL)
	{
		if(strcmp(tmp->label,find)==0)
			return tmp->LOCCTR;
		tmp = tmp->next;
	}
	return -1;
}
int Find_Register(char* reg)		//This if all register number
{
	if (strcmp(reg, "A") == 0)
		return 0;
	else if (strcmp(reg, "X") == 0)
		return 1;
	else if (strcmp(reg, "L") == 0)
		return 2;
	else if (strcmp(reg, "PC") == 0)
		return 8;
	else if (strcmp(reg, "SW") == 0)
		return 9;
	else if (strcmp(reg, "B") == 0)
		return 3;
	else if (strcmp(reg, "S") == 0)
		return 4;
	else if (strcmp(reg, "T") == 0)
		return 5;
	else if (strcmp(reg, "F") == 0)
		return 6;
	return -1;
}
int Pass2(char* filename)		//Pass 2 calculate distance from pc or base according to how long the distance from pc
{
	int option;
	int target;
	int dis;
	int code;
	int ptr;
	int i,j;
	int locctr = 0;
	int length;
	int line = 0;
	int pc=0;
	int err_flag = 0;
	int char_num=0;
	int start_address;
	int record_len = 0;
	char tmp1[30], tmp2[30];
	char buf[100];
	char label[30], opcode[30], operand[30],operand2[30];
	HashTable *tmp;
	FILE *obj,*lst,*read,*inter;

	typedef struct {		// this structure use for make record file
		int num;
		int format;
		int loc;
		int flag;
	}object;

	object record[500];


	inter = fopen(filename, "r+");
	for (i = strlen(filename); i>strlen(filename)-4; i--)
		filename[i] = '\0';
	strcat(filename,".obj");
	obj = fopen(filename, "w");
	for (i = strlen(filename); i>strlen(filename) - 4; i--)
		filename[i] = '\0';
	strcat(filename,".asm");
	read = fopen(filename,"r");
	for (i = strlen(filename); i>strlen(filename) - 4; i--)
		filename[i] = '\0';
	strcat(filename,".lst");
	lst = fopen(filename,"w");

	for (i = strlen(filename); i>strlen(filename) - 4; i--)
		filename[i] = '\0';

							//read instruction from ,asm and read pc from intermediate file(.txt) and make l.lst and .obj
	if (obj == NULL || lst == NULL)
	{
		printf("FILE Pointer ERROR!!\n");
		return 1;
	}
	
	fscanf(inter, "%x", &length);
	fprintf(obj, "H      %06X%06X",locctr, length);
	start_address = locctr;
	while (fgets(buf, 100, read) != NULL) {
		locctr = pc;
		for (i = 0; i < 30; i++)
		{
			label[i] = '\0';
			operand[i] = '\0';
			operand2[i] = '\0';
			opcode[i] = '\0';
			tmp1[i] = '\0';
			tmp2[i] = '\0';
		}
		code = 0;
		target = 0;
		dis = 0;
		option = 0;
		err_flag = 0;
		buf[strlen(buf) - 1] = '\0';
		line += 5;
		if (buf[0] == '.')
		{
			fprintf(lst, "%-5d       %-60s\n", line, buf);
			continue;
		}

		for (i = 0; i < strlen(buf); i++)
		{
			if (buf[i] == ' ' || buf[i] == '\t')
				break;
		}
		if (i > 0)
			sscanf(buf, "%s %s %[^,]%*c %s", label, opcode, operand, operand2);
		else
			sscanf(buf, "%s %[^,]%*c %s", opcode, operand, operand2);


		if (strcmp(opcode, "START") == 0) {
			fscanf(inter, "%s %s %x", tmp1,tmp2,&locctr);
			fseek(obj,0, 0);
			fprintf(obj, "H%-6s%06X%06X", tmp1, locctr, length);
			fscanf(inter, "%x", &pc);
			start_address = locctr;
			fprintf(lst, "%-5d%04X   %s\n", line, locctr, buf);
			continue;
		}
		if (strcmp(opcode, "BASE") == 0) {
			fprintf(lst, "%-5d       %-s\n", line, buf);
			continue;
		}
		else if (strcmp(opcode, "END") == 0) {			//if opcode is END this is the end of program
			fprintf(lst, "%-5d       %-s\n", line, buf);
			record[char_num].format = -1;
			record[char_num].flag = 1;
			record[char_num].loc = start_address;
		
			break;
		}
		
		fscanf(inter, "%X", &pc);
		strcpy(tmp1, opcode);
		if (opcode[0] == '+')
		{
			for (i = 0; i<strlen(opcode); i++)
				tmp1[i] = opcode[i+1];
		}
		tmp=Find_opcode(tmp1);
		if (tmp != NULL)		//according to opcode form and distance, do certain operation 
		{
			switch (tmp->form) {
			case 1:
				code += tmp->opcode;
				fprintf(lst, "%-5d%04X   %-30s%02X\n", line, locctr, buf, code);
				record[char_num].num = code;
				record[char_num].format = 1;
				record[char_num].loc = locctr;
				record[char_num].flag = 0;
				char_num++;
				break;
			case 2:
				if (operand2[0] != '\0')
				{
					if (Find_Register(operand) == -1 || Find_Register(operand2) == -1) {
						err_flag = 1;
						break;
					}
						code += Find_Register(operand)*16;
						code += Find_Register(operand2);
				}
				else if(operand[0] !='\0')
				{
					if (Find_Register(operand) == -1) {
						err_flag = 1;
						break;
					}
					code += Find_Register(operand)*16;
				}
				code += tmp->opcode * 16 * 16;
				fprintf(lst, "%-5d%04X   %-30s%04X\n", line, locctr, buf, code);
				record[char_num].num = code;
				record[char_num].format = 2;
				record[char_num].loc = locctr;
				record[char_num].flag = 0;
				char_num++;
				break;
			case 3:
				if (opcode[0] == '+')
					code += tmp->opcode * 16 * 16 * 16 * 16 * 16 * 16;
				else
					code += tmp->opcode * 16 * 16 * 16 * 16;

				strcpy(tmp2, operand);
				if (strcmp(operand2,"X")==0)
					option += 8 * 16 * 16 * 16;

				if (operand[0] == '#' || operand[0] == '@')
				{
					for (i = 0; i<strlen(operand); i++)
						tmp2[i] = operand[i + 1];
				}
				target = Find_Symbol(tmp2);
				if (target!=-1)
				{
					if (target - pc<2048 && target - pc>-2049) {
						option += 2 * 16 * 16 * 16;
						dis = target - pc;
						if (dis < 0)
							dis = (unsigned)dis % 0x1000;
					}

					else if ((unsigned)(target - BASE) < 4097 )
					{
						option += 4 * 16 * 16 * 16;
						dis = (unsigned)(target - BASE);
					}

					if (operand[0] == '#') {
						option += 1 * 16 * 16 * 16 * 16;
					}

					else if (operand[0] == '@') {
						option += 2 * 16 * 16 * 16 * 16;

					}
					else
					{
						option += 3 * 16 * 16 * 16 * 16;

					}
					if (opcode[0] == '+')
					{
						option *= 16 * 16;
						option += 1 * 16 * 16 * 16 * 16 * 16;
						if (operand[0] != '#')
							dis = target;
					}

					code += dis + option;
					record[char_num].flag = 0;
				}
				else if (strcmp(opcode, "RSUB") == 0){
					code = 0x4f0000;
					if(operand[0]!='\0')
						err_flag=1;
				}
				else if (target == -1) {
					if (operand[0] == '#') {
						ptr = sscanf(tmp2, "%d", &dis);
						if (ptr == 1) {
							option = 0;
							err_flag = 0;
							record[char_num].flag = 2;
						}
						option += 1 * 16 * 16 * 16 * 16;
						if (opcode[0] == '+') {
							option *= 16 * 16;
							option += 1 * 16 * 16 * 16 * 16 * 16;
						}
						code += dis + option;
					}
					else
						err_flag = 1;
				}
				fprintf(lst, "%-5d%04X   %-30s%06X\n", line, locctr, buf, code);
				record[char_num].num = code;
				record[char_num].loc = locctr;
				if (opcode[0] != '+')
					record[char_num].format = 3;
				else if(opcode[0] =='+')
					record[char_num].format = 4;
				char_num++;
				break;
			}
			
		}
		else

			err_flag=1;
		// From now on, this is for variables and constants, constants make object code, but variables are not
		if (operand[0] == 'C' && strcmp(opcode, "BYTE") == 0 && operand[1] == '\'' && operand[strlen(operand) - 1] == '\'') {
			code = 0;
			err_flag = 0;
			fprintf(lst,"%-5d%04X   %-30s",line,locctr,buf);
			for (i = 2; i < strlen(operand) - 1; i++)
			{
				code += (int)operand[i];
				record[char_num].format = 1;
				record[char_num].loc = locctr;
				record[char_num].num = (int)operand[i];
				record[char_num].flag = 0;
				char_num++;
				locctr += (i-2) ;
				fprintf(lst,"%X",code);
				code=0;
			}
			fprintf(lst, "\n");
			
		}
		if (operand[0] == 'X' && strcmp(opcode, "BYTE") == 0 && operand[1] == '\'' && operand[strlen(operand) - 1] == '\'') {
			code = 0;
			err_flag = 0;
			for (i = 2; i < strlen(operand) - 1; i++)
				tmp1[i - 2] = operand[i];
			tmp1[i - 2] = '\0';
			sscanf(tmp1, "%X", &code);
			record[char_num].format = 1;
			record[char_num].loc = locctr;
			record[char_num].num = code;
			record[char_num].flag = 0;
			char_num++;
			fprintf(lst, "%-5d%04X   %-30s%02X\n", line, locctr, buf, code);
		}
		if (strcmp(opcode, "WORD") == 0)
		{
			code = 0;
			err_flag = 0;
			for (i = 0; i < strlen(operand); i++)
				tmp1[i] = operand[i];
			tmp1[i] = '\0';
			sscanf(tmp1, "%X", &code);
			record[char_num].format = 3;
			record[char_num].loc = locctr;
			record[char_num].num = code;
			record[char_num].flag = 0;
			char_num++;
			fprintf(lst, "%-5d%04X   %-30s%06X\n", line, locctr, buf, code);
		}
		if (strcmp(opcode, "RESW") == 0) {
			fprintf(lst, "%-5d%04X   %-s\n", line, locctr, buf);
			record[char_num].flag = 1;
			if (record[char_num - 1].flag != 1)
				char_num++;
			continue;
		}
		else if (strcmp(opcode, "RESB") == 0) {
			fprintf(lst, "%-5d%04X   %-s\n", line, locctr, buf);
			record[char_num].flag = 1;
			if (record[char_num - 1].flag != 1)
				char_num++;
			continue;
		}
		if (err_flag == 1)
		{
			printf("ERROR ON LINE %d : %s\n", line, buf);
			strcat(filename,".lst");
			remove(filename);
			for (i = strlen(filename); i>strlen(filename) - 4; i--)
				filename[i] = '\0';
			strcat(filename, ".obj");
			remove(filename);
			for (i = strlen(filename); i>strlen(filename) - 4; i--)
				filename[i] = '\0';
			strcat(filename, ".txt");
			remove(filename);
			fclose(lst);
			fclose(inter);
			fclose(obj);
			fclose(read);
			symbol=NULL;
			return 1;
		}
	}

	for (i=0;i<500;i++)	//make record file , if charter is over 60 record next line, and if variable line coms, record next line too
	{
		if (record[i].format == -1) {
			fprintf(obj, "%02X", record_len);
			for (; j < i; j++) {
				if (record[j].format == 1)
					fprintf(obj, "%02X", record[j].num);
				else if (record[j].format == 2)
					fprintf(obj, "%04X", record[j].num);
				else if (record[j].format == 3)
					fprintf(obj, "%06X", record[j].num);
				else if (record[j].format == 4)
					fprintf(obj, "%08X", record[j].num);
			}

			break;
		}
		if (record_len == 0)
		{
			fprintf(obj, "\nT%06X", record[i].loc);
			j = i;
		}
		
			record_len += record[i].format;
			if (record_len > 0x1e || record[i].flag == 1)
			{
				record_len -= record[i].format;
				fprintf(obj, "%02X", record_len);
				for (; j < i; j++) {
					if(record[j].format==1)
						fprintf(obj, "%02X", record[j].num);
					else if (record[j].format == 2)
						fprintf(obj, "%04X", record[j].num);
					else if (record[j].format == 3)
						fprintf(obj, "%06X", record[j].num);
					else if (record[j].format == 4)
						fprintf(obj, "%08X", record[j].num);
				}
				i = j-1;
				if (record[j].flag == 1)
					i++;
				record_len = 0;
			}
				
	}
	for (i = 0; i < 500; i++)	//record Modification record for absolute address and record end record at the end
	{
		if ((record[i].format == 4) && (record[i].flag!=2))
		{
			if (record[i].loc)
				fprintf(obj, "\nM%06X05", record[i].loc - start_address+1);
		}
		if (record[i].format == -1){
			fprintf(obj, "\nE%06X\n", start_address);
			break;
		}
	}

	printf("	output file : [%s.lst], [%s.obj]\n",filename,filename);

	strcat(filename,".txt");
	remove(filename);
	fclose(lst);
	fclose(obj);
	fclose(inter);
	fclose(read);
	return 0;
}
int Pass1(char *filename)		//make all loccation counter by checking label and opcode form
{	
	int ptr;
	int i,j;
	int locctr=0;
	int cur_locctr;
	int line=0;
	int real_operand;
	char file[30],exten[30];
	char dot;
	char buf[100];
	char label[30],opcode[30],operand[30];
	char BASE_loc[30];
	HashTable *tmp;
	FILE *souce,*inter;


	ptr=sscanf(filename,"%[^.] %c %s",file,&dot,exten);

	if(ptr==3 && dot=='.')
	{
		if(strcmp(exten,"asm")==0)
		{
			strcat(file,".txt");
			souce=fopen(filename,"r");
			inter=fopen(file,"w+");

		}
		else
		{
			printf("FILE OPEN ERROR!\n");
			return 1;
		}
	 	if(souce == NULL || inter == NULL) {
			printf("FILE Pointer ERROR!!\n");
			return 1;
		}
	}		
	else
	{
		printf("FILE OPEN ERROR!\n");
		return 1;
	}
	
	while(fgets(buf,100,souce)!=NULL){
		line+=5;
		for(i=0;i<30;i++)
		{
			label[i]='\0';
			operand[i]='\0';
			opcode[i]='\0';
		}
		buf[strlen(buf)-1]='\0';
		if(buf[0]=='.')
		{
			continue;
		}
		
		for(i=0;i<strlen(buf);i++)
		{
			if(buf[i]==' ' || buf[i]=='\t')
				break;
		}
		if(i>0)
		{
			sscanf(buf,"%s %s %s",label,opcode,operand);
			if (strcmp(opcode, "START") == 0) {
				sscanf(operand, "%x", &locctr);
				cur_locctr = locctr;
				fprintf(inter, "%04X    %s\n", locctr, buf);
				continue;
			}
			else {
				if (Insert_Symbol(label, locctr)==1)
				{
					printf("ERROR ON LINE %d : %s\n", line, buf);
					fclose(souce);
					fclose(inter);
					symbol=NULL;
					return 1;
				}
			}
		}
		else
			sscanf(buf,"%s %s",opcode,operand);
		
		if(strcmp(opcode,"BASE")==0){		//BASE is reserved word so it has no object code but memory the sBASE locctr
			strcpy(BASE_loc, operand);
			continue;
		}
		else if(strcmp(opcode,"END")==0){
			fprintf(inter,"        %s\n",buf);
			fseek(inter,0,0);
			fprintf(inter,"%4X",locctr);
			BASE = Find_Symbol(BASE_loc);
			fclose(souce);
			fclose(inter);
			return Pass2(file);
		}
		cur_locctr=locctr;
		if(opcode[0]=='+')
		{
			
			locctr+=1;
			for(i=0;i<strlen(opcode);i++)
				opcode[i]=opcode[i+1];
		}
		tmp = Find_opcode(opcode);		//form means byte number
		if(tmp!=NULL)
		{
			
			switch(tmp->form){
			case 1:
				locctr+=1;
				break;
			case 2:
				locctr+=2;
				break;
			case 3:
				locctr+=3;
				break;
			}
		}
		else
		{
			if(strcmp(opcode,"RESB")==0)		
			{
				
				real_operand=atoi(operand);
				locctr+=1*real_operand;
			}
			else if(strcmp(opcode,"RESW")==0)		//variables need room for numbe of variable
			{
				
				real_operand=atoi(operand);
				locctr+=3*real_operand;
			}
			else if(strcmp(opcode,"BYTE")==0)		//make room for size of byte
			{
				
				if(operand[0]=='C')
				{
					j=0;
					for(i=2;i<strlen(operand)-1;i++){
						j++;
					}
					locctr+=1*j;
				}
				else if(operand[0]=='X')
				{
					for(i=2;i<strlen(operand)-1;i++){
					}
					locctr+=1;
				}
			}
			else if (strcmp(opcode,"WORD") == 0)
			{
				locctr += 3;
			}
			
			else
			{
				printf("ERROR ON LINE %d : %s\n",line,buf);
				remove(file);
				fclose(souce);
				fclose(inter);
				symbol=NULL;
				return 1;
			}
			
			
		}
		

		fprintf(inter,"%04X\n",cur_locctr);
		
	}
	
	printf("THRER IS NO 'END' OPCODE\n");
	symbol=NULL;
	return 1;
}
