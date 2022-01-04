#include "thingspeak.h"
#include "http_request.h"

#include <SPI.h>
#include <WiFi101.h>

WiFiClient client;

// forward declaration of helper functions
int getResponseCode();
char* getContent();    // return prointer to body inside response

// define the global object first
class ThingSpeak ThingSpeak;

#define   BUF_LENGTH  128
char buf[BUF_LENGTH]; // for receiving response body (no header)
const char host[]="api.thingspeak.com";
const int port=80;

int ThingSpeak::init(const char *channelApiKey, unsigned long channelId, const char *talkbackApiKey, unsigned long talkbackId)
{
    _channelApiKey = channelApiKey;
    _channelId = channelId;
    _talkbackApiKey = talkbackApiKey;
    _talkbackId = talkbackId;
    return 0;
}

void ThingSpeak::channelInit()
{
    buf[0]=0;
}

int ThingSpeak::channelSetField(const char *field, const char *value)
{
    if (strlen(buf) + strlen(field) + strlen(value) + 2 > BUF_LENGTH) 
    {
        Serial.println(F("ERROR: buffer overflow in channelSetField()"));
        for(;;);
    }
    
    strcat(buf, "&");
    strcat(buf,field);
    strcat(buf, "=");
    strcat(buf, value); 
    return strlen(buf);
}

int ThingSpeak::channelSetField(const char *field, int value)
{
  char b[10];
  return channelSetField(field, itoa(value, b, 10));
}

int ThingSpeak::channelSetField(const char *field, long value)
{
  char b[10];
  return channelSetField(field, ltoa(value, b, 10));
}

int ThingSpeak::channelSetField(const char *field, double value)
{
  char temp[20];
  return channelSetField(field, dtostrf(value, 1, 3, temp));
}


int ThingSpeak::channelUpdate()
{
  char temp[10];
  int contentLength=strlen(buf);
  
  Serial.println(F("Updating channel ...")); Serial.println(buf);

  /* Try connecting to the website */
  if (!client.connect(host, port)) 
  {
    Serial.print(F("Failed to connect to server : ")); 
    Serial.println(host);
    return 1;
  }

#ifdef DEBUG
    Serial.print(F(CMD_UPDATE_1));
    Serial.print(_channelApiKey);
    Serial.println(buf);
    Serial.print(F(CMD_UPDATE_2));
#endif
  
  client.print(F(CMD_UPDATE_1));
  client.print(_channelApiKey);
  client.print(buf);
  client.print(F(CMD_UPDATE_2));
  
  int code= getResponseCode();
  char * content=getContent();
  
  Serial.print(F("Response code : "));
  Serial.println(code);
  
  Serial.print(F("Response : "));
  Serial.println(content);
  
  client.stop();
  return 0;
}

char* ThingSpeak::commandExecute() 
{
  Serial.println(F("Fetch next command ..."));
  
  /* Try connecting to the website */
  if (!client.connect(host, port)) 
  {
    Serial.print(F("Failed to connect to server : ")); 
    Serial.println(host);
    return (char*)"";
  }

  client.print(F(CMD_EXECUTE_1));
  client.print(itoa(_talkbackId, buf, 10));
  client.print(F(CMD_EXECUTE_2));
  client.print(_talkbackApiKey);
  client.print(F("\n\n"));

  int code= getResponseCode();
  char * content=getContent();
  
  Serial.print(F("Response code : "));
  Serial.println(code);
  
  Serial.print(F("Response : "));
  Serial.println(content);
  
  client.stop();
  return content;
}


// ============ helpers ==============
void getRest() 
{
  char c;
  int i=0;
    while(client.connected() && !client.available()) delay(1);
    while (client.available()) {
        c = client.read();
        // Serial.print(c);
        buf[i++] = c;
        if (i==BUF_LENGTH) {
          Serial.println(F("buffer overflow in getFullResponse()"));
          for(;;);
        }
    }
    buf[i]=0;
}

void getLine()
{
  char c;
  int i=0;
  while(client.connected() && !client.available()) delay(1);
  while (client.available()) {
    c = client.read();

    // check EOL
    if (c == '\r') {
      if (client.peek() == '\n') client.read(); // consume EOL
      break;
    }
    
    // Serial.print(c);
    buf[i++] = c;
    if (i==BUF_LENGTH) {
      Serial.println(F("buffer overflow in getFullResponse()"));
      for(;;);
    }
  }
  buf[i]=0;
}
  
int getResponseCode() 
{
  getLine();
  return atoi(buf+9);
}

char* getContent() 
{
  for(;;) {
    getLine();
    if (buf[0] == 0) break;
  }

  getRest();
  return buf;
}









