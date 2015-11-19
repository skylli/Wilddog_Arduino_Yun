/*
  Platform_ArduinoYun.cpp - Library for flashing Platform_ArduinoYun code.
  Created by Sky.Li, October 27, 2015.
  Released into the public domain.
*/
#include "Platform_ArduinoYun.h"
//extern "C"
//{
#include "Wilddog_manage.h"
//};
#include "../Wilddog_config.h"

//extern int sjson_get_value(const char *input, const char *name,
//                        char *output, int *maxlen);
						
int Platform_ArduinoYun::initUrl_num = 0;

void Platform_ArduinoYun::_print(const char *src)
{
	Serial.print(src);
	Serial.flush();
}

void Platform_ArduinoYun::_printArray(const char *src,int len)
{
	Serial.write(src,len);
	Serial.flush();
}
Platform_ArduinoYun::Platform_ArduinoYun()
{
  // Initialize Bridge
}

Platform_ArduinoYun::~Platform_ArduinoYun() {
  _p.close();
}
/*
*/
long int Platform_ArduinoYun::_platform_send(Daemon_cmd_T cmd,Wilddog_EventType_T event,unsigned long index,
					const char *src,const char *p_host)
{
	int res,len = 0,src_len = 0; 
	char *p_buf = NULL;
	if(src)
	{
		src_len = strlen((char*)src);
	}
	if(p_host)
	{
		len += strlen((char*)p_host);
	}
	len += 100 + src_len;
	p_buf = (char*)malloc(len);
	if(p_buf == NULL)
		return -1;

	memset(p_buf,0,len);
	/* get send packet*/
	res = manage_getSendPacket(p_buf,&len,cmd,index,event,src,p_host);	

	/** sending out.*/
	if( res > 0 && strlen(p_buf) > 0 )
	{
		if( _p.running())
			_p.close();

		String sendString = String(p_buf);
		
		_p.runShellCommandAsynchronously(sendString);	
		_p.flush();
#if 1	

		/* DEBUGing....*/
		_print("\n cmd :");
		Serial.print(cmd);
		_print("\n event :");
		Serial.print(event);
		_print("\n index :");
		Serial.print(index);

		if(p_host)
		{
			_print("\n p_host :");
			Serial.print(p_host);
		}
		if(src)
		{
			_print("\n data :");
			Serial.print(src);
		}


		_print("\n src_len  :");
		Serial.print(src_len);
		
		_print("\n malloc len :");
		Serial.print(len);
		_print("\n shell cmd 3 :");
		_printArray(p_buf,strlen(p_buf));
		_print("\n shell cmd len :");
		Serial.print(strlen(p_buf));
		
#endif
	}
	
	free(p_buf);
	return res;
}
/* return cmd.*/
int Platform_ArduinoYun::_platform_receives(Daemon_cmd_T *p_cmd,int *p_error)
{


  char *buffer = NULL;
  int len = 0,receiveLen = 0;
  if( (receiveLen = _p.available()) > 0)
  {
  
  		//_print(" receive data: \n");
		
  		buffer = (char*)malloc(receiveLen+1);
		if(buffer == NULL)
			return 0;
		
		memset(buffer,0,receiveLen);
  		for(len = 0;_p.available() > 0;len++)
	  	{
	  		if(len  < receiveLen)
	  		{
	  			char c;
	  			buffer[len]=_p.read();
				c = buffer[len];
				Serial.print(c);
	  			}
	  	}
		*p_cmd = (Daemon_cmd_T)manage_handleReceive(buffer,&index,p_error);
#if 1		
		_print(" \n _platform_receives cmd :\n");
		Serial.print(*p_cmd,DEC);
		_print(" \n _platform_receives index :\n");
		Serial.print(index,DEC);
		_print(" \n _platform_receives error :\n");
		Serial.print(*p_error,DEC);
		_print("\tend \n");
#endif		
		
		if(buffer)
			free(buffer);

		return receiveLen;
  	}

	return 0;
}
void Platform_ArduinoYun::_nonBlockingReive()
{
	int block_time = 0,error = 0;
	Daemon_cmd_T cmd = _CMD_MAX;

	while(1) 
	{
		//_print("+");

		if( _platform_receives(&cmd,&error) > 0 )
			break;
		
		delay(1);
		if( block_time++ > WILDDOG_RECEIVE_TIMEOUT)
			break;
	}
}

void Platform_ArduinoYun::notifyRequest(unsigned long wd_index)
{
	_platform_send(_CMD_NOTIFY,(Wilddog_EventType_T)0,wd_index,NULL,NULL);
}

void Platform_ArduinoYun::trysync()
{


	_nonBlockingReive();
	notifyRequest(index);

}

/* return index.*/
unsigned long Platform_ArduinoYun::platform_init(const char *url)
{
	int cmd = -1, error = 0;
	long int res = 0;
#if 1	
	if(initUrl_num == 0)
	{
		res = _platform_send(_CMD_INIT,(Wilddog_EventType_T)0,0,NULL,NULL);
		if(res == -1)
			return NULL;
		/* wait and read respond.*/
		while(1)
		{
			_platform_receives((Daemon_cmd_T*)&cmd,&error);
			if(cmd == _CMD_INIT )
			{
				 if(error < 0 )
				 	return NULL;
				 break;
			}
		}
	}
#endif	
	initUrl_num++;

	res = _platform_send(_CMD_INIT_WILDDOG,(Wilddog_EventType_T)0,0,url,NULL);
	if(res == -1)
		return -1;
	
	while(1)
	{
		_platform_receives((Daemon_cmd_T*)&cmd,&error);
		if(cmd == _CMD_INIT_WILDDOG )
		{
			if(error < 0 )
				return NULL;
			break;
		}
	}
	/*get index.*/
	return index;
}
int Platform_ArduinoYun::platform_deinit(void)
{
	int cmd = -1, error = 0;
	long int  res = 0;
	
	res = _platform_send(_CMD_DESTORY_WILDDOG,(Wilddog_EventType_T)0,index,NULL,NULL);
	if(res <0)
		return -1;
	/*blocking and waitting for respond.*/
	while(1)
	{
		_platform_receives((Daemon_cmd_T*)&cmd,&error);
		if(cmd == _CMD_DESTORY_WILDDOG )
		{
			if(error < 0 )
				return NULL;
			break;
		}
	}
	initUrl_num--;
	if(initUrl_num == 0)
	{
		
		res = _platform_send(_CMD_DESTORY,(Wilddog_EventType_T)0,index,NULL,NULL);
		if(res < 0 )
			return -1;
			
		while(1)
		{
			_platform_receives((Daemon_cmd_T*)&cmd,&error);
			if(cmd == _CMD_DESTORY )
			{
				if(error < 0 )
					return NULL;
				break;
			}
		}
		manage_destoryList();
		/*blocking and waitting for respond.*/
		
	}
	return error;
}
/* only off.*/
int Platform_ArduinoYun::platform_send(Daemon_cmd_T cmd,Wilddog_EventType_T event)
{
	long int res = 0;

	res = _platform_send(cmd,event,index,NULL,NULL);
	
	return res;
}
int Platform_ArduinoYun::platform_send(Daemon_cmd_T cmd,CallBackFunc f_callback,void *arg)
{
	long int res = 0;
	res = _platform_send(cmd,(Wilddog_EventType_T)0,index,NULL,NULL);
	if( manage_appendNode(cmd,index,f_callback,arg) == NULL )
		return -1;
	else
		return res;
}
int Platform_ArduinoYun::platform_send(Daemon_cmd_T cmd,Wilddog_EventType_T event,CallBackFunc f_callback,void *arg)
{
	long int res = 0;
	res = _platform_send(cmd,event,index,NULL,NULL);
	
	if(manage_appendNode(cmd,index,f_callback,arg) == NULL )
	{
		
		return -1;
	
	}
	else
		return res;
}
int Platform_ArduinoYun::platform_send(Daemon_cmd_T cmd,const char *data,CallBackFunc f_callback,void *arg)
{
	long int res = 0;
	res = _platform_send(cmd,(Wilddog_EventType_T)0,index,data,NULL);
	
	if(manage_appendNode(cmd,index,f_callback,arg) == NULL )
		return -1;
	else
		return res;
}
int Platform_ArduinoYun::platform_send(Daemon_cmd_T cmd,const char *data,const char *p_host,CallBackFunc f_callback,void *arg)
{
	long int res = 0;
	res = _platform_send(cmd,(Wilddog_EventType_T)0,index,data,p_host);
	
	if(manage_appendNode(cmd,index,f_callback,arg) == NULL )
		return -1;
	else
		return res;
}

/**
*Closes a process started by runAsynchronously().
**/
void Platform_ArduinoYun::wilddogRunAsynchronously(){
	
	 _p.runAsynchronously();
}
/*
* wilddogRunShellCommand() is a blocking function. That is, once you call Process.wilddogRunShellCommand(), nothing else will * happen in your sketch until it has completed. The time depends on the nature of the command you are executing. For a  non-blocking alternative, see runShellCommandAsynchronously().
*/
unsigned int Platform_ArduinoYun::wilddogRunShellCommand(const String &command){
	
	return _p.runShellCommand(command);
}
/*
* Get the number of bytes (characters) available for reading from the Linux connection. This is data that's already arrived and stored in the receive buffer. available() inherits from the Stream utility class.*/
int Platform_ArduinoYun::available(){
	 return _p.available();
}

boolean Platform_ArduinoYun::platform_Busying() 
{ 
	return _p.running();
}
/*
*Reads incoming data from a Linux process. read() inherits from the Stream utility class.
*/
int Platform_ArduinoYun::read(){
	 return _p.read();
}
/*
*Returns the next byte (character) of incoming data from a Linux process without removing it from the internal buffer. Successive calls to peek() will return the same character, as will the next call to read(). peek() inherits from the Stream utility class.
*/
int Platform_ArduinoYun::peek(){
	 return _p.peek();
}

/*
Writes data to a Linux process. This data is sent as a byte or series of bytes. write() inherits from the Stream utility class.
*/
size_t Platform_ArduinoYun::write(uint8_t c){
	 return _p.write(c);
} 
 
/*
Clears the Process buffer of any bytes. Waits for the transmission of outgoing data to complete.
*/
void Platform_ArduinoYun::flush(){
	 return _p.flush();
}
