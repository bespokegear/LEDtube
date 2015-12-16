/********************************************************
/****** LED Tube SLAVE Unit *************************
/****** by Matt Little **********************************
/****** Date: 8/12/15 **********************************
/****** matt@re-innovation.co.uk ************************
/****** www.re-innovation.co.uk *************************
/********************************************************

  See www.re-innovation.co.uk for information and 
  construction details
 

/*************Details of Code*****************************

  Device ID
  Each device has an ID number which can be changed. This means more than one pedalog device can be used on a system. 
  “aXXCHIDXXE--“
  Where the last two values (XX) are the new device ID (from AA to ZZ).
  

 
  Updates: 
  8/12/15 Code started - Matt Little

**********************************************************************************************************/
/************ External Libraries*****************************/

#include <stdlib.h>
#include <avr/pgmspace.h>  // Library for putting data into program memory
#include <EEPROM.h>        // For writing values to the EEPROM
#include <avr/eeprom.h>    // For writing values to EEPROM
#include <Adafruit_NeoPixel.h>  // For writing to the LED string
#include <avr/power.h>  // Not sure if this is needed.

/************User variables and hardware allocation**********************************************/

// **********Arduino Hardware information********************************************
#define MAXLEDS 360 // The total number of LEDs on the strip (12 x 30)
#define PIN 6   // The digital pin for data to LEDs

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(MAXLEDS, PIN);


// These constants won't change.  They're used to give names to the pins used:
const int piezo1 = A0; 
const int piezo2 = A1; 
const int piezo3 = A2;  
const int piezo4 = A3;
const int piezo5 = A4;  
const int piezo6 = A5;  

const int LEDperPixel = 2; // (50 max) This is the number of LEDs that make up a pixel unit
const int pixelsPerSlave = 4;  // Number of pixels from one slave unit

const int decayTime = 1500; // Time in mS for LEDs to fade in/out with piezo hit
const int decayRampUp = 1;  // Parameter to control grandient of ramp up
const int decayRampDown = 1;  // Parameter to control grandient of ramp down

const int piezoThreshold = 25; // Analog value to say the piezo has been 'hit'

//*********END of hardware information****************************************

char deviceID[3]; // A buffer to hold the device ID

String outputString;    // This is the holder for the data as a string. Start as blank

// ****** Serial Data Read***********
// Variables for the serial data read
char inByte;         // incoming serial char
String str_buffer = "";  // This is the holder for the string which we will display
#define MAX_STRING 50      // Sets the maximum length of string probably could be lower
char stringBuffer[MAX_STRING];  // A buffer to hold the string when pulled from program memory

// Varibales for writing to EEPROM
int hiByte;      // These are used to store longer variables into EERPRPROM
int loByte;

long int number;
int n = 0;
int counter = 0;  // For slowing the serial read

boolean updateLEDflag = false;

// Data for Piezo reads
char piezoUpdated;  // This holds a 8 bit number. Each bit corresponds to a different piezo being hit
  
//****************INITIALISE ROUTINE******************************
void setup()
{
  pixels.begin();
  pixels.show(); // Initialize all pixels to 'off'
  
  Serial.begin(115200);    // Set up a serial output for data display and changing parameters
  
  // Read in the values from EEPROM
  // Read the device ID
  deviceID[0] = char(EEPROM.read(0));
  deviceID[1] = char(EEPROM.read(1)); 
  Serial.print("Device ID is: ");
  Serial.print(deviceID[0]);
  Serial.println(deviceID[1]);
}

void loop()
{
//  // ********** READ THE PIEZO DATA ************************//
  piezoUpdated = 0; // Reset if the Piezo has been updated
  if(analogRead(piezo1)>=piezoThreshold)
  {
    piezoUpdated = piezoUpdated+1;
    Serial.println("Piezo 0");
  }
  if(analogRead(piezo2)>=piezoThreshold)
  {
    piezoUpdated = piezoUpdated+2;
    Serial.println("Piezo 1");
  } 
  if(analogRead(piezo3)>=piezoThreshold)
  {
    piezoUpdated = piezoUpdated+4;
  }
  if(analogRead(piezo4)>=piezoThreshold)
  {
    piezoUpdated = piezoUpdated+8;
  }
  if(analogRead(piezo5)>=piezoThreshold)
  {
    piezoUpdated = piezoUpdated+16;
  }
  if(analogRead(piezo6)>=piezoThreshold)
  {
    piezoUpdated = piezoUpdated+32;
  }

//  if(piezoUpdated!=0)
//  {
//    Serial.println(piezoUpdated);
//  }
  
  // ********* READ ANY SERIAL DATA ************************//
  getData();  // Check the serial port for data    

  // ********** SET THE LEDS DEPENDING ON DATA ***************// 
  // Data will be written in pixels, so each lot of LEDperPixel written same data

  if (updateLEDflag == true)
  {
    int y=0;
    for (int j = 0; j <pixelsPerSlave; j++)
    {
      // Go through each LED
      // Broken into PIXELS here:
      for (int z = 0; z<LEDperPixel; z++)
      {
        // Colours are RED GREEN BLUE
        // so OFF is 0,0,0
        pixels.setPixelColor(y, (int)str_buffer[(j*3)+3], (int)str_buffer[(j*3)+4], (int)str_buffer[(j*3)+5]);
        y++;
      }  
      
    }
    pixels.show();

    // Reset everything
    str_buffer="";  // Reset the buffer to be filled again  
    updateLEDflag = false;    
  }
}


//// This routine pulls the string stored in program memory so we can use it
//// It is temporaily stored in the stringBuffer
//char* getString(const char* str) {
//	strcpy_P(stringBuffer, (char*)str);
//	return stringBuffer;
//}

// **********************GET DATA SUBROUTINE*****************************************
// This sub-routine picks up and serial string sent to the device and sorts out a power string if there is one
// All values are global, hence nothing is sent/returned

void getData()
{ 
  // **********GET DATA*******************************************
  // We want to find the bit of interesting data in the serial data stream
  // As mentioned above, we are using LLAP for the data.
  // All the data arrives as serial commands via the serial interface.
  // All data is in format aXXDDDDDDDDD where XX is the device ID
  if (Serial.available() >= ((pixelsPerSlave*3)+3)) 
  {
    inByte = Serial.read(); // Read whatever is happening on the serial port
    if(inByte=='a')    // If we see an 'a' then read in the next 39 chars into a buffer.
    {   
      str_buffer+=inByte;
      for(int i = 0; i<((pixelsPerSlave*3)+2);i++)  // Read in the next 11 chars - this is the data
      {
        inByte = Serial.read(); 
        str_buffer+=inByte;
      }
      Serial.println(str_buffer);  // TEST - print the str_buffer data (if it has arrived)
      sortData();  
    }
  }
}

// **********************SORT DATA SUBROUTINE*****************************************
// This sub-routine takes the read-in data string (12 char, starting with a) and does what is required with it
// The str-buffer is global so we do not need to send it to the routine

void sortData()
{ 
  // We first want to check if the device ID matches.
  // If it does not then we disregard the command (as it was not meant for this device      
  if(str_buffer.substring(1,3) == deviceID)
  {
    // If yes then we can do further checks on ths data
    // This is where we do all of the checks on the incomming serial command:
    
    // Change device ID:
    // Device ID
    // “aXXCHIDXXE--“
    // Where the last two values (XX) are the new device ID (from AA to ZZ).
    if(str_buffer.substring(3,7) == "CHID")
    {
      // First check if the NEW device ID is within the allowable range (AA-ZZ)
      // to do this we can convert to an int and check if it is within the OK levels
      // A -> int is 65, Z -. int is 90
      // So our new device ID as an int must be between 65 and 90 for it to be valid
      if(65<=int(str_buffer[7])&&int(str_buffer[7])<=90&&65<=int(str_buffer[8])&&int(str_buffer[8])<=90)   
      { // If is all OK then write the data
        // Change device ID
        Serial.print("Change Device ID to ");
        Serial.println(str_buffer.substring(7,9));  // This will change the device ID
        deviceID[0] = str_buffer[7];
        deviceID[1] = str_buffer[8];
        // Also want to store this into EEPROM
        EEPROM.write(0, deviceID[0]);    // Do this seperately
        EEPROM.write(1, deviceID[1]);   
      }
      else
      {
        // Invalid ID used!!!
        Serial.println("Invalid ID!!");
      } 
      str_buffer="";  // Reset the buffer to be filled again      
    } 
    else
    {
      updateLEDflag = true;
      Serial.println("Valid LED Update");  // TEST - got into this routine
    }    
  }
  else
  {
    // We enter here if the device ID does not match.
    // Want to just stay quiet
    // Serial.println("Device ID does not match");
    str_buffer="";  // Reset the buffer to be filled again 
  }
}

