/*
  Wilddog.cpp - Library for flashing Wilddog code.
  Created by Sky.Li, October 27, 2015.
  Released into the public domain.
*/

#include "Wilddog.h"
#include "wilddog_type.h"
#include "utility/Wilddog_conn.h"


void Wilddog::trysync()
{
	wd_connect.trysync();
}

Wilddog::Wilddog(const char *p_url)
{
	wilddog_index = wd_connect.connect_init(p_url);
	Serial.print(wilddog_index,DEC);
}
/**
*Closes 
**/
Wilddog::~Wilddog() 
{
	wd_connect.connect_deInit();
	wilddog_index = 0;
}
/*
* 
*

boolean Wilddog::wilddogBusying() 
{ 
	return wd_connect.platform_Busying();
}


int Wilddog::connect_init(const char *p_url)
{
	
  return wd_connect.cmd_send(_CMD_INIT_WILDDOG,p_url); 
}
**/
/*
* 
*/
int Wilddog::getValue(CallBackFunc f_callback,void *arg)
{
  if(wilddog_index)	
  	return wd_connect.connect_send(_CMD_GET,f_callback,arg);  
  else
  	return -1;
}
/*
* 
*/
int Wilddog::setValue(const char *p_data,CallBackFunc f_callback,void *arg)
{
	if(wilddog_index)
		return wd_connect.connect_send(_CMD_SET,p_data,f_callback,arg);  
	else
		return -1;
}
/*
* 
*/
int Wilddog::push(const char *p_data,CallBackFunc f_callback,void *arg)
{
	if(wilddog_index)
  		return wd_connect.connect_send(_CMD_PUSH,p_data,f_callback,arg);  
	else
		return 0;
}
/*
* 
*/
int Wilddog::removeValue(CallBackFunc f_callback,void *arg)
{
	
	if(wilddog_index)
  		return wd_connect.connect_send(_CMD_REMOVE,f_callback,arg); 
	else 
		return -1;
}
/*
* 
*/
int Wilddog::addObserver(Wilddog_EventType_T event,CallBackFunc f_callback,void *arg)
{
	if(wilddog_index)
  		return wd_connect.connect_send(_CMD_ON,event,f_callback,arg);  
	else
		return -1;
}
/*
* 
*/
int Wilddog::removeObserver(Wilddog_EventType_T event)
{
	if(wilddog_index)
  		return wd_connect.connect_send(_CMD_OFF,event);  
	else
		return -1;
}
/*
* 
*/
int Wilddog::auth(const char *p_token,const char *p_host,CallBackFunc f_callback,void *arg)
{
	if(wilddog_index)
  		return wd_connect.connect_send(_CMD_AUTH,p_token,p_host,f_callback,arg);  
	else
		return -1;
}
