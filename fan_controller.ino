
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h> // includes the LiquidCrystal Library 
 
// Data wire is conntec to the Arduino digital pin 4
#define ONE_WIRE_BUS 8

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
float set_temp = 23.0;
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

int numberOfDevices; // Number of temperature devices found
DeviceAddress tempDeviceAddress;

LiquidCrystal lcd(2, 5, 6, 11, 4, 7); // Creates an LCD object. Parameters: (rs, enable, d4, d5, d6, d7)

//configure Timer 1 (pins 9,10) to output 25kHz PWM
// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
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
void setPWM1A(float f){
    f=f<0?0:f>1?1:f;
    OCR1A = (uint16_t)(320*f);
}
//equivalent of analogWrite on pin 10
void setPWM1B(float f){
    f=f<0?0:f>1?1:f;
    OCR1B = (uint16_t)(320*f);
}
//equivalent of analogWrite on pin 3
void setPWM2(float f){
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
  numberOfDevices = sensors.getDeviceCount();
  
  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");

  // Loop through each device, print out address
  for(int i=0;i<numberOfDevices; i++) {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)) {
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      printAddress(tempDeviceAddress);
      Serial.println();
		} else {
		  Serial.print("Found ghost device at ");
		  Serial.print(i, DEC);
		  Serial.print(" but could not detect address. Check power and cabling");
		}
  }
     
    pinMode(9,OUTPUT); //1A
    pinMode(10,OUTPUT); //1B
    setupTimer1();
    //enable outputs for Timer 2
    pinMode(3,OUTPUT); //2
    setupTimer2();
    //note that pin 11 will be unavailable for output in this mode!
    //example...
    setPWM1A(0.0f); //set duty to 10% on pin 9
    setPWM1B(0.0f); //set duty to 10% on pin 10
    setPWM2(0.0f); //set duty to 10% on pin 3
}
void loop(){
    sensors.requestTemperatures(); 
  
    Serial.print("Sensor 1 Temp: ");
    // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
    float temp1 = sensors.getTempCByIndex(0);
    if (temp1 > set_temp)
    {
      setPWM1A(0.8f);
    } else
    { setPWM1A(0.0f);
    }   
    Serial.print(temp1);
    Serial.print(" C | ");

    Serial.print("Sensor 2 Temp: ");

    float temp2 = sensors.getTempCByIndex(1); 
    
    if (temp2 > set_temp)
    {
      setPWM1B(0.8f);
    } else
    { setPWM1B(0.0f);
    }   
    Serial.print(temp2);
    Serial.print(" C | ");
    // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire 
    Serial.print("Sensor 3 Temp: ");

    float temp3 = sensors.getTempCByIndex(2);
    Serial.print(temp3);
    Serial.print(" C ");

  //  for(int i=0;i<numberOfDevices; i++) {
  //  Search the wire for address
  //  Serial.print("Sensor 1 Temp: ");
  //  temp = sensors.getTempCByIndex(2);
  //}
    Serial.print("\n"); 
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("1:");
    lcd.print(temp1,1);
    lcd.print((char)223);    
    lcd.setCursor(0, 1);
    lcd.print("2:");
    lcd.print(temp2,1);
    lcd.print((char)223);
    lcd.setCursor(8, 0);
    lcd.print("3:");
    lcd.print(temp3,1);
    lcd.print((char)223);     
    lcd.setCursor(8, 1);
    lcd.print("Sensors");
    delay(1000);
}
