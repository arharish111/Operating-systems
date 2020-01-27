/*
 Name:Harish Harish

*/
#define _GNU_SOURCE

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <errno.h>



#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 4     // Max arguments

#define FORWARD_SLASH "/"       //Change directory delimiter



uint16_t  BPB_RsvdSecCnt;
uint16_t  BPB_BytesPerSec;
uint8_t   BPB_SecPerClus;
uint32_t  FATSz32;
uint8_t   BPB_NumFATS;
uint32_t  rootClusterAddress;
char      BS_VolLab[11];

FILE *fp;
int fileOpenCheck = -1;

	
struct __attribute__((__packed__)) DirectoryEntry
{
	char     DIR_Name[11];
	uint8_t  DIR_Attr;
	uint8_t  Unused1[8];
	uint16_t DIR_FirstClusterHigh;
	uint8_t  Unused2[4];
	uint16_t DIR_FirstClusterLow;
	uint32_t DIR_FileSize;
};
	
struct DirectoryEntry dir[16];

int LBAToOffset(int32_t sector);
int16_t NextLB( uint32_t sector);
void toSetRootClusterAddressAndPrintInfo(char *cmd); //This function will set the rootClusterAddress 
                                                     // variable and display contents of root cluster if command is "info".

void toReadAndDisplayTheContensofBlock (char *cmd); //This function will read the contents of block 
                                                    //and display the file names if command is "ls".

char* generateFAT_fileNameConvention(char *str); //This function will convert user input file name to 
                                                 //file name convention followed by FAT file system.


int main()
{
   int index;

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }
	
	if(token[0] != NULL)
    {
		
		if(!strcmp(token[0],"open"))
		{
			
			fp = fopen(token[1],"r");
			
			if(fp==NULL) 
			{
				if (errno == 2)
					printf("Error: File system image not found.\n");
			    else if (errno == 14)
					printf("Error: Please specify file system image.\n");
				else
					printf("errno:%d",errno);
			}
			else
			{
				fileOpenCheck += 1;
				toSetRootClusterAddressAndPrintInfo(token[0]);
				if (fileOpenCheck > 0)
				{
					printf("Error: File system image already open.\n");
				}
			}				
		}
		else if(!strcmp(token[0],"close"))
		{
			if(fileOpenCheck < 0)
			{
				printf("Error: File system not open.\n");
			}
			else
			{
				fileOpenCheck = -1;
				fclose(fp);
			}
		}
		else if(!strcmp(token[0],"info"))
		{
			if(fileOpenCheck < 0)
			{
				printf("Error: File system image must be opened first.\n");
			}
			else
			{
				toSetRootClusterAddressAndPrintInfo(token[0]);
			}
		}
		else if(!strcmp(token[0],"ls"))
		{
			if(fileOpenCheck < 0)
			{
				printf("Error: File system image must be opened first.\n");
			}
			else
			{
				toReadAndDisplayTheContensofBlock(token[0]);
			}
		}
		else if(!strcmp(token[0],"cd"))
		{
			if(fileOpenCheck < 0)
			{
				printf("Error: File system image must be opened first.\n");
			}
			else
			{
				int fileFoundCheck = 0;
				char *cdTokens[100],i;
				token_count =0;
				toReadAndDisplayTheContensofBlock(token[0]);
				while ( (arg_ptr = strsep(&token[1], FORWARD_SLASH ) ) != NULL)
				{
					cdTokens[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
					if( strlen( cdTokens[token_count] ) == 0 )
					{
						cdTokens[token_count] = NULL;
					}
					token_count++;
				}
				for (i=0;i<token_count;i++)
				{		 
					char subDirectoryName[12];
					for(index=0;index<strlen(cdTokens[i]);index++)
					{
						if(cdTokens[i][index]!='\0')
						subDirectoryName[index] = cdTokens[i][index];
					}
					for(index=strlen(cdTokens[i]);index<11;index++)
					{
						subDirectoryName[index] = 32;
					}
					subDirectoryName[11]='\0';
					for(index =0;index < 16;index++)
					{
						char name[12];
						memcpy(name,dir[index].DIR_Name,11);
						name[11] = '\0';
						if(!strcmp(subDirectoryName,name) && dir[index].DIR_Attr == 16)
						{
							fileFoundCheck = 1;
							if(dir[index].DIR_FirstClusterLow == 0)
							{
								toSetRootClusterAddressAndPrintInfo(token[0]);
							}
							else
							{
								rootClusterAddress = LBAToOffset(dir[index].DIR_FirstClusterLow);
							}
							toReadAndDisplayTheContensofBlock(token[0]);
						}
					}
					if(!fileFoundCheck)
					{
						printf("Error: No Such directory.\n");
					}
				}	
			}
		}
		else if(!strcmp(token[0],"stat"))
		{
			if(fileOpenCheck < 0)
			{
				printf("Error: File system image must be opened first.\n");
			}
			else
			{
				int fileFoundCheck = 0;
				char *fileName = generateFAT_fileNameConvention(token[1]);
				for(index =0;index < 16;index++)
				{
					char name[12];
					memcpy(name,dir[index].DIR_Name,11);
					name[11] = '\0';
					if(!strcmp(fileName,name))
					{
						fileFoundCheck = 1;
						printf("%s%14s%30s\n","Attribute","Size","Starting Cluster Number");
					    printf("%d%20d%14d\n",dir[index].DIR_Attr,dir[index].DIR_FileSize,dir[index].DIR_FirstClusterLow);
					}
				}
				if(!fileFoundCheck)
				{
					printf("Error: File not found.\n");
				}
			}
		}
		else if(!strcmp(token[0],"get"))
		{
			if(fileOpenCheck < 0)
			{
				printf("Error: File system image must be opened first.\n");
			}
			else
			{
				int fileFoundCheck = 0;
				char *fileName = generateFAT_fileNameConvention(token[1]);
				for(index =0;index < 16;index++)
				{
					char name[12];
					memcpy(name,dir[index].DIR_Name,11);
					name[11] = '\0';
					if(!strcmp(fileName,name))
					{
						fileFoundCheck = 1;
						FILE *newFp;
						newFp = fopen(token[1],"w");
						int fileSize = dir[index].DIR_FileSize;
						int lowClusterNumber = dir[index].DIR_FirstClusterLow;
						int offset = LBAToOffset(lowClusterNumber);
						fseek(fp,offset,SEEK_SET);
						char * ptr  = (char*)malloc(512);
						while(fileSize >= 512)
						{
							fread( ptr, 1, 512, fp );
							fwrite(ptr,512,1,newFp);
							fileSize = fileSize - 512;
							
							lowClusterNumber = NextLB(lowClusterNumber);
							if(lowClusterNumber == -1)
								break;
							offset = LBAToOffset(lowClusterNumber);
							fseek(fp,offset,SEEK_SET);
						}
						if(fileSize > 0) {
							fread( ptr, 1, fileSize, fp );
							fwrite(ptr,fileSize,1,newFp);
						}
						fclose(newFp);
					}
				}
				if(!fileFoundCheck)
				{
					printf("Error: File not found.\n");
				}
			}
		}
		else if(!strcmp(token[0],"read"))
		{
			if(fileOpenCheck < 0)
			{
				printf("Error: File system image must be opened first.\n");
			}
			else
			{
				int fileFoundCheck = 0;
				char *fileName = generateFAT_fileNameConvention(token[1]);
				for(index =0;index < 16;index++)
				{
					char name[12];
					memcpy(name,dir[index].DIR_Name,11);
					name[11] = '\0';
					if(!strcmp(fileName,name))
					{
						fileFoundCheck = 1;
						char readBytes;
						int position = atoi(token[2]);
						int numberOfBytes = atoi(token[3]);
						int lowClusterNumber = dir[index].DIR_FirstClusterLow;
						if(position >= 512)
						{
							lowClusterNumber = NextLB(lowClusterNumber);
							position = position - 512;
						}
						int offset = LBAToOffset(lowClusterNumber);
						printf("%x\n",offset);
						fseek(fp,offset,SEEK_SET);
						fseek(fp,position,SEEK_CUR);
						while(numberOfBytes > 0)
						{
							fread(&readBytes,1,1,fp);
							printf("%x ",readBytes);
							numberOfBytes = numberOfBytes - 1;
						}
						printf("\n");
					}
				}
				if(!fileFoundCheck)
				{
					printf("Error: File not found.\n");
				}
			}
		}
		else if(!strcmp(token[0],"volume"))
		{
			if(fileOpenCheck < 0)
			{
				printf("Error: File system image must be opened first.\n");
			}
			else
			{
				fseek(fp,71,SEEK_SET);
				fread(&BS_VolLab,11,1,fp);
				if(strlen(BS_VolLab) != 0)
				printf("Volume name of the file is'%s'.\n",BS_VolLab);
			    else
				printf("Error:Volume name not found.\n");
			}
		}
		else
		{
			printf("Command not found.\n");
		}
    }

    free( working_root );

  }
	
	return 0;	
}

int LBAToOffset(int32_t sector)
{
	return ((sector - 2) * BPB_BytesPerSec) + (BPB_BytesPerSec*BPB_RsvdSecCnt) + (BPB_NumFATS*FATSz32*BPB_BytesPerSec);	
}

int16_t NextLB( uint32_t sector)
{
	uint32_t FATAddress = (BPB_BytesPerSec*BPB_RsvdSecCnt) + (sector*4);
	int16_t val;
	fseek(fp,FATAddress, SEEK_SET);
    fread(&val,2,1,fp);
	return val;
}
void toSetRootClusterAddressAndPrintInfo(char *cmd)
{
	
	fseek(fp,11,SEEK_SET);	
	fread(&BPB_BytesPerSec,1,2,fp);

	fseek(fp,13,SEEK_SET);	
	fread(&BPB_SecPerClus,1,1,fp);

	fseek(fp,14,SEEK_SET);	
	fread(&BPB_RsvdSecCnt,1,2,fp);

	fseek(fp,0,SEEK_CUR);	
	fread(&BPB_NumFATS,1,1,fp);

	fseek(fp,36,SEEK_SET);	
	fread(&FATSz32,1,4,fp);

	if (!strcmp(cmd,"info"))
	{
		printf("\nBPB_BytesPerSec: %d %x\n",BPB_BytesPerSec,BPB_BytesPerSec);
		printf("\nBPB_SecPerClus: %d %x\n",BPB_SecPerClus,BPB_SecPerClus);
		printf("\nBPB_RsvdSecCnt: %d %x\n",BPB_RsvdSecCnt,BPB_RsvdSecCnt);
		printf("\nBPB_NumFATS: %d %x\n",BPB_NumFATS,BPB_NumFATS);
		printf("\nFATSz32: %d %x\n",FATSz32,FATSz32);
	}
		
	rootClusterAddress = (BPB_RsvdSecCnt*BPB_BytesPerSec) + (BPB_NumFATS*FATSz32*BPB_BytesPerSec);	
}


void toReadAndDisplayTheContensofBlock (char *cmd)
{
	int index;
	fseek(fp,rootClusterAddress,SEEK_SET);
				
	for(index =0;index < 16;index++)
	{
		fread(&dir[index],sizeof(struct DirectoryEntry),1,fp);
	}
	if(!strcmp(cmd,"ls"))
	{
		for(index =0;index < 16;index++)
		{
			char name[12];
			memcpy(name,dir[index].DIR_Name,11);
			name[11] = '\0';
			if(name[0]!= (char)0xE5)
			{
				if(dir[index].DIR_Attr == 1 || dir[index].DIR_Attr == 16 || dir[index].DIR_Attr == 32 || dir[index].DIR_Attr == 48)
				printf("%s\n",name);
			}
		}
	}
}


char* generateFAT_fileNameConvention(char *str)
{
	int i = 0,j=0,p;
    int ascii_value;
	char *a = (char*)malloc(sizeof(char)*12);
    
    while(str[i] != '\0')
    {
        ascii_value = (int)str[i];
        if(ascii_value >=97 && ascii_value <=122)
        {
            a[j++] = (char)ascii_value-32;
        }
        else if(ascii_value == 46)
        {
            for(p=j;p<8;p++)
            {
                a[j++] = 32;
            }
        }
        else
        {
            a[j++] = str[i]; 
        }
        i++;
    }
    a[j] = '\0';
	
	if(strlen(a)<11)
	{
		for(i=strlen(a);i<11;i++)
		{
			a[i] = 32;
		}
		a[11]='\0';
	}
 
	return a;
}
