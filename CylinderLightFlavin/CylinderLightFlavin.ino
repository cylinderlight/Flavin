// cylinderlight flavin mode sketch
// Marco Brianza
// June 24th 2013
// tested on Arduino IDE 1.5.2


#include <Ethernet.h>
#include <EthernetUdp.h>
#include <OSCBundle.h>
#include <OSCBoards.h>
#include <SPI.h>
#include "FastSPI_LED2.h"
#include <avr/eeprom.h>
#include <MemoryFree.h>


//LEDs stuff---------------------------------------
const byte X_LEDS=4;  // number of sectors
const byte Y_LEDS=60; //number of LEDs per secor
const int NUM_LEDS = X_LEDS * Y_LEDS;
const byte OUT_PIN=7;
struct CRGB leds[NUM_LEDS];
const byte NUM_PRESETS=5;

//network stuff------------------------------------
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 3); //the Arduino's IP
const byte MAX_MSG_SIZE=64;  // 64 sould be ok
byte packetBuffer[MAX_MSG_SIZE];  //buffer to hold incoming udp packet,
EthernetUDP Udp;
const unsigned int inPort = 10000;
const unsigned int outPort = 11000;
const int MESSAGE_MIN_TIME=300; // this avoids udating too quickly slider messages

byte r,g,b;

struct fRGB {float r; float g; float b;};


CRGB topColor[NUM_PRESETS][X_LEDS]; 
CRGB bottomColor[NUM_PRESETS][X_LEDS];

byte ledTop[X_LEDS]={0,0,0,0}; //selected item to be changed on Top
byte ledBottom[X_LEDS]={0,0,0,0}; //selected item to be changed on Bottom

int preset=0;
int presetA=0;
int presetB=0;
float crossFade=0;
float crossFadeTemp;
byte play=1; // play mode 1=STOP, 2 = PLAY
float fadeIncrement=0;

unsigned long time,lastTimeFade,lastTimeRGB,lastTimeLoop;

// ---------------------------------------
void setup(){   
 Serial.begin(115200);
 
  Ethernet.begin(mac,ip);
  Udp.begin(inPort);
 
 LEDS.setBrightness(255);
 LEDS.addLeds<TM1809, OUT_PIN>(leds,  NUM_LEDS);

 Serial.print("Mem=");
 Serial.println(freeMemory());


}

// ---------------------------------------  
void loop(){
  OSCBundle bundleIN;
  OSCMessage messageIN;

  int s;
  s = Udp.parsePacket();
  
  if(s)
  {
    Udp.read(packetBuffer,s);
    bundleIN.fill(packetBuffer,s);

    if(!bundleIN.hasError())
    {
      int bs=bundleIN.size();
//      Serial.print("bundle messages ");
//      Serial.println(bs);
      if (bs){
        bundleIN.route("/e", echo); //debug 
        
        bundleIN.route("/r", light_r);
        bundleIN.route("/g", light_g);
        bundleIN.route("/b", light_b);
         
         bundleIN.route("/0",ledtop_0);
         bundleIN.route("/1",ledtop_1);
         bundleIN.route("/2",ledtop_2);
         bundleIN.route("/3",ledtop_3);
         
         bundleIN.route("/4",ledbottom_0);
         bundleIN.route("/5",ledbottom_1);
         bundleIN.route("/6",ledbottom_2);
         bundleIN.route("/7",ledbottom_3);
         
         bundleIN.route("/s",savePreset);
         bundleIN.route("/l",loadPreset);
         
         bundleIN.route("/p/1/1",loadPreset_1);
         bundleIN.route("/p/2/1",loadPreset_2);
         bundleIN.route("/p/3/1",loadPreset_3);
         bundleIN.route("/p/4/1",loadPreset_4);
         bundleIN.route("/p/5/1",loadPreset_5);
        
         bundleIN.route("/A/1/1",loadPresetA_1);
         bundleIN.route("/A/2/1",loadPresetA_2);
         bundleIN.route("/A/3/1",loadPresetA_3);
         bundleIN.route("/A/4/1",loadPresetA_4);
         bundleIN.route("/A/5/1",loadPresetA_5);
        
         bundleIN.route("/B/1/1",loadPresetB_1);
         bundleIN.route("/B/2/1",loadPresetB_2);
         bundleIN.route("/B/3/1",loadPresetB_3);
         bundleIN.route("/B/4/1",loadPresetB_4);
         bundleIN.route("/B/5/1",loadPresetB_5);
        
         bundleIN.route("/c",setCrossFade);
         bundleIN.route("/t",timePlay);
         bundleIN.route("/i",setTimeSpeed);
         
         bundleIN.route("/pp", loadPreset_P);
         bundleIN.route("/pA", loadPreset_A); 
         bundleIN.route("/pB", loadPreset_B);             
         
      }
      else
      {
        messageIN.fill(packetBuffer,s);
        if(!messageIN.hasError())
        {
          messageIN.route("/e", echo); //debug
                    
          messageIN.route("/r", light_r);
          messageIN.route("/g", light_g);
          messageIN.route("/b", light_b);
           
           messageIN.route("/0",ledtop_0);
           messageIN.route("/1",ledtop_1);
           messageIN.route("/2",ledtop_2);
           messageIN.route("/3",ledtop_3);
           
           messageIN.route("/4",ledbottom_0);
           messageIN.route("/5",ledbottom_1);
           messageIN.route("/6",ledbottom_2);
           messageIN.route("/7",ledbottom_3);
           
           messageIN.route("/s",savePreset);
           messageIN.route("/l",loadPreset);
           
           messageIN.route("/p/1/1",loadPreset_1);
           messageIN.route("/p/2/1",loadPreset_2);
           messageIN.route("/p/3/1",loadPreset_3);
           messageIN.route("/p/4/1",loadPreset_4);
           messageIN.route("/p/5/1",loadPreset_5);
          
          
           messageIN.route("/A/1/1",loadPresetA_1);
           messageIN.route("/A/2/1",loadPresetA_2);
           messageIN.route("/A/3/1",loadPresetA_3);
           messageIN.route("/A/4/1",loadPresetA_4);
           messageIN.route("/A/5/1",loadPresetA_5);
          
          
           messageIN.route("/B/1/1",loadPresetB_1);
           messageIN.route("/B/2/1",loadPresetB_2);
           messageIN.route("/B/3/1",loadPresetB_3);
           messageIN.route("/B/4/1",loadPresetB_4);
           messageIN.route("/B/5/1",loadPresetB_5);
          
          
           messageIN.route("/c",setCrossFade);
           messageIN.route("/t",timePlay);
           messageIN.route("/i",setTimeSpeed);
           
           messageIN.route("/pp", loadPreset_P);
           messageIN.route("/pA", loadPreset_A); 
           messageIN.route("/pB", loadPreset_B);              

        }
      }
    }
  }
  
if (play==2){
            time=millis();
            if (time-lastTimeLoop>1000)
            {
              crossFadeTemp=crossFade+fadeIncrement;
  
             if ((crossFadeTemp>0) && (crossFadeTemp<254))
               {
                 crossFade=crossFadeTemp;
                 showRGBWithFade(); 
                 lastTimeLoop=time;
                 OSC_replay("/c",crossFade);  
               }  
            }
          }  
  
} // end main---------------------

//---------------------------------------------

void saveRGB(){
int i; 
  for (i=0; i<X_LEDS;i++){
    if (ledTop[i]) {
      topColor[preset][i].r=r;
      topColor[preset][i].g=g;
      topColor[preset][i].b=b;
    }  
  }

for (i=0; i<X_LEDS;i++){
    if (ledBottom[i]) {
      bottomColor[preset][i].r=r;
      bottomColor[preset][i].g=g;
      bottomColor[preset][i].b=b;
    }  
  }

   time=millis();
    
   // Serial.println(time,DEC);    
    
    if (time-lastTimeRGB>MESSAGE_MIN_TIME){
      showRGB();
      lastTimeRGB=time;
     // Serial.println("RGB");   
    }

}


void showRGB() {
  float slot=(float)Y_LEDS;//NUM_LEDS/(float)4.0;
  int ledN;
  float _r,_g,_b;

//    Serial.print("RGB preset=");
//    Serial.println(preset,DEC);

  for (int l=0; l<X_LEDS;l++)
  {
      for(int i = 0 ; i <slot; i++ ) {
       
        if ((l==0) || (l==2)){
         ledN=l*slot+i; 
        }
        else{
         ledN=((l+1)*slot-1)-i ;
        }
       _r = (float)(bottomColor[preset][l].r)/slot*(slot-i) + (float)topColor[preset][l].r/slot*(i);
       _g = (float)(bottomColor[preset][l].g)/slot*(slot-i) + (float)topColor[preset][l].g/slot*(i);
       _b = (float)(bottomColor[preset][l].b)/slot*(slot-i) + (float)topColor[preset][l].b/slot*(i);
      
//    Serial.print("red=");
//    Serial.println(_r,DEC);
       
       leds[ledN]=CRGB(_b,_r, _g);
      }  
  }
   calibrateLED();
   LEDS.show();
}

void showRGBWithFade() {
 float slot=(float)Y_LEDS;//NUM_LEDS/(float)4.0;
  int ledN;
  fRGB cA,cB; 
  float _r,_g,_b;
  
//  Serial.print("FADE presets=");
//  Serial.print(presetA,DEC);
//  Serial.print("-");
 //Serial.println(byte(crossFade),DEC);
  
  for (int l=0; l<X_LEDS;l++)
  {
      for(int i = 0 ; i <slot; i++ ) {
       
        if ((l==0) || (l==2)){
         ledN=l*slot+i; 
        }
        else{
         ledN=((l+1)*slot-1)-i ;
        }
       cA.r = (float)bottomColor[presetA][l].r/slot*(slot-i) + (float)topColor[presetA][l].r/slot*(i);
       cA.g = (float)bottomColor[presetA][l].g/slot*(slot-i) + (float)topColor[presetA][l].g/slot*(i);
       cA.b = (float)bottomColor[presetA][l].b/slot*(slot-i) + (float)topColor[presetA][l].b/slot*(i);
 
       cB.r = (float)bottomColor[presetB][l].r/slot*(slot-i) + (float)topColor[presetB][l].r/slot*(i);
       cB.g = (float)bottomColor[presetB][l].g/slot*(slot-i) + (float)topColor[presetB][l].g/slot*(i);
       cB.b = (float)bottomColor[presetB][l].b/slot*(slot-i) + (float)topColor[presetB][l].b/slot*(i);
       
       _r = (cA.r*(255.0-crossFade) + cB.r*crossFade)/255.0;
       _g = (cA.g*(255.0-crossFade) + cB.g*crossFade)/255.0;
       _b = (cA.b*(255.0-crossFade) + cB.b*crossFade)/255.0;
 

       leds[ledN]=CRGB(_b,_r, _g);
      }  

  }
  calibrateLED();
  LEDS.show();
}

//------- OSC callbacks --------------------------

void echo(OSCMessage &msg, int addrOffset){
 float echoValue=msg.getFloat(0); 
  //analogWrite(9,byte(r));  
//  logIp(_mes);
//  logOscAddress(_mes);
Serial.print("Mem=");
Serial.println(freeMemory());
//   Serial.print("Debug=");
//   Serial.println(echoValue);

   OSC_replay("/d", echoValue);
  
}
// ------------------------------------

void light_r(OSCMessage &msg, int addrOffset){
  r=byte(msg.getFloat(0));
//  Serial.print("red=");
//  Serial.print(r,DEC);
  saveRGB();
}

void light_g(OSCMessage &msg, int addrOffset){
  g=byte(msg.getFloat(0));
  saveRGB();
}

void light_b(OSCMessage &msg, int addrOffset){
   b=byte(msg.getFloat(0));
  saveRGB();
}
 //--------------------------------------------
void ledtop_0(OSCMessage &msg, int addrOffset){
ledTop[0]=byte(msg.getFloat(0));
//Serial.println(ledTop[0]);
}

void ledtop_1(OSCMessage &msg, int addrOffset){
ledTop[1]=byte(msg.getFloat(0));
}

void ledtop_2(OSCMessage &msg, int addrOffset){
ledTop[2]=byte(msg.getFloat(0));
}

void ledtop_3(OSCMessage &msg, int addrOffset){
ledTop[3]=byte(msg.getFloat(0));
}

//----------------------------------------------

void ledbottom_0(OSCMessage &msg, int addrOffset){
ledBottom[0]=byte(msg.getFloat(0));
//Serial.println(ledBottom[0]);
}

void ledbottom_1(OSCMessage &msg, int addrOffset){
ledBottom[1]=byte(msg.getFloat(0));
}

void ledbottom_2(OSCMessage &msg, int addrOffset){
ledBottom[2]=byte(msg.getFloat(0));
}

void ledbottom_3(OSCMessage &msg, int addrOffset){
ledBottom[3]=byte(msg.getFloat(0));
}

//----------------------------------------------

void savePreset(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
    eeprom_write_block((const void*)&topColor, (void*)0, sizeof(topColor));
    eeprom_write_block((const void*)&bottomColor, (void*)sizeof(topColor), sizeof(bottomColor));

  }
}

void loadPreset(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
   loadPresetFromFlash();
    showRGB();
  }
}

void loadPresetFromFlash(){
    eeprom_read_block((void*)&topColor, (void*)0, sizeof(topColor));
    eeprom_read_block((void*)&bottomColor, (void*)sizeof(topColor), sizeof(bottomColor));     
}


//--preset----------

void loadPreset_1(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
    preset=0;
    showRGB();

  }
}

void loadPreset_2(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
    preset=1;
    showRGB();
  }
}

void loadPreset_3(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
    preset=2;
    showRGB();    
  }
}

void loadPreset_4(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
    preset=3;
    showRGB();
  }
}

void loadPreset_5(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
    preset=4;
    showRGB();
  }
}



//--------preset A-----------------------
void loadPresetA_1(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
    presetA=0;

  }
}

void loadPresetA_2(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
    presetA=1;
  }
}

void loadPresetA_3(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
    presetA=2;
  }
}

void loadPresetA_4(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
    presetA=3;
  }
}

void loadPresetA_5(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
    presetA=4;
  }
}




//--------preset B-----------------------
void loadPresetB_1(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
    presetB=0;

  }
}

void loadPresetB_2(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
    presetB=1;
  }
}

void loadPresetB_3(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
    presetB=2; 
  }
}

void loadPresetB_4(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
    presetB=3;
  }
}

void loadPresetB_5(OSCMessage &msg, int addrOffset){
  if(msg.getFloat(0)){
    presetB=4;
  }
}

// presets with argument
// single call for preset_P
void loadPreset_P(OSCMessage &msg, int addrOffset){
  byte p;  
  p=byte(msg.getFloat(0));
  
//   Serial.print("Preset");
//   Serial.println(p); 
  if (p>0 && p<=NUM_PRESETS)
  {
   preset=(p-1);
   showRGB();
  }
}

void loadPreset_A(OSCMessage &msg, int addrOffset){
  byte p;  
  p=byte(msg.getFloat(0));
//   Serial.print("Preset A");
//   Serial.println(p);   
  if (p>0 && p<=NUM_PRESETS)
  {
   presetA=(p-1);
   showRGB();
  }
}

void loadPreset_B(OSCMessage &msg, int addrOffset){
  byte p;  
  p=byte(msg.getFloat(0));
  if (p>0 && p<=NUM_PRESETS)
  {
   presetB=(p-1);
   showRGB();
  }
}



//---------------------------------------
void setCrossFade(OSCMessage &msg, int addrOffset){
    crossFade=msg.getFloat(0);
      
     // Serial.print("crossFade=");

    time=millis();
    
   // Serial.println(time,DEC);    
    
    if (time-lastTimeFade>MESSAGE_MIN_TIME){
      showRGBWithFade();
      lastTimeFade=time;
      
//      Serial.print("crossFade=");
//      Serial.println(crossFade,DEC);      
    }
  // showRGBWithFade();   
    
  }    

 
void timePlay(OSCMessage &msg, int addrOffset){
  byte time;  
  time=byte(msg.getFloat(0)); 
   if (time) 
   {
     play=time; 
   }
//    Serial.print("timePlay=");
//    Serial.print(time,DEC);  
  }
 
void setTimeSpeed(OSCMessage &msg, int addrOffset){
   
  fadeIncrement=msg.getFloat(0); 
 OSC_replay("j",fadeIncrement);
  
  }
 
void OSC_replay(char* address, float value)
  {
//    Serial.print("response to ");
//    Serial.println(Udp.remoteIP());
    
    Udp.beginPacket(Udp.remoteIP(), outPort);
    OSCMessage msgOut(address);
    msgOut.add(value); 
    msgOut.send(Udp); // send the bytes to the SLIP stream
    Udp.endPacket(); // mark the end of the OSC Packet
    msgOut.empty(); // free space oc
  //  OSCBundle bndl; 
  //  bndl.add("/d").add(d);
  //  bndl.send(Udp); // send the bytes to the SLIP stream
  //  Udp.endPacket(); // mark the end of the OSC Packet
  //  bndl.empty(); // empty
    
  } 
 
 // calobrate color for better perception

void calibrateLED() {

  float gammaR=3;
  float gammaG=2;
  float gammaB=2.5;

 float outRangeR=220;
 float outRangeG=255;
 float outRangeB=255;
  
  float inRange=255;
  float r, g, b;
  float rC, gC, bC;
  
  //Serial.println("Calibrate");
  
  for (int i=0; i<NUM_LEDS; i++)
   {
    r=leds[i].r;
    g=leds[i].g;
    b=leds[i].b;
  

  rC=pow((r/inRange), gammaR)*outRangeR;
  gC=pow((g/inRange), gammaG)*outRangeG;
  bC=pow((b/inRange), gammaB)*outRangeB;
  
    leds[i].r=byte(rC);
    leds[i].g=byte(gC);
    leds[i].b=byte(bC);
    
  
//    Serial.print(r);
//    Serial.print(" ");
//    Serial.println(byte(rC));
   }
} 





