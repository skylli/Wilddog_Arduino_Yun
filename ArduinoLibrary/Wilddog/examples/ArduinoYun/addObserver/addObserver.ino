/*
  Running process using wilddog class.

  This sk/etch demonstrate how to subscribe your data on Wilddog Yun
  using an Arduino Yún.

  created on 2015/11/20.
  by skyli.

  This example code is in the public domain.

  http://www.wilddog.com/

*/
#include <Wilddog.h>
#include "Wilddog_utility.h"
#define YOURURL  "coap://sky.wilddogio.com"
#define _KEY_PIN  "pin"

#define _MAX_PIN_   13

Wilddog *p_wd = NULL;


                        
int getPinValue(const char *src,int pinNumber,int *value)
{
  char pinName[100],pinValue[100];
  int len =100;

  memset(pinName,0,sizeof(pinName));
  memset(pinValue,0,sizeof(pinValue));

  sprintf(pinName,"%s%d",_KEY_PIN,pinNumber);
  if(sjson_get_value(src,pinName,pinValue,&len) < 0 )
    return 0;
    
 *value = atoi(pinValue);
  return 1;
}

void handleReceivePacket(const char *src)
{
    int i=0,value;

    for(i=0;i<=_MAX_PIN_;i++ )
    {
      if(getPinValue(src,i,&value) == 0)
        continue;
      if(value == 0)
        digitalWrite(i, LOW);
      else
        digitalWrite(i, HIGH);
      }
      
   return ;   
}
                        
void addObserverCallBack(const char *pdata, int error, void* arg)
{
  Serial.print("\n get error : ");
  Serial.print(error);
  if(pdata)
  {
      Serial.print("\n get newest data : ");
      Serial.print(pdata);
      /* get pin no and pin value */
       handleReceivePacket(pdata);
    }
  if (arg)
     Serial.print(*(char*)arg);
}

void setup() {
  int res = 0 ;
  // Initialize Bridge
  Bridge.begin();
  // Initialize Serial
  Serial.begin(9600);
  // Wait until a Serial Monitor is connected.
  while (!Serial);
  Serial.print(YOURURL);
  //wd.setValue(TEST_DATA,getcallbackFunc,(void*)NULL);
  
  Serial.print(YOURURL);
  p_wd = new  Wilddog(YOURURL);   
  Serial.print("\n star addObserver \n");
  
  res = p_wd->addObserver(WD_ET_VALUECHANGE,addObserverCallBack,(void*)NULL);
  if(res < 0 )
     Serial.print("\n subscribe fault \n ");
     
  Serial.print("\n setup end : ");
}

void loop()
{
  Serial.print("trysyncing ...\n");   
  if(p_wd)
    p_wd->trysync(); 
}
