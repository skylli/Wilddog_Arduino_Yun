/*
  Wilddog_manage.h - Library for flashing Platform_ArduinoYun code.
  Created by Sky.Li, October 27, 2015.
  Released into the public domain.
*/

#ifndef _WILDDOG_TYPE_H_
#define _WILDDOG_TYPE_H_


typedef enum DAEMON_CMD{
    
    _CMD_INIT_WILDDOG,//
    _CMD_GET,
    _CMD_SET,
    _CMD_PUSH,
    _CMD_REMOVE,
    _CMD_ON,
    _CMD_OFF,//
    _CMD_AUTH,
	
    _CMD_DESTORY_WILDDOG,//
    _CMD_INIT,//
    _CMD_DESTORY,//

    _CMD_NOTIFY,
    _CMD_MAX
    
}Daemon_cmd_T;

typedef enum WILDDOG_EVENTTYPE_T
{
    WD_ET_NULL        = 0x00,
    WD_ET_VALUECHANGE = 0x01,
    WD_ET_CHILDADD    = 0x02,
    WD_ET_CHILDCHANGE = 0x04,
    WD_ET_CHILDREMOVE = 0x08,
    WD_ET_CHILDMOVED  = 0x10,
}Wilddog_EventType_T;

#endif /* _WILDDOG_TYPE_H_ */
