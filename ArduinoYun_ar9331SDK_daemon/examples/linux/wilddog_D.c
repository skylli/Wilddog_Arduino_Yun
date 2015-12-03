
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../../src/wilddog_endian.h"
#include "wilddog.h"
#include "utlist.h"

#include "wilddog_D.h"
#define _DAEMON_SERVER_PORT (9527)
#define _DAEMON_CLIENT_IP 	"127.0.0.1"
#define _DAEMON_NODE_LEN    (127)
#define _RECV_BUFFERLEN	(1300)
#define _ACK_NOINDEX    "class init fault."
#define _ACK_JSON_NOCMD    "cmd not find."



typedef struct _DAEMON_NODE_T{
	struct _DAEMON_NODE_T *next;

    Wilddog_Address_T d_srcAddr;
    
	int index;
	Daemon_cmd_T cmd;
    Wilddog_EventType_T event_type;
}_Daemon_Node_T;

typedef enum _DAEMON_JSONTYPE_T{
    _JSON_CMD,
    _JSON_INDEX,
    _JSON_DATA,
    _JSON_HOST,
    _JSON_KEY,
    _JSON_VALUE,
    _JSON_EVENTTYPE
}_Daemon_JsonType_T;

static _Daemon_Node_T *l_p_hd = NULL;
static int l_destory_flag = 0;
static int l_fd = 0;
static int l_trysyncRunning = 0;

extern Wilddog_Str_T  * WD_SYSTEM wilddog_debug_n2jsonString
        (
    Wilddog_Node_T* p_head
    );

int Daemon_server_recv(_Daemon_Node_T **pp_new,char *p_buf,int *p_bufLen);
static int Daemon_json_parse(const char *src,const char *name,char *dst,int dstLen);
_Daemon_Node_T *Daemon_node_creat(struct sockaddr_in *p_remaddr);
static int Daemon_cmd_intwithurl(_Daemon_Node_T *p_new,const char *src);
static int Daemon_cmd_getValue(_Daemon_Node_T *p_node,const char *src);
static int Daemon_handleReceive(_Daemon_Node_T *p_new,const char *buf);



char* strstr(const char *s1,const char *s2)
{
	const char *p=s1;
	const size_t len=strlen(s2);
	for(;(p=strchr(p,*s2))!=0;p++)
	{
		if(strncmp(p,s2,len)==0)
		return	(char*)p;
	}
    return(NULL);
}

static const char *skip_space(const char *str)
{
	while(*str == ' ' || *str == '\t')
	{
		str++;
	}
	return str;
}
/*
* Function:    sjson_get_value
* Description: get json value
* Input:      input :json ; name: item's key ; maxlen : input buf sizeof
* Output:     output : value  ;
*                 *maxlen :  value's len
* Return:     value's len
*/

int sjson_get_value(const char *input, const char *name,
                        char *output, int *maxlen)
{
    const char *p_name = strstr(input, name);
    const char *p;
   // const char tmp = ':';
    char start = 0,end = 0,subItem = 0;
    int i;
    int len;
    
    if(p_name == NULL)
        goto err1;
    

    end = '\"';
    p = skip_space(p_name);
    
    if( strncmp(p,":\"",sizeof(":\"")) == 0)
    {
        p += 2;
    }
    else
    {
        
        while( *p!=':' && p!= NULL)
            p++;
        if(p == NULL)
            goto err1;
        
        p += 1;
        p = skip_space(p);
        if(*p == '{')
        {
            start = '{';
            end = '}';
        }
        else
        if(*p == '\"')
        {
            p += 1;
            start = '\"';
            end = '\"';
        }
        else
        if(*p == '[')
        {
            start = '\[';
            end = ']';
        }
        else
        {
            start = 0;
            end = ',';
        }
        subItem = 0;
    }

    memset(output, 0, *maxlen);
    len = 0;
    for(i = 0; i < *maxlen; i++)
    {
        while(*p == '\\')
        {
            p++;
        }
        subItem = (*p == start)?(subItem+1):subItem;
        if(*p == end)
        {
            if(end == '}')
            {
                if(subItem == 0)
                    break;
                else
                    subItem--;
                output[i] = *p;
            }
            else
            {
                break;
            }
        }
        
        output[i] = *p;
        p++;
        len++;
    }
    *maxlen = len;
    
    return len;
err1:
    return -1;
}


int Daemon_server_creat(void)
{
    struct sockaddr_in si_me;
    int serverPort = 0,res = 0,len =0;
    char fileName[256];
    char responBuff[256];
    FILE  *fp = NULL;

    memset(responBuff,0,sizeof(responBuff));
    memset( fileName,0,sizeof(fileName));
    
    if ((l_fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    {
        //wilddog_debug_level(WD_DEBUG_ERROR,"creat udp server falt");
        return l_fd;
    }

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
   // si_me.sin_port = htons(_DAEMON_SERVER_PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if ( (res = bind(l_fd, &si_me, sizeof(si_me)))==-1)
    {
        sprintf(responBuff,"{\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\"}",\
            _JSON_CMD_,_CMD_INIT,_JSON_SERVERPORT_,0,_JSON_ERRORCODE_,-1);    
    }
    else
    {
        len = sizeof(struct sockaddr_in);
        memset(&si_me,0,len);
        getsockname(l_fd,&si_me,&len);
        serverPort = ntohs(si_me.sin_port);
        sprintf(responBuff,"{\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\"}",\
            _JSON_CMD_,_CMD_INIT,_JSON_SERVERPORT_,serverPort,_JSON_ERRORCODE_,0);
    }

    //wilddog_debug("get server port : %d",serverPort);
    /* creat file to save port number.*/
    sprintf(fileName,"%s",_FILE_PATH);
    /* */
    if( access(fileName,R_OK) == -1)
    {
        umask(0);
        if( mkdir(fileName,0777) == -1)
            return -1;
    }

    
    memset( fileName,0,sizeof(fileName));
    sprintf(fileName,"%s%s",_FILE_PATH,_FILE_DAEMON_NAME_);
	fp = fopen(fileName, "w");
	if( fp == NULL)
		return -1;
    
    while( flock(fileno(fp), LOCK_EX) != 0 )
	{
		sleep(1);
		//printf("write wdd file was locked ...\n");
	}

    
	fputs(responBuff,fp);
    fputs(" \n",fp);
	fflush(fp);
	//printf("write wdd locking file \n");
	flock(fileno(fp), LOCK_UN);
	//printf("write wdd wiret done \n");

	if(fp)
		fclose(fp);
	
    wilddog_debug_level( WD_DEBUG_LOG,"open soket %d",l_fd);
    return res;  
}

void Daemon_server_close()
{
    if(l_fd)
		close(l_fd);	
}
static int Daemon_server_send(Wilddog_Address_T* addr_in,\
        void* tosend,s32 tosendLength)
{
    int ret;
    
    struct sockaddr_in servaddr;    /* server address */
    /* fill in the server's address and data */
    memset((char*)&servaddr, 0, sizeof(servaddr));

     wilddog_debug_level(WD_DEBUG_LOG, "addr_in->port = %d, ip = %u.%u.%u.%u\n", addr_in->port, addr_in->ip[0], \
        addr_in->ip[1], addr_in->ip[2], addr_in->ip[3]);
     
    if(addr_in->len == 4 ){
        servaddr.sin_family = AF_INET;
    }
    else{
        wilddog_debug_level(WD_DEBUG_ERROR, "wilddog_send-unkown addr len!");
        return -1;
    }
    servaddr.sin_port = wilddog_htons(addr_in->port);
    memcpy(&servaddr.sin_addr.s_addr,addr_in->ip,addr_in->len);


    wilddog_debug_level(WD_DEBUG_LOG, "addr_in->port = %d, ip = %u.%u.%u.%u\n", addr_in->port, addr_in->ip[0], \
        addr_in->ip[1], addr_in->ip[2], addr_in->ip[3]);
    
    if((ret = sendto(l_fd, tosend, tosendLength, 0, (struct sockaddr *)&servaddr,
             sizeof(servaddr)))<0){
        perror("sendto failed");
        return -1;
    }
    return ret;
}

int Daemon_server_recv(_Daemon_Node_T **pp_new,char *p_buf,int *p_bufLen)
{
	struct sockaddr_in   si_other;
	int s, i, slen=sizeof(si_other);
	char recv_buf[_RECV_BUFFERLEN];
	int res = 0;

	memset((char *) recv_buf, 0, sizeof(recv_buf));	
	memset((char *) p_buf, 0, *p_bufLen );
	


	if ( (res = recvfrom(l_fd, recv_buf, _RECV_BUFFERLEN, MSG_DONTWAIT, &si_other, &slen))==-1)
	{
		//perror("receive error");
		return res;
	}
#if 0   
	printf("Received packet from %s:%d\nData: %s\n\n", 
		inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), recv_buf);	
	printf("Received dataLen : %d \n ",strlen(recv_buf));
#endif
	if(*p_bufLen >= res )
    {
		memcpy(p_buf,recv_buf,res);
        *p_bufLen = res;
    }
    else
        *p_bufLen = 0;

    
    if(( *pp_new = Daemon_node_creat(&si_other))==NULL)
    {
        return -1;
    }
   return res;
}

#if 1
static int file_write(const char *fileName,const char *str)
{
	char newline = '\n';
	FILE  *fp = NULL;
	if(fileName == NULL)
		return -1;
	
	fp = fopen(fileName, "a+");
	if( fp == NULL)
		return -1;
	
	while( flock(fileno(fp), LOCK_EX) != 0 )
	{
		sleep(3);
		//printf("write :: file was locked ...\n");
	}
	
	fputs(str,fp);
    fputs(" \n",fp);
	fflush(fp);
	//printf("write :: locking file \n");
	flock(fileno(fp), LOCK_UN);
	//printf("write :: wiret done \n");

	if(fp)
		fclose(fp);
	
	return  0;
	
} 
static void Daemon_file_clear(void)
{
    char p_filename[_RECV_BUFFERLEN];
    
    memset(p_filename,0,_RECV_BUFFERLEN);
    sprintf(p_filename,"rm %s%s*",_FILE_PATH,_FILE_NAME_);
    system(p_filename);
}
static int Daemon_server_ack2File(_Daemon_Node_T *p_node,int error,int index,char *data)
{
    int fd = 0;
    char buf[_RECV_BUFFERLEN],p_filename[_RECV_BUFFERLEN];
    size_t length = 0;

    memset(buf,0,_RECV_BUFFERLEN);
    memset(p_filename,0,_RECV_BUFFERLEN);
    if(data)
        sprintf(buf,"{\".cmd\":\"%d\",\".error\":\"%d\",\".index\":\"%d\",\".data\":%s}",p_node->cmd,error,index,data);
    else
        sprintf(buf,"{\".cmd\":\"%d\",\".error\":\"%d\",\".index\":\"%d\"}",p_node->cmd,error,index);

    //wilddog_debug("ack data %s ",buf);

    /* write to an file.*/
     /* The file to which to append the timestamp.  */ 
  sprintf(p_filename,"%s%s%d",_FILE_PATH,_FILE_NAME_,index);
    
  return file_write(p_filename,buf);
#if 0    
  /* Open the file for writing. If it exists, append to it; 
     otherwise, create a new file.  */ 
  fd = open (p_filename, O_WRONLY | O_CREAT|O_TRUNC, 0666); 
  
  /* Compute the length of the timestamp string.  */ 
  length = strlen (buf); 
  /* Write the timestamp to the file.  */ 
  write(fd, buf, length); 
  /* updata file in destop*/
  fsync(fd);
  /* All done.  */ 
  close (fd); 
  return 0;
 #endif 
}

#endif


static int Daemon_server_ack(_Daemon_Node_T *p_node,int error,int index,char *data)
{
    char buf[_RECV_BUFFERLEN];

    memset(buf,0,_RECV_BUFFERLEN);
    if(data)
        sprintf(buf,"{\".cmd\":\"%d\",\".error\":\"%d\",\".index\":\"%d\",\".data\":%s}",p_node->cmd,error,index,data);
    else
        sprintf(buf,"{\".cmd\":\"%d\",\".error\":\"%d\",\".index\":\"%d\"}",p_node->cmd,error,index);

    printf("ack data %s\n",buf);
    
   fflush(stdout);
   Daemon_server_send(&p_node->d_srcAddr,buf,strlen(buf));

   return Daemon_server_ack2File(p_node,error,index,data);
}

static int Daemon_json_parse(const char *src,const char *name,char *dst,int dstLen)
{
	return sjson_get_value(src,name,dst,&dstLen);
}
static int Daemon_json_getIndex(const char *src)
{
    char tmp[128];

    memset(tmp, 0, sizeof(tmp));
    if(Daemon_json_parse(src,_JSON_INDEX_,tmp,sizeof(tmp)) < 0 )
        return 0;
    
    return atoi(tmp);
}
_Daemon_Node_T *Daemon_node_creat(struct sockaddr_in *p_remaddr)
{
	_Daemon_Node_T *p_new = NULL;
	
	p_new = (_Daemon_Node_T*)wmalloc(sizeof(_Daemon_Node_T));
	if(p_new == NULL)
		return NULL;
    
    memcpy(p_new->d_srcAddr.ip, &p_remaddr->sin_addr.s_addr, 4);
    p_new->d_srcAddr.port =  wilddog_ntohs(p_remaddr->sin_port);
    p_new->d_srcAddr.len = 4;
    
	return p_new;
}
static int Daemon_node_register(_Daemon_Node_T* p_node,Daemon_cmd_T cmd,int index)
{
    if(p_node == NULL)
        return -1;
    
    p_node->cmd = cmd;
	p_node->index = index;
    
	LL_APPEND(l_p_hd,p_node);
    return 0;
}
static BOOL *Daemon_node_exist(int getIndex)
{
    _Daemon_Node_T *curr,*tmp;
    LL_FOREACH_SAFE(l_p_hd,curr,tmp)
    {
        if(curr->index == getIndex)
            return TRUE;
        
    }

    return FALSE;
}
void Daemon_node_destory(_Daemon_Node_T **pp_delete)
{
    if( *pp_delete == NULL)
        return ;

        
    /* remove from the list*/
    if(l_p_hd)
        LL_DELETE(l_p_hd,*pp_delete);  

   
    wfree(*pp_delete);
    
    *pp_delete = NULL;
}
#if 0
static void Daemon_node_deleteFile(int index)
{
    char fileName[_RECV_BUFFERLEN];
    memset(fileName,0,_RECV_BUFFERLEN);
    sprintf(fileName,"%s%d",_FILE_NAME_,index);
    wilddog_debug("remove file %s ",fileName);
    remove(fileName);
}
#endif
STATIC char *Daemon_cb_parsePayload(const Wilddog_Node_T* p_snapshot)
{
    char *p_recv_json = NULL,*malloc_buf = NULL;

    if(p_snapshot)
     {
        
        p_recv_json = wilddog_debug_n2jsonString(p_snapshot); 
        if( p_snapshot->d_wn_type == WILDDOG_NODE_TYPE_UTF8STRING )
        {
            malloc_buf = (char*)malloc(strlen(p_recv_json)+20);
            if( malloc_buf == NULL)
            {
                return NULL;
            }

            memset( malloc_buf,0,(strlen(p_recv_json)+20));

            sprintf(malloc_buf,"\"%s\"",p_recv_json);
            wfree(p_recv_json);
            
            return malloc_buf;
        }
        else
            return p_recv_json;
        
    }
    
    return NULL;
}
/* call back functions */
STATIC void Daemon_cb_getValue
    (
    const Wilddog_Node_T* p_snapshot, 
    void* arg, 
    Wilddog_Return_T err
    )
{
    _Daemon_Node_T *p_node = NULL;
    char *p_payload_str = NULL;
    if(arg == NULL )
        goto _GET_ACK;

    p_node = (_Daemon_Node_T*)arg;

    if(err < WILDDOG_HTTP_OK || err >= WILDDOG_HTTP_NOT_MODIFIED)
    {
        wilddog_debug_level(WD_DEBUG_LOG,"getValue fail!");
        goto _GET_ACK;
    }

    p_payload_str = Daemon_cb_parsePayload(p_snapshot);
    // printf("\ngetValue success %p \n",p_payload_str);
    // if(p_payload_str)
    //    printf("\ngetValue success %s \n",p_payload_str);
    
_GET_ACK:    
    Daemon_server_ack(p_node,err,p_node->index,p_payload_str);
#if 0    
    if(p_node->cmd == _CMD_ON)
        Daemon_server_ack2File(p_node,err,p_node->index,p_recv_json);
#endif
    wilddog_debug_level(WD_DEBUG_LOG,"recv node %p index :%d",p_node,p_node->index);
    wfree(p_payload_str);
    
    if(p_node->cmd != _CMD_ON) 
        Daemon_node_destory(&p_node);
    
    return ;
 }


STATIC void Daemon_cb_removeValue(void* arg, Wilddog_Return_T err)
{
    _Daemon_Node_T *p_node = NULL;
    if(arg == NULL )
        goto _REMOVE_ACK;

    p_node = (_Daemon_Node_T*)arg;

    if(err < WILDDOG_HTTP_OK || err >= WILDDOG_HTTP_NOT_MODIFIED)
    {
        //wilddog_debug_level(WD_DEBUG_LOG,"setValue fail!");
        goto _REMOVE_ACK;
    }
    
_REMOVE_ACK:    
    Daemon_server_ack(p_node,err,p_node->index,NULL);
    Daemon_node_destory(&p_node);

    return ;
}
STATIC void Daemon_cb_setValue(void* arg, Wilddog_Return_T err)
{
    _Daemon_Node_T *p_node = NULL;
    
    if(arg == NULL )
        goto _SET_ACK;

    p_node = (_Daemon_Node_T*)arg;
    if(err < WILDDOG_HTTP_OK || err >= WILDDOG_HTTP_NOT_MODIFIED)
    {
        //wilddog_debug_level(WD_DEBUG_LOG,"getValue fail!");
        goto _SET_ACK;
    }

_SET_ACK:    
   Daemon_server_ack(p_node,err,p_node->index,NULL);
   Daemon_node_destory(&p_node);
   
   return;
}
STATIC void Daemon_cb_push(u8 *p_path,void* arg, Wilddog_Return_T err)
{
    _Daemon_Node_T *p_node = NULL;
    char *p_payloadStr = NULL;
    
    if(arg == NULL )
        goto _PUSH_ACK;

    p_node = (_Daemon_Node_T*)arg;
    if(err < WILDDOG_HTTP_OK || err >= WILDDOG_HTTP_NOT_MODIFIED)
    {
        p_path = NULL;
        //wilddog_debug_level(WD_DEBUG_LOG,"push fail Error %d ",err);
        goto _PUSH_ACK;
    }
    
   
_PUSH_ACK:    

   if(p_path) 
   {
        p_payloadStr = wmalloc(strlen(p_path)+10);
        if(p_payloadStr)
        {
            memset(p_payloadStr,0,strlen(p_payloadStr) +10);
            sprintf(p_payloadStr,"\"%s\"",p_path);
        }
   }
   Daemon_server_ack(p_node,err,p_node->index,p_payloadStr);
   Daemon_node_destory(&p_node);
   wfree(p_payloadStr);
   
   return;

}


static int Daemon_cmd_intwithurl(_Daemon_Node_T *p_new,const char *src)
{
    int index = 0,res =0;
    char url[_RECV_BUFFERLEN];

    memset(url,0,_RECV_BUFFERLEN);

    if(Daemon_json_parse(src,_JSON_DATA_,url,_RECV_BUFFERLEN) < 0 )
        goto ACK_ERROR;

    //wilddog_debug("url %s ",url);
    
    index = wilddog_initWithUrl(url);
    //wilddog_debug("get index : %p",index);
    if(index == 0)
       goto ACK_ERROR;
    
    Daemon_server_ack(p_new,0,index,NULL);
    
    if(Daemon_node_exist(index) == FALSE)
        Daemon_node_register(p_new,_CMD_INIT_WILDDOG,index);
    
    return 0;
    
ACK_ERROR:    
    Daemon_server_ack(p_new,-1,index,NULL);
    Daemon_node_destory(&p_new);
    return -1;
}

static int Daemon_cmd_getValue(_Daemon_Node_T *p_node,const char *src)
{
    int index = 0,res = -1;

    if((index  = Daemon_json_getIndex(src)) == 0)
        goto _Get_REQUEST_ERROR;
    
    wilddog_debug_level(WD_DEBUG_LOG,"get index %d",index);
    if(index == 0)
       goto _Get_REQUEST_ERROR;

    Daemon_node_register(p_node,_CMD_GET,index);

    
    res = wilddog_getValue(index, (onQueryFunc)Daemon_cb_getValue, \
                           (void*)p_node);
    
    if(res < 0 )
       goto _Get_REQUEST_ERROR;
    
    return res;
    
_Get_REQUEST_ERROR:  
    
    Daemon_server_ack(p_node,res,index,NULL);
    Daemon_node_destory(&p_node);
    return -1;
}

static int Daemon_cmd_addObserver(_Daemon_Node_T *p_node,const char *src)
{
    int index = 0,res = -1;
    int event_type = 0;
    
    if( (index  = Daemon_json_getIndex(src)) == 0 ||
        Daemon_json_parse(src,_JSON_EVENTTYPE_,&event_type,sizeof(event_type)) < 0 )
        goto _ADDOBSERVER_REQUEST_ERROR;
    
    if(index == 0)
        goto _ADDOBSERVER_REQUEST_ERROR;

    event_type = atoi(&event_type);
    Daemon_node_register(p_node,_CMD_ON,index);
    p_node->event_type = event_type;
    res = wilddog_addObserver( index, event_type,\
                               (onQueryFunc)Daemon_cb_getValue, \
                               (void*)p_node );
    if(res < 0 )
       goto  _ADDOBSERVER_REQUEST_ERROR;
    
    return res;
    
_ADDOBSERVER_REQUEST_ERROR:  
    
    Daemon_server_ack(p_node,res,index,NULL);
    Daemon_node_destory(&p_node);
    return -1;

}
static int Daemon_cmd_offObserver
       (
       _Daemon_Node_T *p_node,
       const char *src
       )
{
    int index = 0,res = -1;
    _Daemon_Node_T *curr = NULL, *tmp = NULL;
    Wilddog_EventType_T event_type = 0;

    if( (index  = Daemon_json_getIndex(src)) == 0 ||
        Daemon_json_parse(src,_JSON_EVENTTYPE_,&event_type,sizeof(event_type)) < 0 )
        goto OFFOBSERVER_REQUEST_ERROR;
    
    if(index == 0)
       goto OFFOBSERVER_REQUEST_ERROR;
    
    event_type = atoi(&event_type);
    
    res = wilddog_removeObserver(index, event_type);
    if(res < 0 )
       goto OFFOBSERVER_REQUEST_ERROR; 
    /* remove add observer*/
    LL_FOREACH_SAFE(l_p_hd,curr,tmp)
    {
        if( curr->index == index && 
            curr->cmd == _CMD_ON &&
            curr->event_type == event_type)

        {
            Daemon_node_destory(&curr);
            }
    }
    /* destory itself*/
    Daemon_server_ack(p_node,res,index,NULL);
    Daemon_node_destory(&p_node);
    return res;
    
OFFOBSERVER_REQUEST_ERROR:  
    
    Daemon_server_ack(p_node,res,index,NULL);
    Daemon_node_destory(&p_node);
    return -1;
}

static int Daemon_cmd_removeValue(_Daemon_Node_T *p_node,const char *src)
{
    int index = 0,res = -1;

    if( (index  = Daemon_json_getIndex(src)) == 0)
        goto _REMOVE_REQUEST_ERROR;
    
    if(index == 0)
        goto _REMOVE_REQUEST_ERROR;

    Daemon_node_register(p_node,_CMD_REMOVE,index);
    res = wilddog_removeValue(index, Daemon_cb_removeValue, \
                                      (void*)p_node);
    
    if(res < 0 )
       goto  _REMOVE_REQUEST_ERROR;
    
    return res;
    
_REMOVE_REQUEST_ERROR:  
    
    Daemon_server_ack(p_node,res,index,NULL);
    Daemon_node_destory(&p_node);
    return -1;

}


static int Daemon_cmd_setValue(_Daemon_Node_T *p_node,const char *src)
{
    int index = 0,res = -1;
    unsigned char data[_DAEMON_NODE_LEN];
    Wilddog_Node_T * p_wd_node = NULL,*p_wd_head = NULL;
    
    memset(data,0,_DAEMON_NODE_LEN);


    
    wilddog_debug_level(WD_DEBUG_LOG,"payload data:",src);
    if((index  = Daemon_json_getIndex(src)) == 0 ||
        Daemon_json_parse(src,_JSON_DATA_,data,_DAEMON_NODE_LEN) < 0 )
        goto _SETVALUE_REQUEST;
    if(index == 0)
        goto _SETVALUE_REQUEST;
    
    Daemon_node_register(p_node,_CMD_SET,index);

    /*Create an node */
    p_wd_head = wilddog_jsonStr2node(data);

    //wilddog_debug_printnode(p_wd_head);
    
    res = wilddog_setValue(index,p_wd_head,Daemon_cb_setValue, \
                                   (void*)p_node);
    
    wilddog_node_delete(p_wd_head);

    
    if(res < 0 )
       goto  _SETVALUE_REQUEST;
    
    return res;
    
_SETVALUE_REQUEST:  
    
    Daemon_server_ack(p_node,res,index,NULL);
    Daemon_node_destory(&p_node);
    return -1;

}

static int Daemon_cmd_push(_Daemon_Node_T *p_node,const char *src)
{
    int index = 0,res = -1;
    unsigned char data[_DAEMON_NODE_LEN];
    Wilddog_Node_T * p_wd_node = NULL,*p_wd_head = NULL;
    
    memset(data,0,_DAEMON_NODE_LEN);

    if((index  = Daemon_json_getIndex(src)) == 0 ||
        Daemon_json_parse(src,_JSON_DATA_,data,_DAEMON_NODE_LEN) < 0  )
        goto _SETVALUE_REQUEST;
    
    Daemon_node_register(p_node,_CMD_PUSH,index);

        /*Create an node which type is an object*/
    p_wd_head = wilddog_jsonStr2node(data);

    //wilddog_debug_printnode(p_wd_head);
    
    res = wilddog_push(index,p_wd_head,Daemon_cb_push, \
                                   (void*)p_node);
    wilddog_node_delete(p_wd_head);

    
    if(res < 0 )
       goto  _SETVALUE_REQUEST;

    
    return res;
    
_SETVALUE_REQUEST:  
    
    Daemon_server_ack(p_node,res,index,NULL);
    Daemon_node_destory(&p_node);
    return -1;
}



static int Daemon_cmd_setAuth(_Daemon_Node_T *p_node,const char *src)
{
    int index = 0,res = -1;
    char value[_RECV_BUFFERLEN],host[_RECV_BUFFERLEN];

    memset(value,0,_RECV_BUFFERLEN);
    memset(host,0,_RECV_BUFFERLEN);
    
    if((index  = Daemon_json_getIndex(src)) == 0 ||
        Daemon_json_parse(src,_JSON_DATA_,value,_RECV_BUFFERLEN) < 0 ||
        Daemon_json_parse(src,_JSON_HOST_,host,_RECV_BUFFERLEN) < 0 
        )
        goto _AUTH_REQUEST_ERROR;
    
    if(index == 0)
       goto _AUTH_REQUEST_ERROR;

    Daemon_node_register(p_node,_CMD_AUTH,index);
    res = wilddog_auth((u8*)host,(u8*)value, \
                               strlen((const char *)value),
                               Daemon_cb_setValue,(void*)p_node);
    if(res < 0 )
       goto _AUTH_REQUEST_ERROR;
    
    return res;
    
_AUTH_REQUEST_ERROR:  
    
    Daemon_server_ack(p_node,res,index,NULL);
    Daemon_node_destory(&p_node);
    return -1;
}
static int Daemon_cmd_init(_Daemon_Node_T *p_node,const char *src)
{
    
    Daemon_server_ack(p_node,0,0,NULL);
    Daemon_node_destory(&p_node);

    return 0;
}

static int Daemon_cmd_destory(_Daemon_Node_T *p_node,const char *src)
{
    
    l_destory_flag= 1;
    p_node->cmd = _CMD_DESTORY;
    Daemon_server_ack(p_node,0,0,NULL);
    Daemon_node_destory(&p_node);

    /* clearn the file*/
    Daemon_file_clear();

    return 0;
}
static int Daemon_cmd_destory_wilddog(_Daemon_Node_T *p_node,const char *src)
{
    int wd_index = 0,index = 0 , res = -1;
    _Daemon_Node_T *curr = NULL, *tmp = NULL;

    
    p_node->cmd = _CMD_DESTORY_WILDDOG;
    
    if( (index  = Daemon_json_getIndex(src)) == 0 )
        goto _DESTORY_REQUEST;

    res = 0;
    //Daemon_node_register(p_node,_CMD_DESTORY_WILDDOG,index);
    if(Daemon_node_exist(index) == FALSE)
    {
            /* cann't find such wilddog.*/
            Daemon_server_ack(p_node,-1,index,NULL);
            Daemon_node_destory(&p_node);
            return -1;
    }
    /*wait for trysunc thread*/
    wd_index = index;
    Daemon_server_ack(p_node,res,index,NULL);
    
    Daemon_node_destory(&p_node);
    LL_FOREACH_SAFE(l_p_hd,curr,tmp)
    {
        if(curr->index == index)
        {
            Daemon_node_destory(&curr);
         }
    }
    wilddog_destroy((Wilddog_T*)&wd_index);
    
    
    return res;
    
_DESTORY_REQUEST:
    Daemon_server_ack(p_node,res,index,NULL);
    Daemon_node_destory(&p_node);
    return res;
}

static int Daemon_handleReceive(_Daemon_Node_T *p_new,const char *buf)
{
    int cmd = 0,res = 0;
    
    if( Daemon_json_parse(buf,_JSON_CMD_,&cmd,sizeof(cmd)) < 0 )
     {
        
        Daemon_server_ack(p_new,-1,0,_ACK_JSON_NOCMD);
        Daemon_node_destory(&p_new);
        return -1;
    }

    cmd =  atoi(&cmd);

    /* node must be exit in link list.*/
    if( cmd == _CMD_GET ||
        cmd == _CMD_SET ||
        cmd == _CMD_PUSH ||
        cmd == _CMD_REMOVE ||
        cmd == _CMD_ON ||
        cmd == _CMD_OFF ||
        cmd == _CMD_AUTH ||
        cmd == _CMD_DESTORY_WILDDOG )
        {
          int getIndex = 0;
          
          if(   (getIndex  = Daemon_json_getIndex(buf)) == 0 ||
                Daemon_node_exist( getIndex) == FALSE)
            {
                p_new->cmd = cmd;
                Daemon_server_ack(p_new,-1,getIndex,_ACK_NOINDEX);
                Daemon_node_destory(&p_new);
                return -1;
          }
    }
    /* send request to server.*/
    switch(cmd)
    {
        case _CMD_INIT_WILDDOG:
            
            res = Daemon_cmd_intwithurl(p_new,buf);
            
             break;
        case _CMD_GET:
            res = Daemon_cmd_getValue(p_new,buf);
            break;

        case _CMD_SET:
            res = Daemon_cmd_setValue(p_new,buf);
             break;
        case _CMD_PUSH:
            res = Daemon_cmd_push(p_new,buf);
            break;
        case _CMD_REMOVE:
            res = Daemon_cmd_removeValue(p_new,buf);
             break;
             
        case _CMD_ON:             
        case _CMD_NOTIFY:
            res = Daemon_cmd_addObserver(p_new,buf);
            break;
            
        case _CMD_OFF:
            res = Daemon_cmd_offObserver(p_new,buf);
            break;         
        case _CMD_AUTH:
            res = Daemon_cmd_setAuth(p_new,buf);
            break;   
        case _CMD_DESTORY_WILDDOG:   
            res = Daemon_cmd_destory_wilddog(p_new,buf);
            break;
       case _CMD_INIT: 
            res = Daemon_cmd_init(p_new,buf);
            break;
                
        case _CMD_DESTORY: 
            res = Daemon_cmd_destory(p_new,buf);
            break;
    }
     return res;
}

int Daemon_init(void)
{
    
    return Daemon_server_creat();
}

void Daemon_deInit(void)
{
    _Daemon_Node_T *curr = NULL, *tmp = NULL;
    int wd_index = 0;
    
    LL_FOREACH_SAFE(l_p_hd,curr,tmp)
    {
        wd_index = curr->index;
        Daemon_node_destory(&curr);
        wilddog_destroy((Wilddog_T*)&wd_index);
    }
    
    l_p_hd = NULL;
    Daemon_server_close(l_fd);
}

int main_thread(void)
{
    int res = 0,bufLen  = _RECV_BUFFERLEN;
    char buf[_RECV_BUFFERLEN];
   
    void *exit_arg = NULL;
    
/***
     if(freopen("./log","w",stdout)==NULL)
          fprintf(stderr, "error redirecting stdout to log \n");
    
    printf("redirecting stdout to log file\n");
****/
    //printf("{\".cmd\":\"%d\",\".error\":\"%d\",\".index\":\"%d\"}",0,res,0);

    if(Daemon_init()== -1)
    {
        
        //printf( "{\".cmd\":\"9\",\".error\":\"-1\",\".index\":\"0\"}\n" );
        fflush(stdout);
        return -1;
        
    }
    //printf( "{\".cmd\":\"9\",\".error\":\"0\",\".index\":\"0\"}\n" );
    fflush(stdout);
  // printf("{\".cmd\":\"%d\",\".error\":\"%d\",\".index\":\"%d\"}",0,res,0); 
    while(1)
    {
    
        _Daemon_Node_T *p_new = NULL;
        bufLen = _RECV_BUFFERLEN;
        
        res = Daemon_server_recv(&p_new,buf,&bufLen);
        if(res > 0)
        {
        
            Daemon_handleReceive(p_new,buf);
        }

         /* NULL wilddog request that not need to trysync .*/
         if( l_p_hd )
         {       
            wilddog_trySync(); 
         }
         
        if( l_destory_flag )
            break;
    }
    
    /*destory all */
    Daemon_deInit();
  
    return res;
    
}
#if 0
int Daemon_findProgressByName()
{
    FILE *pstr; char cmd[128],buff[512],*p;
    pid_t pID;
    int pidnum;
    const char name[] = "wilddog_daemon";
    int ret=3;
    memset(cmd,0,sizeof(cmd));

    sprintf(cmd, "ps -ef|grep %s ",name);
    pstr=popen(cmd, "r");
    
    if( pstr==NULL )
    { return 1;}
    memset(buff,0,sizeof(buff));
    fgets(buff,512,pstr);
    p=strtok(buff, " ");
    //这句是否去掉，取决于当前系统中ps后，进程ID号是否是第一个字段 pclose(pstr);
    p=strtok(NULL, " ");
    if(p==NULL)
    { return 1; }
    if(strlen(p)==0)
    { return 1; }
    if((pidnum=atoi(p))==0)
    { return 1; }
    printf("pidnum: %d\n",pidnum);
    pID=(pid_t)pidnum;
    /* we don't kill it, just test if it's exit*/
    ret=kill(pID,0);
    printf("ret= %d \n",ret);
    if(0==ret)
        printf("process: %s exist!\n",name);
    else printf("process: %s not exist!\n",name);

    return 0;
}

#endif
int main(int argc, char** argv)
{

	pid_t pid;
    pid = fork();

	switch(pid)
	{
	    /* */
		case 0:
            main_thread();
            break;
		case -1:
    		printf( "{\".cmd\":\"9\",\".error\":\"-1\",\".index\":\"0\"}\n" );
    		exit(-1);
		default:
		    exit(0);
	}
	return 0;
}

