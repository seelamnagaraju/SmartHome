
#ifndef _thingspeak_h_
#define _thingspeak_h_

class ThingSpeak 
{
  public:
        int init(const char *channelApiKey, unsigned long channelId, const char *talkbackApiKey, unsigned long talkbackId);
  
        void channelInit();
        int  channelSetField(const char *field, int value);
        int  channelSetField(const char *field, long value);
        int  channelSetField(const char *field, double value);
        int  channelSetField(const char *field, const char * value);
        int  channelUpdate();      
        char* commandExecute();   // return command string in *cmd
    
  private:
      const char * _channelApiKey;
      long _channelId;   
      const char * _talkbackApiKey;
      long _talkbackId;
};


extern ThingSpeak ThingSpeak;
extern void wifi_init(const char* wlan_ssid, const char* wlan_pass);

#endif





