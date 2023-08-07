#include <EEPROM.h>
#include <ARDUINO.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display

//variables for buttons and buzzer
unsigned int mode = 0;
const int up = 13, back = 11, enter = 12, buzz = 7, address = 0;
bool sound = true;

//global declaration of variables for rpm as it uses interrupt function
unsigned long rpmtime;
float rpmfloat;
unsigned int rpm;
bool tooslow = 1;

//-----------------------------------------------------------------------------------------------------------------------------------------


void setup() {
  u8g2.begin();
  u8g2.setFont(u8g2_font_helvR10_te);
  pinMode(up, INPUT_PULLUP);
  pinMode(enter, INPUT_PULLUP);
  pinMode(back, INPUT_PULLUP);
  pinMode(buzz, OUTPUT);
  battery();
  delay(5000);
}

//-----------------------------------------------------------------------------------------------------------------------------------------


void loop() {

  if(!digitalRead(up))
  {
    buzzer();
    mode++;
  }

//-------------------------------------------------------

  if(mode > 4)
  {
    mode = 0;
  }

//-------------------------------------------------------

  switch (mode) 
  {
  case 0:
        menu1();
        display1();
        break;
  case 1:
        menu1();
        display2();
        break;
  case 2:
        menu2();
        display1();
        break;  
  case 3:
        menu2();
        display2();
        break; 
  case 4:
        menu3();
        display1();
        break;                   
  }

//-------------------------------------------------------

  if(!digitalRead(enter))
  {
    buzzer();

    switch (mode) {
    case 0:
          tachometer();
          break;
    case 1:
          capmeter();
          break;
    case 2:
          ultrasonic();
          break;  
    case 3:
          temperature();
          break;  
    case 4:
          setting();
          break;
    }
  }
}

//-----------------------------------------------------------------------------------------------------------------------------------------

void menu1() {

  u8g2.clearBuffer();
  u8g2.drawStr(14,17,"SELECT MODE");
  u8g2.drawStr(14, 38, "TACHOMETER");
  u8g2.drawStr(25, 60, "CAP METER");

}

void menu2() {

  u8g2.clearBuffer();
  u8g2.drawStr(14,17,"SELECT MODE");
  u8g2.drawStr(20, 38, "ULTRASONIC");
  u8g2.drawStr(12, 60, "TEMPERATURE");

}

void menu3() {

  u8g2.clearBuffer();
  u8g2.drawStr(14,17,"SELECT MODE");
  u8g2.drawStr(30, 38, "SETTINGS");

}

//-------------------------------------------------------

void display1() {

  u8g2.drawFrame(4, 24, 120, 18);
  u8g2.sendBuffer();
  
} 

void display2() {

  u8g2.drawFrame(4, 46, 120, 18);
  u8g2.sendBuffer();

}

//-----------------------------------------------------------------------------------------------------------------------------------------

void ultrasonic()
{
  #define trigPin 3
  #define echoPin 4
  unsigned int duration,distance;
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 
  u8g2.clearBuffer();
  u8g2.drawStr(0,20,"ULTRASONIC");
  u8g2.sendBuffer();
  u8g2.drawStr(0,40,"SCALE");
  u8g2.sendBuffer();
  delay(1000);
 
  while(digitalRead(back))
  {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2; 
    u8g2.clearBuffer();

    if(distance < 50)
    {
    u8g2.setCursor(0, 20);
    u8g2.setFont(u8g2_font_helvR14_tr);
    u8g2.print(F("cm  "));
    u8g2.print(distance);
    u8g2.sendBuffer();
    u8g2.setFont(u8g2_font_helvR10_te);
    }
    else 
    {
    
      u8g2.clearBuffer();
      u8g2.setCursor(0, 20);
      u8g2.print("OUT OF RANGE");
      u8g2.sendBuffer();
    
    }
  }
}

//-----------------------------------------------------------------------------------------------------------------------------------------

void tachometer()
{

  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= (1 << CS12); //Prescaler 256
  TIMSK1 |= (1 << TOIE1); //enable timer overflow
  pinMode(2, INPUT);
  attachInterrupt(0, RPM, FALLING); 

  while(digitalRead(back))
  {

    delay(1000);
    if (tooslow == 1) 
    {
      u8g2.clearBuffer();
      u8g2.drawStr(1, 20, "SLOW!");
      u8g2.sendBuffer();
    }
    else 
    {
      rpmfloat = 120 / (rpmtime/ 31250.00);
      rpm = round(rpmfloat);
      u8g2.clearBuffer();
      u8g2.setCursor(1,20);
      u8g2.print(rpm);
      u8g2.sendBuffer();
    }
  }
}

//-------------------------------------------------------

void RPM () 
{
  rpmtime = TCNT1;
  TCNT1 = 0;
  tooslow = 0;
}

//-------------------------------------------------------

ISR(TIMER1_OVF_vect) 
{
  tooslow = 1;
}

//-----------------------------------------------------------------------------------------------------------------------------------------

void capmeter()
{

  u8g2.clearBuffer();
  u8g2.drawStr(0, 20, "CAPMETER");
  u8g2.sendBuffer();

  #define analogPin      A0        // analog pin for measuring capacitor voltage
  #define chargePin      6         // pin to charge the capacitor - connected to one end of the charging resistor
  #define dischargePin   5         // pin to discharge the capacitor
  #define resistorValue  8160.0F   // change this to whatever resistor value you are using

  unsigned long startTime;
  unsigned long elapsedTime;
  float microFarads;               // floating point variable to preserve precision, make calculations
  float nanoFarads;

  pinMode(chargePin, OUTPUT);      // set chargePin to output
  pinMode(dischargePin, OUTPUT);
  digitalWrite(chargePin, LOW);
  digitalWrite(dischargePin, LOW);
  delay(1000);
  pinMode(dischargePin, INPUT);

  while(digitalRead(back))
  {
    u8g2.clearBuffer();
    u8g2.drawStr(0, 20, "READY");
    u8g2.sendBuffer();
  
    if(!digitalRead(enter))
    {
      buzzer();
      u8g2.clearBuffer();
      u8g2.drawStr(0, 20, "MEASURING");
      u8g2.sendBuffer();
      pinMode(dischargePin, INPUT);
      digitalWrite(chargePin, HIGH);   // set chargePin HIGH and capacitor charging
      startTime = millis();

      while(analogRead(analogPin) < 648){ }      // 647 is 63.2% of 1023, which corresponds to full-scale voltage

        elapsedTime= millis() - startTime;     // convert milliseconds to seconds ( 10^-3 ) and Farads to microFarads ( 10^6 ),  net 10^3 (1000)
        microFarads = ((float)elapsedTime / resistorValue) * 1000;
        u8g2.clearBuffer();
        u8g2.setCursor(0, 20);
        u8g2.print(elapsedTime);          // print the value to serial port
        u8g2.drawStr(40,20,"mS");         // print units and carriage return
        u8g2.sendBuffer();
        delay(1000);

        if (microFarads > 1)
        {
          u8g2.clearBuffer();
          u8g2.setCursor(0, 20);
          u8g2.print((long)microFarads);            // print the value to serial port
          u8g2.drawStr(0,40,"microFarads");         // print units and carriage return
          u8g2.sendBuffer();
          delay(1000);
        } 
        else
        {
          nanoFarads = microFarads * 1000.0;        // multiply by 1000 to convert to nanoFarads (10^-9 Farads)
          u8g2.clearBuffer();
          u8g2.setCursor(0, 20);
          u8g2.print((long)nanoFarads);             // print the value to u8g2 port
          u8g2.setCursor(0, 40);
          u8g2.println("nanoFarads");               // print units and carriage return
          u8g2.sendBuffer();
          delay(1000);
        }
      }
    pinMode(dischargePin, OUTPUT);
    digitalWrite(chargePin, LOW);               // set charge pin to  LOW
    digitalWrite(dischargePin, LOW);            // set discharge pin LOW

    while(analogRead(analogPin) > 0){ }          // wait until capacitor is completely discharged
    pinMode(dischargePin, INPUT);               // set discharge pin back to input
  }
}

//-----------------------------------------------------------------------------------------------------------------------------------------

void temperature()
{

  u8g2.clearBuffer();        
  u8g2.drawStr(0,20,"TEMPERATURE");         
  u8g2.sendBuffer();

  #define RT0 15000                   // Ω
  #define B   4194.25                 // K
  #define VCC 5.0                     //Supply voltage
  #define R   13910                   //R=10KΩ

  float RT, VR, ln, TX, T0, VRT;
  T0 = 25 + 273.15;
  delay(1000);

  while(digitalRead(back))
  {
    VRT = analogRead(A1);              //Acquisition analog value of VRT
    VRT = (5.0 / 1023.00) * VRT;      //Conversion to voltage
    VR = VCC - VRT;
    RT = VRT / (VR / R);               //Resistance of RT
    ln = log(RT / RT0);
    TX = (1 / ((ln / B) + (1 / T0))); //Temperature from thermistor
    TX = TX - 273.15;                 //Conversion to Celsius
    u8g2.clearBuffer(); 
    u8g2.setCursor(0, 20);  
    u8g2.print(TX);     
    u8g2.drawStr(50,20,"C");         
    u8g2.sendBuffer();
  }
}

//-----------------------------------------------------------------------------------------------------------------------------------------

void battery()
{

  int value = 0;
  float voltage, perc;

  value = analogRead(A2); 
  voltage = value * 3.7 / 1024;
  perc = float_map(voltage, 3.0, 4.2, 0, 100);
  u8g2.clearBuffer();
  u8g2.drawStr(0,20,"Voltage");
  u8g2.setCursor(50, 20); 
  u8g2.print(voltage);
  u8g2.drawStr(0,40,"Battery");
  u8g2.setCursor(50, 40);
  u8g2.print(perc);
  u8g2.sendBuffer();
}

//-------------------------------------------------------

long float_map(float x, float in_min, float in_max, float out_min, float out_max){
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//-----------------------------------------------------------------------------------------------------------------------------------------

void buzzer()
{
  if(EEPROM.read(address))
  {
    tone(7, 2000);
    delay(100);
    noTone(7);
    delay(100);
  }
  else
  {
    delay(200);
  }
}

//-----------------------------------------------------------------------------------------------------------------------------------------

void setting()
{
  while(digitalRead(back))
  {
    u8g2.clearBuffer();
    u8g2.drawStr(30, 17, "SETTINGS");
    u8g2.drawStr(43, 38, "TONE");
    u8g2.drawFrame(4, 24, 120, 18);
      if(EEPROM.read(address))
      {
        u8g2.drawStr(100, 38, "on");
      }
      else
      {
        u8g2.drawStr(100, 38, "off");
      }
      u8g2.sendBuffer();
      if (!digitalRead(enter)){
      delay(200);
      sound = !sound;
      EEPROM.update(address,sound);
    }
  }
}  
