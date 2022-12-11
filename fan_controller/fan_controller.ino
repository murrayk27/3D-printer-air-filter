
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h> // includes the LiquidCrystal Library 
 
// Data wire is conntec to the Arduino digital pin 4
#define ONE_WIRE_BUS 8

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
float set_temp = 23.0; // if temp lower than set the fan will idle at lowest speed
float max_temp = 30.0; // if temp higher than max the fan run at full blast
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

int numberOfDevices; // Number of temperature devices found
DeviceAddress tempDeviceAddress;
DeviceAddress Thermometer;

uint8_t sens1_ID[8] = {0x28, 0x13, 0xE3, 0x16, 0xA8, 0x01, 0x3C, 0xB3};
uint8_t sens2_ID[8] = {0x28, 0x90, 0xE7, 0x16, 0xA8, 0x01, 0x3C, 0x1F};
uint8_t sens3_ID[8] = {0x28, 0xBF, 0xC1, 0x16, 0xA8, 0x01, 0x3C, 0xE9};

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
    lcd.print("Init Arduino");

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
    printAddress(Thermometer);
  }
  // Loop through each device, print out address
//  for(int i=0;i<numberOfDevices; i++) {
//    // Search the wire for address
//    if(sensors.getAddress(tempDeviceAddress, i)) {
//      Serial.print("Found device ");
//      Serial.print(i, DEC);
//      Serial.print(" with address: ");
//      printAddress(tempDeviceAddress);
//      
 //     Serial.println();
//		} else {
//		  Serial.print("Found ghost device at ");
//		  Serial.print(i, DEC);
//		  Serial.print(" but could not detect address. Check power and cabling");
	//	}
  //}
     
    pinMode(9,OUTPUT); //1A
    pinMode(10,OUTPUT); //1B
    setupTimer1();
    //enable outputs for Timer 2
    pinMode(3,OUTPUT); //2
    setupTimer2();
    //note that pin 11 will be unavailable for output in this mode!
    //example...
    setPWMF1(0.0f); //set duty to 10% on pin 9 labeled 'fan 1'
    setPWMF3(0.0f); //set duty to 10% on pin 10 'fan 3'
    setPWMF2(0.0f); //set duty to 10% on pin 3 'fan 2'
}
void loop(){
    sensors.requestTemperatures(); 
  
    Serial.print("Sensor 1 Temp: ");

    float temp1 = sensors.getTempC(sens1_ID);
    if (temp1 > max_temp)
    { setPWMF1(1.0f); // set fan to full power if temp above max
    }
    else if (temp1 > set_temp)
    {
      float power_frac = 0.5*(temp1-set_temp)/(max_temp-set_temp)+0.5;       
      setPWMF1(power_frac); // proportionally ramp up fan speed
    }       
    else
    { setPWMF1(0.0f);
    }   
    
    Serial.print(temp1);
    Serial.print(" C | ");

    Serial.print("Sensor 2 Temp: ");

    float temp2 = sensors.getTempC(sens2_ID); 
    if (temp2 > max_temp)
    { setPWMF2(1.0f);      
    }
    else if (temp2 > set_temp)
    {
      float power_frac = 0.5*(temp2-set_temp)/(max_temp-set_temp)+0.5;       
      setPWMF2(power_frac);
    } else
    { setPWMF2(0.0f);
    }   
    Serial.print(temp2);
    Serial.print(" C | ");
 
    Serial.print("Sensor 3 Temp: ");

    float temp3 = sensors.getTempC(sens3_ID); 
    if (temp3 > max_temp)
    { setPWMF3(1.0f);      
    }
    else if (temp3 > set_temp)
    {
      float power_frac = 0.5*(temp3-set_temp)/(max_temp-set_temp)+0.5;       
      setPWMF3(power_frac);
    } else
    { setPWMF3(0.0f);
    }    
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
