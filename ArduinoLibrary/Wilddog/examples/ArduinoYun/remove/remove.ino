
/*
  Remove your Yun data.

  This sk/etch demonstrate how to remove node on Wilddog Yun
  using an Arduino Yún.

  created on 2015/11/20.
  by skyli.

  This example code is in the public domain.

  http://www.wilddog.com/

*/
#include <Wilddog.h>
#include "Wilddog_utility.h"
#define YOURURL  "coap://sky.wilddogio.com"

#define _MAX_PIN_   13

Wilddog *p_wd = NULL;
                      
void removeCallBack(const char *pdata, int error, void* arg)
{
  Serial.print("\n get error : ");
  Serial.print(error);
  if(pdata)
  {
      Serial.print("\n get receive  data : ");
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
  Serial.print("\n remove value\n");
  // set value on Wilddog yun.
  res = p_wd->removeValue(removeCallBack,(void*)NULL);
  if(res < 0 )
     Serial.print("\n remove value  fault \n ");
     
}

void loop()
{
  Serial.print("trysyncing ...\n");   
  // receive and transmit.
  if(p_wd)
    p_wd->trysync();
    
}
