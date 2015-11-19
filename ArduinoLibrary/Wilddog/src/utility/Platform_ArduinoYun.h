/*
  Platform_ArduinoYun.h - Library for flashing Platform_ArduinoYun code.
  Created by Sky.Li, October 27, 2015.
  Released into the public domain.
*/
/*
  Platform_ArduinoYun.h - Library for flashing Platform_ArduinoYun code.
  Created by Sky.Li, October 27, 2015.
  Released into the public domain.
*/

#ifndef PLATFORM_ARDUINOYUN_H_
#define PLATFORM_ARDUINOYUN_H_

#include "Arduino.h"
#include <Bridge.h>
#include <Process.h>

#include "../Wilddog_type.h"

typedef void (*CallBackFunc)(const char *pdata, int error, void* arg);
	

class Platform_ArduinoYun
{
  public:
    
    Platform_ArduinoYun();
	~Platform_ArduinoYun();
    unsigned long platform_init(const char *url);
    int platform_deinit(void);
    int platform_send(Daemon_cmd_T cmd,CallBackFunc f_callback,void *arg);
    int platform_send(Daemon_cmd_T cmd,Wilddog_EventType_T event);
    int platform_send(Daemon_cmd_T cmd,Wilddog_EventType_T event,CallBackFunc f_callback,void *arg);
    int platform_send(Daemon_cmd_T cmd,const char *data,CallBackFunc f_callback,void *arg);
    int platform_send(Daemon_cmd_T cmd,const char *data,const char *p_host,CallBackFunc f_callback,void *arg);
	void trysync();
  
	
	void wilddogRunAsynchronously();
	unsigned int wilddogRunShellCommand(const String &command);

    boolean platform_Busying();	
    void notifyRequest(unsigned long wd_index);
	
  private:

    
    Process _p; 
    
    int available();
    int read();
    int peek();
    size_t write(uint8_t);
	void flush();

    /* debug*/
    void _print(const char *src);
    void _printArray(const char *src,int len);

    void _nonBlockingReive();
    int _platform_receives(Daemon_cmd_T *p_cmd,int *p_error);
    long int _platform_send(Daemon_cmd_T cmd,Wilddog_EventType_T event,unsigned long index,
					const char *src,const char *p_host);
    
    CallBackFunc f_cb;
	void *p_arg;
    unsigned long index;
	static int initUrl_num;
};



#endif