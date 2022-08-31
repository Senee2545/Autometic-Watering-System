#define BLYNK_PRINT Serial
#define RXD2 16
#define TXD2 17
#define LINE_TOKEN  "81d5wfUFxa6RSkMNS9bInMJdCdAZgVDoAdaL3trz6Ak"
/* Fill-in your Template ID (only if using Blynk.Cloud) */
//#define BLYNK_TEMPLATE_ID   "YourTemplateID"

#include <TridentTD_LineNotify.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "THNi2gtYZIXy4G9viPuJvM2YlDLaXDJ8";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Senee";
char pass[] = "12345543210";
String inputString= ""; // a string to hold incoming data 
boolean stringComplete= false; // whether the string is complete
int btn = 0; // btn ON and OFF // 
int btnFA = 0; // btn NPK46 // 
int btnFB = 0; // btn NPK15 //
int btnSV = 0; // btn solenoid valve //
String StartDate = ""; // choose date //
int FertilizerA = 0; // choose value for NPK46 //
int FertilizerB = 0; // choose value for NPK15 //
String sMois = ""; // substring Mois //
int Mois = 0; //  Recieve the data from STM32F4 //
int date = 0; // send date to STM32F4//

//Variable for update to blynk
String startHours = "";
String startMinute = "";
String SetTimeStart = ""; // To combline startHours and startpMinute together //
String sStart = ""; // For LineNotify comblined text with the data //
String stopHours = "";
String stopMinute = "";
String SetTimeStop = ""; // To combline stopHours and stopMinute together //

// Blynk variable //
BlynkTimer clocktimer;
BlynkTimer mytimer;
BlynkTimer mytimer2; 
BlynkTimer mytimer3; 
BlynkTimer mytimer4; 
BlynkTimer Settimer;
WidgetRTC rtc;
WidgetLCD lcd(V3);

String currentDate; // Current Date using rtc widget //
String currentTime; // Current Time using rtc widget //


void setup()
{
  // Debug console
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Blynk.begin(auth, ssid, pass, IPAddress(147, 182, 177, 185), 8080);
  rtc.begin();
  clocktimer.setInterval(100L, clockDisplay); //1sec
  LINE.setToken(LINE_TOKEN);
  mytimer.setInterval(1000L, sendDataUp);
  Settimer.setInterval(1000L, SetData);
  mytimer2.setInterval(100L,RecieveDataToSTM32F4);
  mytimer3.setInterval(1000L,RecieveData);
  mytimer4.setInterval(100L,sendDatatoSTM32F4);
  inputString.reserve(200);
  
}

BLYNK_WRITE(V10) {
  StartDate = param.asInt();
  date = param.asInt();
}

// *This one for LineNotify to send the data into Line applicattion if I clike the button on/off automatic* //
// *If I click on and off it will show different data by clicking the button* //
BLYNK_WRITE(V2){
    btn = param.asInt();
    // *ON Button* //
    if(btn == 0){
      LINE.notify("\n"
                  "Automatic Watering System Status:\n" 
                  +sStart+ "\n"
                  "Stop Time: " +SetTimeStop+ "\n"   
                  "Moisture: " +Mois+ "%\n"
                  "System Soil Moisture: Stop");
    }
    // *OFF Button* //
    else{
      LINE.notify("\n"
                  "Automatic Watering System Status:\n" 
                  +sStart+ "\n"
                  "Start Time: " +SetTimeStart+ "\n"   
                  "Moisture: " +Mois+ "%\n"
                  "System Soil Moisture: Working");
    }
}
 
void SetData(){
    if(btn == 0){
      SetTimeStart = startHours +":"+ startMinute; // *Set the data to combine it together* //
      SetTimeStop = stopHours +":"+ stopMinute; 
      sStart = "System Day: " + StartDate;
    
      Blynk.virtualWrite(V1,"AUTOMATIC: OFF");
      lcd.print(0, 0, ">WATERING STATUS:"); // This part is for the lcd advance widget in blynk //
      lcd.print(0, 1, ">STOPPED"); // If the button is 0 it will show Stop //
    }
    else{
      Blynk.virtualWrite(V1,"AUTOMATIC: ON");
      lcd.print(0, 0, ">WATERING STATUS:");
      lcd.print(0, 1, ">WORKING"); // If the button is 1 it will show Working //

    }
}


BLYNK_WRITE(V13) {
  FertilizerA = param.asInt();
}

BLYNK_WRITE(V14) {
  FertilizerB = param.asInt();
}

// *Button NPK46 to send text into the following widget* //
BLYNK_WRITE(V4) {
  btnFA = param.asInt();
  if(btnFA == 0){
    Blynk.virtualWrite(V40,"OFF");
  }
  else{
    Blynk.virtualWrite(V40,"ON");
  }
}

// *Button NPK15 to send text into the following widget* //
BLYNK_WRITE(V5) {
  btnFB = param.asInt();
  if(btnFB == 0){
    Blynk.virtualWrite(V41,"OFF");
  }
  else{
    Blynk.virtualWrite(V41,"ON");
  }
}

// *This one for Solenoid Valve switch* //
// *And send the text into the following widget* //
BLYNK_WRITE(V6) {
  btnSV = param.asInt();
  if(btnSV == 0){
    Blynk.virtualWrite(V42,"OFF"); // if btnSV is 0 that's mean it off //
  }
  else{
    Blynk.virtualWrite(V42,"ON"); // if btnSV is 1 that's mean it on //
  }
}

BLYNK_WRITE(V11) {
  startHours = param.asInt();
}

BLYNK_WRITE(V12) {
  startMinute = param.asInt();
}

BLYNK_WRITE(V15) {
  stopHours = param.asInt();
}

BLYNK_WRITE(V16) {
  stopMinute = param.asInt();
}

// *This part is for Current Date and Time* //
void clockDisplay() {
  currentDate = String(day()) + "/" + month() + "/" + year();
  currentTime = String(hour()) + ":" + minute() + ":" + second();
  Blynk.virtualWrite(V70, currentDate);
  Blynk.virtualWrite(V71, currentTime);
  Blynk.virtualWrite(V72, currentDate);
  Blynk.virtualWrite(V73, currentTime);
  Blynk.virtualWrite(V74, currentTime);
  Blynk.virtualWrite(V75, currentDate);
}

void loop()
{
  Blynk.run();
  mytimer.run();
  clocktimer.run();
  Settimer.run();
  mytimer2.run();
  mytimer3.run();
  mytimer4.run();
  serialEvent();
  Serial.println(SetTimeStart);

  // *Get the data from STM32F4 and call it in Arduino* //
  // *Substring for Mois* // 
  if (stringComplete){
    stringComplete = false;
    sMois = inputString.substring(1,5); 
    Mois = sMois.toInt();
    inputString = "";
  }

  // *This part is just want to show in monitor page *//
  // *(NPK46)* //
  if (FertilizerA <= 10) {
    Blynk.virtualWrite(V32, FertilizerA);
    Blynk.virtualWrite(V37, FertilizerA);
  }
  else {
     
  }

  // *This part is just want to show in monitor page *//
  // *(NPK15)* //
  if (FertilizerB <= 10) {
    Blynk.virtualWrite(V33, FertilizerB);
    Blynk.virtualWrite(V38, FertilizerB);
  }
  else {
  
  }
}

// *Sending the data about date and time to the following widget* // 
void sendDataUp(){
  Blynk.virtualWrite(V34, SetTimeStart);
  Blynk.virtualWrite(V31, SetTimeStart);
  Blynk.virtualWrite(V30, StartDate);
  Blynk.virtualWrite(V36, StartDate);
  Blynk.virtualWrite(V35, SetTimeStop);
}

// *Sending the data Mois to the following widget* //
void RecieveData(){
  Blynk.virtualWrite(V17, Mois);
  Blynk.virtualWrite(V18, Mois);
}

void serialEvent(){
  while (Serial2.available()){
    char inChar = (char)Serial2.read();
    inputString += inChar; 
    if (inChar == '\n') {
      stringComplete = true; 
    }
  }
}

// *This part is is recieve the data from STM32F4 (Moisture covert by 0-4095 to 0 -99)* //
void RecieveDataToSTM32F4(){
  Serial2.print("#%.1f");
  Serial2.print(Mois);
  Serial2.print("\n");
}

// *This part is the data that send to show in STM32F4* //
void sendDatatoSTM32F4(){
  Serial2.print("#DATE=");
  Serial2.print(date);
  Serial2.print("\n");

  Serial2.print("#STARTHOUR=");
  Serial2.print(startHours);
  Serial2.print("\n");

  Serial2.print("#STARTMINUTE=");
  Serial2.print(startMinute);
  Serial2.print("\n");

  Serial2.print("#STOPHOUR=");
  Serial2.print(stopHours);
  Serial2.print("\n");

  Serial2.print("#STOPMINUTE=");
  Serial2.print(stopMinute);
  Serial2.print("\n");

  Serial2.print("#NPK46=");
  Serial2.print(FertilizerA);
  Serial2.print("\n");

  Serial2.print("#NPK15=");
  Serial2.print(FertilizerB);
  Serial2.print("\n");
  
  Serial2.print("#VALVEBTN=");
  Serial2.print(btnSV);
  Serial2.print("\n");

  Serial2.print("#NPK46BTN=");
  Serial2.print(btnFA);
  Serial2.print("\n");

  Serial2.print("#NPK15BTN=");
  Serial2.print(btnFB);
  Serial2.print("\n");
}
