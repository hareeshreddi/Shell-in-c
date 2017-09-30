/*
	Execute the code using command gcc code.c and then type command ./a.out
*/
#include <stdio.h>      /* printf() */
#include <unistd.h>     /* fork() execvp() getcwd() chdir()...cd command rmdir...rm command */
#include <stdlib.h>     /* exit() */
#include <string.h>     /* gets() and strcmp() */
#include <sys/wait.h>   /* wait() */
#include <dirent.h>     /* closedir() opendir() readdir()...ls command */ 
#include <sys/types.h>  /* ls command */ 
#include <fcntl.h>      /* open() creat() .... handling file operations */
#include <ctype.h>      /* isdigit() */

#define MAX_LINE 256 /* The maximum length a command can have is assumed to be 256*/

                /* GLOBAL VARIABLES */
char history[1000000][MAX_LINE]; //history array to store history commands(can store maximum 100000 commands)
int count = 0;//count denotes total no.of commands executed until now
int exc=0;//to check the exit command 

int execute(char *args[],char*);//execute function is the largest and most important function

//Function to display the history i.e. recent num commands
void displayHistory(int num)
{   
    int i,k=1;
    int counter=count-num;
    for (i = count-1; i>=counter;i--)
    {
        printf("%d.  ",k);//command index
        if(i!=counter)
            printf("%s\n",history[i]);
        else
            printf("%s",history[i]);
        k++;
    }
    printf("\n");
} 

/* ----------------------------------------------------------------- */
/* FUNCTION  parse:                                                  */
/*    This function takes an input line i.e. line and length of the  */
/* line i.e. length and finally parse it into arguments i.e. stored  */
/* in args[].It first replaces all white spaces with zeros until it  */
/* hits a non-white space character which indicates the beginning of */
/* an argument.It finally saves the address to args[], and then      */
/* skips all non-white spaces which constitute the argument.         */
/* ----------------------------------------------------------------- */ 
void  parse(char *line, char *args[],int length)
{
    int i;
    char* line1=(char*)malloc(sizeof(char)*256);
    for(i=0;i<length;i++)
        line1[i]=line[i];
    line1[length]='\0';
     while (*line1 != '\0') { /* check end of line*/ 
          while (*line1 == ' ' || *line1 == '\t' || *line1 == '\n')
               *line1++ = '\0';//replace white spaces with \0
          *args++ = line1;//save the argument position
          while (*line1 != '\0' && *line1 != ' ' &&  *line1 != '\t' && *line1 != '\n') 
               line1++;//skip the argument until
     }
     *args = '\0';//mark the end of argument list
}

//used for checking whether string has only digits or not
int numbers_only(const char *s)
{
    while (*s) {
        if (isdigit(*s++) == 0) return 0;
    }
    return 1;
}

void kill_child(int sig)
{
    kill(0,SIGKILL);
}

//Fuction to get the command from shell and set the args parameter
int formatCommand(char inputBuffer[], char *args[])
{
    int i;//length denotes no of bytes read(length)
    int length = read(STDIN_FILENO, inputBuffer, MAX_LINE);//MAX_LINE(maximum lines of the command) is saved in inputBuffer
    char* iB=(char*)malloc(sizeof(char)*256);//iB contains contents of inputBuffer of len length-1
    for(i=0;i<length-1;i++)
        iB[i]=inputBuffer[i];
    iB[length-1]='\0';
    if (length == 0 || inputBuffer[0]=='\n')
        return -1; 
    else if (length < 0)
    {
        printf("Command not read\n");
        exit(-1); 
    }
    parse(inputBuffer,args,length-1);//to get args[] from inputBuffer
    return execute(args,iB);
}

/* ----------------------------------------------------------------- */
/* FUNCTION  main :                                                  */
/*    This function initializes inputBuffer and args[] array.        */
/* It starts a child process which executes the commands the user    */
/* enters in the prompt and I assumed parent process waits for child */
/* process to complete.                                              */
/* ----------------------------------------------------------------- */ 
int main(void)
{
    char inputBuffer[MAX_LINE]; /* buffer to hold the input command */
    char *args[MAX_LINE/2 + 1];/* maximum arguments */
    pid_t pid;  
    pid = fork();        
	if (pid < 0)//if pid is less than 0, forking fails
	{
    	printf("Fork failed.\n");
    	exit (1);
	}
	else if (pid == 0)//if pid ==0 i.e. child process
	{  
        while(exc!=1){
        printf("Shell>");
        fflush(stdout);
        formatCommand(inputBuffer,args);}
    }
	else//parent process waits for child process to complete 
        wait(NULL);
}
/* ------------------------------------------------------------- */
/* FUNCTION  execute :                                           */
/*    execute function to check which command to be executed     */
/* which is major function for working of the Commands           */
/* The following commands will be checked in execute function    */
/* 1.history 2.exit 3.issue 4.cd 5.pwd                           */
/* 6.ls 7.rm 8.rmexcept 9.<program_name> redirection command     */
/* --------------------------------------------------------------*/ 
int execute(char *args[],char* iB)//arguments(args[]) and inputBuffer(iB)
{
    if(strcmp(args[0],"history")==0)//history command //1.history
    {   
        if(count<=0)
                printf("\nNo Commands in the history\n");
        if(args[1]==NULL)//history command
        {
                printf("Shell command History with recent %d commands and starting from most recent command::\n",count);
                displayHistory(count);
        }
        else 
        {
            int num=atoi(args[1]);//history n command i.e. num=n to show n recent commands
            if(num>count)
            {
                printf("Showing only recent %d commands in the history and starting from most recent command\n",count);
                displayHistory(count);
            }
            else if(num>0)
            {
                printf("Shell command History with recent %d commands and starting from most recent command::\n",num);
                displayHistory(num);   
            }
            else if(num==0)
                printf("No Commands in the history i.e. recent 0 commands\n");
            else
                printf("Enter valid value to show the recent commands\n");
        }
        strcpy(history[count],iB); //Updating the history array with input buffer
        count++;
    }
    else if(strcmp(args[0],"exit")==0)//exit command //2.exit
        exc=1;
    else if(strcmp(args[0],"issue")==0)//issue command //3.issue
    {
        int num=atoi(args[1]);//string to integer
        if(num>count)
            printf("Please do not enter value more than %d\n",count);
        else
        {
            char* comm=history[count-num];//comm is pointed to history[count-num]
            printf("%s command is issued\n",comm);//comm command is issued
            parse(comm,args,strlen(comm));
            iB=comm;//iB is pointed to the comm(old command)
            execute(args,iB);//
        }
    }
    else if(strcmp(args[0],"cd")==0)//cd command //5.cd
    {
        strcpy(history[count],iB); //Updating the history array with input buffer
        count++;
        if(args[1]==NULL)
            printf("Argument to cd command is expected\n");
        else
        {
            if(chdir(args[1])!=0)//if we can change the directory 
                perror("No such directory exists");
            else
            {
                printf("Success!! Directory changed to: ");
                char* buff=(char*)malloc(sizeof(char)*256);
                size_t size;
                buff=getcwd(NULL,0);//pwd will be stored in buff 
                if(!buff)
                    { perror("get_current_dir_name");}
                else
                    printf("%s\n",buff);
            }
        }
    }
    else if(strcmp(args[0],"pwd")==0)//pwd(present working directory) command //5.pwd
    {
        strcpy(history[count],iB); //Updating the history array with input buffer
        count++;
        if(args[1]==NULL)
        {
            char* buff=(char*)malloc(sizeof(char)*256);
            buff=getcwd(NULL,0);
            printf("Current working directory is: %s\n",buff);
        }
    }
    else if(strcmp(args[0],"ls")==0)//ls command //6.ls
    {
        char* buff=(char*)malloc(sizeof(char)*256);
        char* dirr=(char*)malloc(sizeof(char)*1000);
        buff=getcwd(NULL,0);//get current working directory
        DIR* dir=opendir(buff);//open pwd
        struct dirent* c;
        printf("List of all current directories/files in the pwd %s are::\n",buff);
        if(dir)
        {
            while((c=readdir(dir))!=NULL){
            dirr=c->d_name;
            if(c->d_type != DT_DIR)
                printf("%s\n",dirr);//print the directories one by one 
            else if(c->d_type == DT_DIR && strcmp(dirr,".")!=0&&strcmp(dirr,"..")!=0)
                printf("%s\n",dirr);
            }
            closedir(dir);//close pwd
        }
        strcpy(history[count],iB); //Updating the history array with input buffer
        count++;
    }
    else if(strcmp(args[0],"rm")==0)//rm command //7.rm
    {
        char* buff=(char*)malloc(sizeof(char)*256);
        char* dirr=(char*)malloc(sizeof(char)*1000);
        char* buff1=(char*)malloc(sizeof(char)*256);
        buff=getcwd(NULL,0);//get current working directory
        int len=strlen(buff),i,ii=2;
        for(i=0;i<len;i++)
            buff1[i]=buff[i];
        buff1[len]='\0';        
        DIR* dir=opendir(buff);//open pwd
        struct dirent* c;
        int x=0;
        if(args[1][0]=='-'&&args[1][1]=='r'&&args[1][2]=='\0')//rm -r command
        {
            if(dir)
            {
                while((c=readdir(dir))!=NULL){//for each file in the path
                    dirr=c->d_name;
                    ii=2;
                    while(args[ii]!='\0')   //for each directory given in the argument
                    if(strcmp(dirr,".")!=0&&strcmp(dirr,"..")!=0&&strcmp(dirr,args[ii])==0)//if there is match it will be removed
                    {
                        buff1=strcat(buff1,"/");
                        buff1=strcat(buff1,dirr);
                        remove(buff1);//buff1 is the path of the directory to be removed
                        x++;
                        for(i=0;i<len;i++)
                            buff1[i]=buff[i];
                        buff1[len]='\0';        
                        break;
                    }
                    else
                        ii++;
                }
            }
        }
        else if(args[1][0]=='-'&&args[1][1]=='v'&&args[1][2]=='\0')//rm -v command
        {
                if(dir)//check whether files are removed or not using ls command afterwards
            {
                while((c=readdir(dir))!=NULL){//for each file in the path
                    dirr=c->d_name;
                    ii=2;
                    while(args[ii]!='\0')   //for each directory given in the argument
                    if(strcmp(dirr,".")!=0&&strcmp(dirr,"..")!=0&&strcmp(dirr,args[ii])==0&&c->d_type != DT_DIR)//if there is match
                        //and as it is not a directory only files will be removed 
                    {
                        buff1=strcat(buff1,"/");
                        buff1=strcat(buff1,dirr);
                        remove(buff1);//buff1 is the path of the directory to be removed
                        x++;
                        printf("File %s is removed\n",args[ii]);
                        for(i=0;i<len;i++)
                            buff1[i]=buff[i];
                        buff1[len]='\0';        
                        break;
                    }
                    else
                        ii++;
                }
            }
            if(x!=0)
                printf("In total %d files were removed\n",x);
            else
                printf("No files were removed\n");
        }
        else//rm -f and rm commands falls here
        {
            if(dir)//check whether files are removed or not using ls command afterwards
            {
                while((c=readdir(dir))!=NULL){//for each file in the path
                    dirr=c->d_name;
                    ii=2;
                    while(args[ii]!='\0')   //for each directory given in the argument
                    if(strcmp(dirr,".")!=0&&strcmp(dirr,"..")!=0&&strcmp(dirr,args[ii])==0&&c->d_type != DT_DIR)//if there is match
                        //and as it is not a directory only files will be removed 
                    {
                        buff1=strcat(buff1,"/");
                        buff1=strcat(buff1,dirr);
                        remove(buff1);//buff1 is the path of the directory to be removed
                        x++;
                        for(i=0;i<len;i++)
                            buff1[i]=buff[i];
                        buff1[len]='\0';        
                        break;
                    }
                    else
                        ii++;
                }
            }
        }
        x=0;
        closedir(dir);//close pwd
        strcpy(history[count],iB); //Updating the history array with input buffer
        count++;
    }
    else if(strcmp(args[0],"rmexcept")==0)//remove all files except those mentioned in the command from current directory //8.rmexcept
    {
        char* buff=(char*)malloc(sizeof(char)*256);
        char* dirr=(char*)malloc(sizeof(char)*1000);
        char* buff1=(char*)malloc(sizeof(char)*256);
        buff=getcwd(NULL,0);//get current working directory
        int len=strlen(buff),i,ii=1,x=0;
        for(i=0;i<len;i++)
            buff1[i]=buff[i];
        buff1[len]='\0';        
        DIR* dir=opendir(buff);//open pwd
        struct dirent* c;
        if(dir)//
            {
                while((c=readdir(dir))!=NULL)//for each file in the path
                {           
                    dirr=c->d_name;
                    ii=1;
                    while(args[ii]!='\0')   //for each directory given in the argument
                        if(strcmp(dirr,".")==0||strcmp(dirr,"..")==0||c->d_type == DT_DIR)
                            break;
                        else if(strcmp(dirr,".")!=0&&strcmp(dirr,"..")!=0&&strcmp(dirr,args[ii])==0&&c->d_type != DT_DIR)//if there is match
                            break;//and it is not directory then then break from inner loop and here type DT_DIR denotes it is a directory
                        else
                            {
                                if(args[ii+1]=='\0')//if there is no next file we need to remove current file
                                {
                                    buff1=strcat(buff1,"/");
                                    buff1=strcat(buff1,dirr);
                                    remove(buff1);//buff1 is the path of the directory to be removed
                                    x++;
                                    printf("File %s is removed\n",dirr);
                                    for(i=0;i<len;i++)
                                        buff1[i]=buff[i];
                                    buff1[len]='\0';        
                                }
                                ii++;
                            }   
                }
            }
            if(x!=0)
                printf("In total %d Files were removed\n",x);
            else
                printf("No files were removed\n");
            x=0;
            closedir(dir);//close dir which is opened initially
            strcpy(history[count],iB); //Updating the history array with input buffer
            count++;
    }
    else if(args[0]!='\0'&& args[1]!='\0'&& args[2]!='\0')//program_name command and used SIGALRM and alarm too.. //9.<program_name>
    {   //assuming atleat 3 commands 
        //input format example:: a.out > in.txt < out.txt
        strcpy(history[count],iB); //Updating the history array with input buffer
        count++;
        pid_t pid;  
        pid = fork();
        int timer=-1;
        if (pid == 0)/* for the child process:         */
        {          
            int fd0,fd1,i,in=0,out=0,x,y;
            char input[256],output[256];//input and output files
            // finds where '<' or '>' occurs and make that args[i] = NULL , to ensure that command wont't read that
            char* inpf=(char*)malloc(sizeof(char)*256);
            char *bb=(char*)malloc(sizeof(char)*256);
            char* outf=(char*)malloc(sizeof(char)*256);
            char* buff=(char*)malloc(sizeof(char)*256);
            bb=getcwd(NULL,0);//get current working directory
            outf=strcat(outf,bb);
            inpf=strcat(inpf,bb);
            buff=strcat(buff,bb);
            outf=strcat(outf,"/");
            inpf=strcat(inpf,"/");
            buff=strcat(buff,"/");
            buff=strcat(buff,args[0]);//getting path name into buff
            for(i=0;args[i]!='\0';i++)
            {
                if(strcmp(args[i],">")==0)
                {        
                    x=i;
                    strcpy(input,args[i+1]);
                    inpf=strcat(inpf,args[i+1]);
                    in=2;           
                }               
                if(strcmp(args[i],"<")==0)
                {      
                    y=i;
                    strcpy(output,args[i+1]);
                    outf=strcat(outf,args[i+1]);
                    out=2;
                }  
                if(numbers_only(args[i])==1)
                    timer=atoi(args[i]);
            }
            args[x]=NULL;
            args[y]=NULL;
            //if '>' char was found in string inputted by user
            if(in)
            {   
                // fdo is file-descriptor and O_RDONLY denotes it is read only
                if ((fd0 = open(inpf, O_RDONLY, 0)) < 0) {//if error occured whille opening file
                    perror("Couldn't open input file");
                    exit(0);
                }      
                // dup2() copies content of fdo in input of preceeding file
                dup2(fd0,0); 
            }
            //if '>' char was found in string inputted by user 
            if (out)
            {
                // fd1 is file-descriptor , O_CREAT helps in creating new file if it did not exist before,O_RDWR helps in read/write of opened file 
                // 0777 gives permission to view the file 
                if ((fd1 = open(outf , O_RDWR | O_CREAT , 0777)) < 0) {//if error occured while opening the file
                    perror("Couldn't open the output file");
                    exit(0);
                }        
                dup2(fd1,1); 
            }
            if(timer!=-1)//SIGALRM is used as per the hint given in the question 
            {
                signal(SIGALRM,(void (*)(int))kill_child);//signal and alarm are used 
                alarm(timer);
            }
            execl(buff,args[0],(char*)NULL);
            if(in)
                close(fd0);
            if(out)
                close(fd1);
            perror("execl::");
            exit(127);
        }
        else if((pid) < 0)
        {     
            printf("fork() failed!\n");
            exit(1);
        }
        else /* for the parent: */  //while (!(wait(&status) == pid)) ; // good coding to avoid race_conditions(errors) 
            waitpid(pid,0,0);
    }
    else
    {
        printf("***Enter any of the valid commands Please***\n");
        strcpy(history[count],iB); //Updating the history array with input buffer
        count++;
    }
    return -1;
}
