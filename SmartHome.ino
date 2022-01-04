/*
Project 3 - Smart Home
By: Nagaraju Seelam
Version: Final
Date: September 11, 2020

Requirements:
1. Hardware setup:
   * Must have a LED to represent lighting.
   * Must have temperature sensor:
      Convert values to Fahrenheit.
   * Must have photo sensor.
   * Must have at least one button to control lighting ON/OFF.
   * Must have Wi-Fi.
2. Cloud setup:
   * Using ThingSpeak.com platform.
   * Set up at least one channel to collect data:
      Light, temperature and LED state.
   * Set up at least one talkback for remote light control.
3. Mobile setup:
   * Use a mobile HTML5 app to view data and control LED via ThingSpeak.
4. Use cases:
   * Collect home data and upload to the cloud.
   * View current data from mobile phone. 
   * Remotely turn ON/OFF lighting.
   * Design your own use case.
   
*/

#include "MemoryFree.h"
#include "thingspeak.h"

// Local Network Settings
//#define WLAN_SSID         "pcville"  // Your network SSID (name)
//#define WLAN_PASS         "useruser" // Your network password

#define WLAN_SSID         "Nagaraju"  // Your network SSID (name)
#define WLAN_PASS         "changeme" // Your network password


#define CHANNEL_ID        154049
#define TALKBACK_ID       10206
#define CHANNEL_API_KEY   "27JR0ASJHRZ63LEE"
#define TALKBACK_API_KEY  "UVK1DN0OGSYI9HH5"

#define  PIN_SWITCH  2
#define  PIN_SW_FAN  3
#define  PIN_LED     4
#define  PIN_FANA    8
#define  PIN_FANB    9

#define  PIN_LIGHT  A0
#define  PIN_TEMP   A1

volatile boolean b_ledState=LOW;
volatile boolean b_LDR_State=LOW;
volatile boolean b_temperatureState=LOW;
volatile char fanState=0;

int  lightLimit; // cut-off value for light control
int  light = 0;
float tempVal = 0;
char * led_status;

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void setup()
{
  // Start Serial for debugging on the Serial Monitor
  Serial.begin(9600);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_FANA, OUTPUT);
  pinMode(PIN_FANB, OUTPUT);
  FAN_Off();
  pinMode(PIN_TEMP, INPUT);     
  pinMode(PIN_LIGHT, INPUT);
  pinMode(PIN_SWITCH, INPUT);
  pinMode(PIN_SW_FAN, INPUT_PULLUP);
  attachInterrupt(0, buttonSwitchPushed, FALLING);
  attachInterrupt(1, fanSwitchPushed, FALLING);
  lightCalibration(); 
  Serial.print("Free RAM: "); Serial.println(freeMemory(), DEC);  
  wifi_init(WLAN_SSID, WLAN_PASS);
  return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/    
void loop()
{  
  light = checkPhotoSensor();
  readTemperatureSensor();
  setledState();
  setFanState();
  ThingSpeakChannelUpdate();
  ThingSpeakCommandExecute();
  delay(5000);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int checkPhotoSensor()
{
  static boolean sensorSwitch=false;
  
  int sensorValue = analogRead(PIN_LIGHT);
  boolean newState;
  if (sensorValue < lightLimit)
       newState = true;
  else
       newState = false;

  if (sensorSwitch == false && newState == true)
  {
        Serial.println("light sensor turn on light");
        b_LDR_State = true;
  }

  if (sensorSwitch == true && newState == false)
  {
        Serial.println("light sensor turn off light");
        b_LDR_State = false;
  }

  sensorSwitch = newState;
  return sensorValue;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
 // read temperature value:
int readTemperatureSensor(void) 
{
 // Find temperature value:
  float voltage = analogRead(PIN_TEMP);
  tempVal = log(((10240000/(voltage)) - 10000));
  tempVal = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempVal * tempVal ))* tempVal );
  tempVal = ((tempVal - 273.15) * 9.0)/ 5.0 + 32.0;    // Convert Kelvin to Fahrenheit
  
  /* uncomment this to get temperature in farenhite */
  Serial.print("TEMPRATURE = ");
  Serial.print(tempVal);
  Serial.print("*F");
  Serial.println();
  delay(10);

  /*
  if (tempVal>90)
  {
      b_temperatureState = 1;
  }
  else
  {
      b_temperatureState = 0;
  }
 */
  
  return 1;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int ThingSpeakChannelUpdate()
{
    ThingSpeak.init(CHANNEL_API_KEY, CHANNEL_ID, TALKBACK_API_KEY, TALKBACK_ID);
    ThingSpeak.channelInit();
    ThingSpeak.channelSetField("field1", light);
    ThingSpeak.channelSetField("field2", tempVal);
    ThingSpeak.channelSetField("field3", led_status);
    ThingSpeak.channelSetField("field4", b_ledState);
    ThingSpeak.channelUpdate();
    delay(100); 
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void ThingSpeakCommandExecute(void)
{
   ThingSpeak.init(CHANNEL_API_KEY, CHANNEL_ID, TALKBACK_API_KEY, TALKBACK_ID);
  char *cmd=ThingSpeak.commandExecute();
  if (strlen(cmd)==0) 
  {
      Serial.println(F("No command to execute"));
  } 
  else 
  {
      Serial.print(F("Got command to execute: ")); 
      Serial.println(cmd);

      if (strcmp(cmd, "on") == 0)  
      {
          b_ledState=1;
          Serial.println("LED ON"); 
      }
      else  
      if (strcmp(cmd, "off") == 0)  
      {
          b_ledState=0;
          Serial.println("LED OFF");
      }
      else  
      if (strcmp(cmd, "FAN_ON") == 0)  
      {
          fanState=1;
          Serial.println("FAN ON");
      }
      else  
      if (strcmp(cmd, "FAN_OFF") == 0)  
      {
          fanState=1;
          Serial.println("FAN OFF");
      }

      setFanState();
      setledState();
  }
  delay(100);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
//Calibrate Lighting Condition.
void lightCalibration()
{  
  //Define variables
  int sensorValue = 0;
  int sensorMin = 1023;
  int sensorMax = 0;

  Serial.println("vary light condition for calibration");

  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  //Calibrate during the first 5 seconds

  while (millis() < 5000)
  {
    sensorValue = analogRead(PIN_LIGHT);
    //Record the Maximum sensor value
    if (sensorValue > sensorMax)
      {
      sensorMax = sensorValue;
      }
    //Record the Minimum sensor value
    if (sensorValue < sensorMin)
      {
      sensorMin = sensorValue;
      }
    delay(100);
  }
  digitalWrite(13, LOW);

  lightLimit = (sensorMin + sensorMax)/2;
  Serial.print("Light limit value is ");
  Serial.println(lightLimit);

} //End of Lighting Calibration



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void buttonSwitchPushed()
{
  static long lastTime = 0;

  if (millis() - lastTime < 200)return;
  lastTime = millis();

  Serial.println("Push button toggle light");
  b_ledState = !b_ledState;
  setledState();
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void setledState()
{
  if ((b_ledState==1) || (b_LDR_State == 1))
  {
     led_status="on";
     digitalWrite(PIN_LED, HIGH);
  }
  else
  if ((b_ledState==0) || (b_ledState == 0))
  {
     led_status="off";
     digitalWrite(PIN_LED, LOW);
  }
  return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void fanSwitchPushed() 
{
  static long lastTime = 0;
  
  if (millis() - lastTime < 200) return;
    lastTime = millis();
  
  Serial.println("Fan Push button toggle state");
  if (fanState==0) 
    fanState = 1;
  else
      fanState = 0;
      
   setFanState();
   return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void setFanState() 
{
  if ((fanState==1) || (b_temperatureState == 1))
  {
    FAN_Forward();
  }
  else
  if ((fanState==0) || (b_temperatureState == 0))
  {
    FAN_Off();
  }
  else 
  if (fanState==2) 
  {
    FAN_Reverse();
  }
  return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int FAN_Forward ()
{
  digitalWrite(PIN_FANA, LOW);
  digitalWrite(PIN_FANB, HIGH);
  return 1;
}


int FAN_Reverse ()
{
  digitalWrite(PIN_FANA, HIGH);
  digitalWrite(PIN_FANB, LOW);
  return 1;
}


int FAN_Off ()
{
  digitalWrite(PIN_FANA, HIGH);
  digitalWrite(PIN_FANB, HIGH);
  return 1;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
