#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
 
#include "Wilddog_daemon.h"

//#define SRV_IP "10.28.6.124"
#define SRV_IP "127.0.0.1"
#define BUFLEN 512
#define NPACK 10
#define PORT 9527

static int l_socket = 0;
const char Usage[]="Usage: input cmd with json type	\
					\n\texample :  ./wilddog_transfer \"{\"cmd\":\"0\",\"data\":\"coap://yourId.wilddogio.com/yourPath\"}\"";
 void diep(char *s)
 {
   perror(s);
   exit(1);
 }

extern int sjson_get_value(const char *input, const char *name,
                        char *output, int *maxlen);
	

long getFilesize( FILE *fp )
{
    long int save_pos;
    long size_of_file;

    /* Save the current position. */
    save_pos = ftell( fp );

    /* Jump to the end of the file. */
    fseek( fp, 0L, SEEK_END );

    /* Get the end position. */
    size_of_file = ftell( fp );

    /* Jump back to the original position. */
    fseek( fp, save_pos, SEEK_SET );

    return( size_of_file );
}
int file_readLine(FILE* fp,char *dst,int dstLen)   
{   
	long int save_pos;
	
	if(fp == NULL)
		return -1;
	
	save_pos = ftell( fp );
	lseek(fileno(fp), 0, SEEK_SET);
	fgets(dst,dstLen,fp);
	fseek( fp, save_pos, SEEK_SET );
	 
	return strlen(dst);
}
void file_deleteLine(FILE* fp, int deleLine)   
{   
   int   line=0,i = 0;   
   int   MaxLine=0;   
   char   Buf[1024]="";   
   char   tmp[100][1024]={0};  
   
   //lseek(fileno(fp), 0, SEEK_SET);
   
   while (fgets(Buf, 1024,fp))   
   {      
	   if( line != deleLine) 
	   {
		strcpy(tmp[i],Buf);	
		i++;	
		}
		
		line++;	
		memset(Buf,0,1024);
   } 
	MaxLine=line;    

	/* clearn that file.*/
	ftruncate(fileno(fp), 0);
	lseek(fileno(fp), 0, SEEK_SET);
	fflush(fp);
	
	for(line=0 ;line <= MaxLine;line++) 
	{
		fputs(tmp[line],fp); 
	}
	
} 

static void file_readfile(const char *fileName)
{
	int buflen = 0;
	char *buffer = NULL;
	FILE *fp = NULL;	

	if ((fp=fopen(fileName,"r+")) == NULL)   
	{   
	  // printf("read ::  Can't   open   file!/n");   
	   return;
	}  
	/*if file empty.*/
	buflen = getFilesize(fp);
	if(buflen == 0)
	   goto _CLOSE_FILE_;

	buffer = (char*)malloc(buflen+1);
	if(buffer == NULL )
	   goto _CLOSE_FILE_;

   	/*lock that file.*/
	if( flock(fileno(fp), LOCK_EX) != 0)
		goto _CLOSE_FILE_;
	
	memset(buffer,0,buflen);
	file_readLine(fp,buffer,buflen);
	printf("%s \n",buffer);
	file_deleteLine(fp, 0);  
	flock(fileno(fp), LOCK_UN);

	
_CLOSE_FILE_:
    if(buffer)
        free(buffer);
	fclose(fp);
	return ;
}	
static int watch_init(void)
{
	struct sockaddr_in si_other;
	int slen=sizeof(si_other);


	if ((l_socket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	{
		perror("open soket error");
		return -1;
	}


	
	return 0;
}
static int transfer_send(const char *src,int daemon_port)
{

	struct sockaddr_in si_other;
	int res = 0,slen=sizeof(si_other);
	
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(daemon_port);
	
	if (inet_aton(SRV_IP, &si_other.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		return -1;
	}
	if ( (res = sendto( l_socket, src, strlen(src), 0, &si_other, slen))==-1)
	{
		perror("send error");
		return -1;
	}

	return res;
} 
static int watch_receive(char *buf,int bufLen)
{
	
	struct sockaddr_in si_other;
	int res = 0,slen=sizeof(si_other);
	
	if ( ( res = recvfrom(l_socket, buf, bufLen, MSG_DONTWAIT, &si_other, &slen))==-1)
		return -1;
	/**
	else
	{
		if(res > 0)
			printf("Received packet from %s:%d\nData: %s\n\n", 
				 inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
		printf("res = %d",res);
		
	}
	*/
	return res;
}
static int watch_detectProcessByName(const char *p_name)
{
    DIR *dir;
    int res = 1;
    struct dirent *ptr;
    FILE *fp;
    char filepath[50];//大小随意，能装下cmdline文件的路径即可
    char filetext[50];//大小随意，能装下要识别的命令行文本即可
    dir = opendir("/proc"); //打开路径
    if (NULL != dir)
    {
        while ((ptr = readdir(dir)) != NULL) //循环读取路径下的每一个文件/文件夹
        {
            //如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0)) continue;
            if (DT_DIR != ptr->d_type) 
                continue;
           
            sprintf(filepath, "/proc/%s/cmdline", ptr->d_name);//生成要读取的文件的路径
            fp = fopen(filepath, "r");//打开文件
            if (NULL != fp)
            {
                fread(filetext, 1, 50, fp);//读取文件
                filetext[49] = '\0';//给读出的内容加上字符串结束符
                //如果文件内容满足要求则打印路径的名字（即进程的PID）
                if (filetext == strstr(filetext,p_name)) 
                {
                   // printf("PID:  %s\n", ptr->d_name);
                    res = 0;
                }
                fclose(fp);
            }
           
        }
        closedir(dir);//关闭路径
    }
    return res;
}
/* */
static int watch_getDaemonPort(const char *fileName)
{
	int buflen = 0,res = -1;
	char *buffer = NULL;
	FILE *fp = NULL;	

	if ((fp=fopen(fileName,"r")) == NULL)   
	{   
	   //printf("read ::  Can't   open   file!/n");   
	   return;
	}  
	/*if file empty.*/
	buflen = getFilesize(fp);
	if(buflen == 0)
	   goto _GETPORT_END_;

	buffer = (char*)malloc(buflen+1);
	if( buffer == NULL )
	   goto _GETPORT_END_;

   	/*lock that file.*/
	if( flock(fileno(fp), LOCK_EX) != 0)
		goto _GETPORT_END_;
	
	memset(buffer,0,buflen);
	file_readLine(fp,buffer,buflen);
	printf("%s \n",buffer);
    res = 0;
    /* unlock that file.*/ 
	flock(fileno(fp), LOCK_UN);

_GETPORT_END_:

    if(buffer)
        free(buffer);
	fclose(fp);
	return res;
}	
int main(int argc, char **argv )
{
	
	int i,res,index = 0, cmd = 0 ,len = 0,daemon_port = 0;
	
	char recvBuffer[BUFLEN];
	char file_name[BUFLEN];
    char buffer[BUFLEN];
	
	memset(recvBuffer,0,BUFLEN);
	memset(file_name,0,BUFLEN);
    memset(buffer,0,BUFLEN);
	if( argc < 2 )
	{
		
		printf("{\".cmd\":\"%d\",\".error\":\"%d\",\".index\":\"%d\",\".data\":\"%s\"}",0,-1,0,_ERROR_INPUT_FAULT_);
		return -1;
	}
	//printf("get packet %s\n",argv[1]);

	//printf(" arg %s \n",argv[1]);
	len = sizeof(cmd);
	if(sjson_get_value(argv[1],_JSON_CMD_,&cmd,&len) < 0 )
	{
		printf("{\".cmd\":\"%d\",\".error\":\"%d\",\".index\":\"%d\",\".data\":\"%s\"}",0,-1,0,_ERROR_INPUT_CMD_FAULT_);
		return -1;	
	}
    cmd = atoi(&cmd);

	if(watch_init() < 0 )
		return -1;
    /* get daemon server port */
    if( cmd != _CMD_INIT)
    {	
        len = BUFLEN;
        if(sjson_get_value(argv[1],_JSON_SERVERPORT_,buffer,&len) < 0 )
        {
				printf("{\".cmd\":\"%d\",\".error\":\"%d\",\".index\":\"%d\"}",cmd,-1,0);
				return -1;
		}
        daemon_port = atoi(buffer);
    }   
	if(cmd >= _CMD_MAX)
	{
		printf("{\".cmd\":\"%d\",\".error\":\"%d\",\".index\":\"%d\",\".data\":\"%s\"}",cmd,-1,0,"error cmd");
		return -1;
	}

	switch(cmd)
	{
		case _CMD_INIT:
            /*find if that process running */
            if(watch_detectProcessByName(_BIN_DAEMON))
            {
                if(system(_BIN_DAEMON) < 0 )
                    printf("{\".cmd\":\"%d\",\".error\":\"%d\",\".index\":\"%d\"}",cmd,-1,0);

			}
            /* read file*/
            sprintf(file_name,"%s%s",_FILE_PATH,_FILE_DAEMON_NAME_);    
            for(i=0;i<4;i++)
            {
                
                if((res = watch_getDaemonPort(file_name))>=0)
                    break;
                sleep(1);
            }
            if(res < 0 )
                 printf("{\".cmd\":\"%d\",\".error\":\"%d\",\".index\":\"%d\"}",cmd,-1,0);
			return res;	
		case _CMD_ON:
			len = 256;
			if(sjson_get_value(argv[1],_JSON_INDEX_,file_name,&len) < 0 )
			{
				printf("{\".cmd\":\"%d\",\".error\":\"%d\",\".index\":\"%d\"}",cmd,-1,0);
				return -1;
			}
			index = atoi(file_name);
			memset(file_name,0,BUFLEN);
			sprintf(file_name,"%s%s%d",_FILE_PATH,_FILE_NAME_,index);
			//printf("reading file : %s \n",file_name);
			transfer_send(argv[1],daemon_port);
			/* while for content*/
			break;
		case _CMD_NOTIFY:
			len = 256;
			if(sjson_get_value(argv[1],_JSON_INDEX_,file_name,&len) < 0 )
			{
				printf("{\".cmd\":\"%d\",\".error\":\"%d\",\".index\":\"%d\"}",cmd,-1,0);
				return -1;
			}
			index = atoi(file_name);
			memset(file_name,0,BUFLEN);
			sprintf(file_name,"%s%s%d",_FILE_PATH,_FILE_NAME_,index);
			//printf("reading file : %s \n",file_name);
			/****
			if(access(file_name,0))
				transfer_send(argv[1]);
			/**/
			file_readfile((const char*)file_name);
			return 0;
		default:
			res = transfer_send(argv[1],daemon_port);
			break;
	}
	
#if 1	
	
	memset(recvBuffer,0,BUFLEN);
	if(	cmd == _CMD_INIT_WILDDOG ||
		cmd == _CMD_DESTORY_WILDDOG ||
		cmd == _CMD_DESTORY )
	{
		while(1)
		{
			res = watch_receive(recvBuffer,BUFLEN);

			if(res > 0)
			{
				printf(" %s \n",recvBuffer);
				break;
			}
		}
	}

#endif

  if(l_socket)
	close(l_socket);
  
  return 0;

	
}