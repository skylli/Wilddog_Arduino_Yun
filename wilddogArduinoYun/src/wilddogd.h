/*
*
*/
#ifndef _WILDDOG_D_H_
#define _WILDDOG_D_H_

#define _JSON_CMD_          ".cmd"
#define _JSON_INDEX_        ".index"
#define _JSON_DATA_         ".data"
#define _JSON_ERRORCODE_    ".error"
#define _JSON_EVENTTYPE_    ".eventType"
#define _JSON_HOST_         ".host"
#define _JSON_SERVERPORT_    ".port"

#define	_FILE_PATH			"/tmp/wilddog/"

#define _FILE_NAME_         ".wd_"
#define _FILE_DAEMON_NAME_    ".wd_D"


#define	_ERROR_INPUT_FAULT_		" illegality input"
#define	_ERROR_INPUT_CMD_FAULT_		"input witchout cmd "

typedef enum DAEMON_CMD{
    
    _CMD_INIT_WILDDOG,
    _CMD_GET,
    _CMD_SET,
    _CMD_PUSH,
    _CMD_REMOVE,
    _CMD_ON,
    _CMD_OFF,
    _CMD_AUTH,
	
    _CMD_DESTORY_WILDDOG,
    _CMD_INIT,
    _CMD_DESTORY,

    _CMD_NOTIFY,
    _CMD_MAX
    
}Daemon_cmd_T;

#endif /* _WILDDOG_D_H_ */
