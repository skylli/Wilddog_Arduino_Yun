
/*
  Running process using wilddog class.

  This sk/etch demonstrate how to push your data on Wilddog Yun
  using an Arduino Yún.

  created on 2015/11/20.
  by skyli.

  This example code is in the public domain.

  http://www.wilddog.com/

*/
#include <Wilddog.h>
#include "Wilddog_utility.h"
#define YOURURL  "coap://sky.wilddogio.com"
#define SETTING_DATA "{\"pin1\":\"1\"}"

Wilddog *p_wd = NULL;
                      
void pushCallBack(const char *pdata, int error, void* arg)
{
  Serial.print("\n get error : ");
  Serial.print(error);
  if(pdata)
  {
      Serial.print("\n get path : ");
      Serial.print(pdata);
    }
  if (arg)
     Serial.print(*(char*)arg);
}

void setup() {
  int res = 0;
  // Initialize Bridge
  Bridge.begin();
  // Initialize Serial
  Serial.begin(9600);
  // Wait until a Serial Monitor is connected.
  while (!Serial);
  
  Serial.print(YOURURL);
  p_wd = new  Wilddog(YOURURL);   
  Serial.print("\n push \n");
  // set value on Wilddog yun.
  res = p_wd->push(SETTING_DATA,pushCallBack,(void*)NULL);
  if(res < 0 )
     Serial.print("\n push  fault \n ");
}

void loop()
{
  Serial.print("trysyncing ...\n");   
  // receive and transmit.
  if(p_wd)
    p_wd->trysync();
    
}