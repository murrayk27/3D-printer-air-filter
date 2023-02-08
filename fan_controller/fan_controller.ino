
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h> // includes the LiquidCrystal Library 
 
// Data wire is conntec to the Arduino digital pin 8
#define ONE_WIRE_BUS 8

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
float set_temp = 24.0; // if temp lower than set the fan will idle at lowest speed
float max_temp = 30.0; // if temp higher than max the fan run at full blast

float max_power_frac = 0.7; // The maximum power fraction that the fans can be set to e.g 0.7 will run the fans at 70%
float min_power_frac = 0.2; // The maximum power fraction that the fans can be set to e.g 0.7 will run the fans at 70%
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

int numberOfDevices; // Number of temperature devices found
DeviceAddress tempDeviceAddress;
DeviceAddress Thermometer;

uint8_t sens1_ID[8] = {0x28, 0x13, 0xE3, 0x16, 0xA8, 0x01, 0x3C, 0xB3};  // Address of sensor 1
uint8_t sens2_ID[8] = {0x28, 0x90, 0xE7, 0x16, 0xA8, 0x01, 0x3C, 0x1F};  // Address of sensor 2
uint8_t sens3_ID[8] = {0x28, 0xBF, 0xC1, 0x16, 0xA8, 0x01, 0x3C, 0xE9};  // Address of sensor 3

int deviceCount = 0;

LiquidCrystal lcd(2, 5, 6, 11, 4, 7); // Creates an LCD object. Parameters: (rs, enable, d4, d5, d6, d7)

//configure Timer 1 (pins 9,10) to output 25kHz PWM
// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{ 
  for (uint8_t i = 0; i < 8; i++)
  {
    Serial.print("0x");
    if (deviceAddress[i] < 0x10) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("");
}

void setupTimer1(){
    //Set PWM frequency to about 25khz on pins 9,10 (timer 1 mode 10, no prescale, count to 320)
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
    TCCR1B = (1 << CS10) | (1 << WGM13);
    ICR1 = 320;
    OCR1A = 0;
    OCR1B = 0;
}
//configure Timer 2 (pin 3) to output 25kHz PWM. Pin 11 will be unavailable for output in this mode
void setupTimer2(){
    //Set PWM frequency to about 25khz on pin 3 (timer 2 mode 5, prescale 8, count to 79)
    TIMSK2 = 0;
    TIFR2 = 0;
    TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
    TCCR2B = (1 << WGM22) | (1 << CS21);
    OCR2A = 79;
    OCR2B = 0;
}
//equivalent of analogWrite on pin 9
void setPWMF1(float f){
    f=f<0?0:f>1?1:f;
    OCR1A = (uint16_t)(320*f);
}
//equivalent of analogWrite on pin 10
void setPWMF3(float f){
    f=f<0?0:f>1?1:f;
    OCR1B = (uint16_t)(320*f);
}
//equivalent of analogWrite on pin 3
void setPWMF2(float f){
    f=f<0?0:f>1?1:f;
    OCR2B = (uint8_t)(79*f);
}

void setup(){
    lcd.begin(16, 2);
    lcd.clear();
    lcd.print("Init Arduino"); // Splash screen when arduino boots up

    //enable outputs for Timer 1
    Serial.begin(9600);      
  // Start up the library
    sensors.begin();

    // Grab a count of devices on the wire
  
  // locate devices on the bus
    Serial.println("Locating devices...");
    Serial.print("Found ");
    deviceCount = sensors.getDeviceCount();
    Serial.print(deviceCount, DEC);
    Serial.println(" devices.");
    Serial.println("");

  for (int i = 0;  i < deviceCount;  i++)
  {
    Serial.print("Sensor ");
    Serial.print(i+1);
    Serial.print(" : ");
    sensors.getAddress(Thermometer, i);
    printAddress(Thermometer); // prints the address of each connected sensor
  }
    // Configuring pins 9,10,3 to output the PWM frequency
    pinMode(9,OUTPUT); //1A
    pinMode(10,OUTPUT); //1B
    setupTimer1();
    //enable outputs for Timer 2
    pinMode(3,OUTPUT); //2
    setupTimer2();
    //note that pin 11 will be unavailable for output in this mode!
    //example...
    setPWMF1(0.2f); //set duty to 20% on pin 9 labeled 'fan 1'
    setPWMF2(0.2f); //set duty to 20% on pin 3 'fan 2'
    setPWMF3(0.2f); //set duty to 20% on pin 10 'fan 3'
}

void adjust_speed(int fan_ID, float sens_temp) // function to adjust the fan speed based on he sensor temperature
{
  float temp_frac = 0.5;
  if (sens_temp > max_temp)
    { temp_frac = max_power_frac; // set fan to full power if temp above max
    }
    else if (sens_temp > set_temp)
    {
      float power_frac = (max_power_frac-min_power_frac)*(sens_temp-set_temp)/(max_temp-set_temp)+min_power_frac;       
      temp_frac = power_frac; // proportionally ramp up fan speed
    }       
    else
    { temp_frac = min_power_frac;
    }    
    
    if (fan_ID == 1) // yeah it's gross, I know
    {setPWMF1(temp_frac);      
    }
    else if(fan_ID == 2)
    {setPWMF2(temp_frac); 
    }
    else if(fan_ID == 3)
    {setPWMF3(temp_frac); 
    }
}

void loop(){
    sensors.requestTemperatures(); 
  
    Serial.print("Sensor 1 Temp: ");
    float temp1 = sensors.getTempC(sens1_ID);
    adjust_speed(1,temp1);
    Serial.print(temp1);
    Serial.print(" C | ");

    Serial.print("Sensor 2 Temp: ");

    float temp2 = sensors.getTempC(sens2_ID); 
    adjust_speed(2,temp2);  
    Serial.print(temp2);
    Serial.print(" C | ");
 
    Serial.print("Sensor 3 Temp: ");

    float temp3 = sensors.getTempC(sens3_ID); 
    adjust_speed(3,temp3); 
    Serial.print(temp3);
    Serial.print(" C ");
    Serial.print("\n"); 
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("1:");
    if (temp1>-100)
    {
      lcd.print(temp1,1);
      lcd.print((char)223);            
    }    
    else
    {
      lcd.print("NA");
    }    
    lcd.setCursor(0, 1);
    lcd.print("2:");
    if (temp2>-100)
    {
      lcd.print(temp2,1);
      lcd.print((char)223);            
    }    
    else
    {
      lcd.print("NA");
    }
    lcd.setCursor(8, 0);
    lcd.print("3:");
    if (temp3>-100)
    {
      lcd.print(temp3,1);
      lcd.print((char)223);            
    }    
    else
    {
      lcd.print("NA");
    }     
    lcd.setCursor(8, 1);
    lcd.print("Sensors");
    delay(1000);
}
