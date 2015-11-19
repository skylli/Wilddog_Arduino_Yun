/*
 * Copyright (C) 2014-2016 Wilddog Technologies. All Rights Reserved. 
 *
 * FileName: demo.c
 *
 * Description: Wilddog demo file.
 *
 * Usage: demo <operation> <-h|-l url> [--key=<arg1>] [--value=<arg2>]
 *      
 *          operations:
 *                  getValue:       get value of the url.
 *                  setValue:       create a new node in the url, need 
 *                                  [--key=<arg1>] and [--value=<arg2>] 
 *                                  followed, arg1 means the node's key which 
 *                                  will add to url, arg2 is arg1's value.
 *                  push:           push a new node in the url, usage similar
 *                                  to <setValue>, but a little different is:
 *                                  <push> will automatically create a node by
 *                                  the server.
 *                  removeValue:    remove the value(and it's children) of the
 *                                  url.
 *                  addObserver:    subscribe the url, any change will be 
 *                                  pushed to the client.
 *                  setAuth:        send the auth token to the server, and get
 *                                  the permission to operate, if you set the
 *                                  control rules in the cloud. 
 *                                  need [--value=<token>] followed.
 *          -h: help
 *          -l: note that a url followed
 *          url:
 *                  like coap://<your appid>.wilddogio.com/[path], <your appid>
 *                  is the appid of the app you created, and path is the path(
 *                  node path) in the app. if the tree like this, <1> is your 
 *                  appid, <a> and <a/b> are both path.
 *                  
 *                  your data tree in cloud:
 *
 *                  1.wilddogio.com
 *                  |
 *                  + a
 *                    |
 *                    + b: 1234
 *
 *      example: if you input :
 *                  demo setValue -l coap://1.wilddogio.com/a --key=b --value=1
 *               you can find the app <1> has a node which key is b,value is 1.
 *               and then if you input :
 *                  demo addObserver -l coap://1.wilddogio.com/a/b
 *               when you change node <b>'s value to 2, the client will receive
 *               the change, and print in the console( and then quit because of
 *               our setting, we set the var <cntmax> in main() to 10, so it 
 *               will break after 10 times, you can remove this setting.)
 *
 * History:
 * Version      Author          Date        Description
 *
 * 0.4.3        Baikal.Hu       2015-07-16  Create file.
 * 0.5.1        Jimmy.Pan       2015-09-22  Add notes.
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h> 
#include "wilddog.h"
#include <signal.h>

typedef enum _TEST_CMD_TYPE
{
    TEST_CMD_NON = 0,
    TEST_CMD_GET,
    TEST_CMD_SET,
    TEST_CMD_PUSH,
    TEST_CMD_DELE,
    TEST_CMD_ON,
    TEST_CMD_SETAUTH,
}TEST_CMD_TYPE;

STATIC void getHostFromAppid(char *p_host,const char *url)
{
    char *star_p = NULL,*end_p = NULL;
    star_p =  strchr(url,'/')+2;
    end_p = strchr(star_p,'.');
    memcpy(p_host,star_p,end_p - star_p);   
}
STATIC void getValue_callback
    (
    const Wilddog_Node_T* p_snapshot, 
    void* arg, 
    Wilddog_Return_T err
    )
{
    if(err < WILDDOG_HTTP_OK || err >= WILDDOG_HTTP_NOT_MODIFIED)
    {
        wilddog_debug("getValue fail!");
        return;
    }
    *(BOOL*)arg = TRUE;

    if(p_snapshot)
        wilddog_debug_printnode(p_snapshot);
    printf("\ngetValue success!\n");

    return;
}

STATIC void removeValue_callback(void* arg, Wilddog_Return_T err)
{
    if(err < WILDDOG_HTTP_OK || err >= WILDDOG_HTTP_NOT_MODIFIED)
    {
        wilddog_debug("removeValue failed!");
        return ;
    }
    wilddog_debug("removeValue success!");
    *(BOOL*)arg = TRUE;
    return;
}
STATIC void setValue_callback(void* arg, Wilddog_Return_T err)
{
                        
    if(err < WILDDOG_HTTP_OK || err >= WILDDOG_HTTP_NOT_MODIFIED)
    {
        wilddog_debug("setValue error!");
        return;
    }
    wilddog_debug("setValue success!");
    *(BOOL*)arg = TRUE;
    return;
}

STATIC void push_callback(u8 *p_path,void* arg, Wilddog_Return_T err)
{
                        
    if(err < WILDDOG_HTTP_OK || err >= WILDDOG_HTTP_NOT_MODIFIED)
    {
        wilddog_debug("push failed");
        return;
    }       
    wilddog_debug("new path is %s", p_path);
    *(BOOL*)arg = TRUE;
    return;
}

STATIC void addObserver_callback
    (
    const Wilddog_Node_T* p_snapshot, 
    void* arg,
    Wilddog_Return_T err
    )
{
    *(BOOL*)arg = TRUE;
    if(err < WILDDOG_HTTP_OK || err >= WILDDOG_HTTP_NOT_MODIFIED)
    {
        wilddog_debug("addObserver failed!");
        return;
    }
    wilddog_debug("Observe new data!");
    wilddog_debug_printnode(p_snapshot);
    printf("\n");

    return;
}
STATIC void auth_callback
    (
        void* arg,
        Wilddog_Return_T err
    )
{
    *(BOOL*)arg = TRUE;
    if(err < WILDDOG_HTTP_OK || err >= WILDDOG_HTTP_NOT_MODIFIED)
    {
        wilddog_debug("auth failed!;error =%d \n",err);
        return;
    }
}

Wilddog_T wilddog;
void sig_fn1(int sig)
{

    Wilddog_Node_T *p_head = NULL, *p_node = NULL;

	BOOL Finish = FALSE;

    p_head = wilddog_node_createObject(NULL);
    
    /*Create an node which type is UTF-8 Sring*/
    p_node = wilddog_node_createUString((Wilddog_Str_T *)"1", \
                                        (Wilddog_Str_T *)"222");
    
    /*Add p_node to p_head, then p_node is the p_head's child node*/
    wilddog_node_addChild(p_head, p_node);

    wilddog_getValue(wilddog, getValue_callback, &Finish);
    alarm(120);
    return;	
}


int main(int argc, char **argv) 
{
    char url[256] = "coap://esp.wilddogio.com";
    wilddog = wilddog_initWithUrl((Wilddog_Str_T *)url);
    signal(SIGALRM, sig_fn1);
    alarm(240);
    while(1)
        wilddog_trySync();
}

