/*
 * This code powers an Arduino Pro Micro based Longterm Sensor.  It is designed to be powered primarily by a Benthic Microbial Fuel Cell.
 * As such, low power consumption was paramount.  Due to the remote deployment of these devices, data logging in a standard format was also
 * an important decision.  The Arduino is currently programmed to produce a CSV file which will allow easy integration Excel and other spreadsheet programs.
 * This file is saved to a Micro-SD card.  In addition to the Pro Micro, a DS3234 RTC and Transflash/MicroSD card breakout were used.
 * Pieces of code where used from Sparkfun for their libraries and Lim Phang Moh on the Rocket Scream Forum for the USB detachment and reattachment code
 * 
 */

//Libraries
#include <LowPower.h>//Low Power Library; Be careful on USB based Arduino to properly activate USB
#include <SPI.h>//Library for SPI, allows communication between components
#include <SparkFunDS3234RTC.h>//Library for Real Time Clock
#include <SD.h>//Library for SD card

//Definitions
#define PRINT_USA_DATE//Configures RTC to follow US "middle-endian" date format
#define DS13074_CS_PIN 10//Configures RTC to be Slave Selected by pin 10
#define SD_CARD_CS_PIN 4//Configures SD card slot to be Slave Selected by pin 4

// Declaration of Variables
int voltCathode = A0;//Sets to for reading the Fuel Cell Voltage to A0
int presVoltage = 0;//Holds ADC measuremnt value
int calculatedVolt;//Holds calculated Voltage Value
String currentFile="data"+String(rtc.hour())+"_"+String(rtc.day())+"_"+String(rtc.month())+"_"+String(rtc.year())+".csv";//Generates file name for use during this sampling time


void setup()
{
  //RTC Configuration    
    rtc.begin(DS13074_CS_PIN);//Begins RTC
    //rtc.autoTime();//Uncomment to have RTC pull data from Compilation time
    
  //Serial Configuration
    Serial.begin(9600);//Begins Serial Communication at 9600 Baud
    
  //SD configuration
    SD.begin(SD_CARD_CS_PIN);//Starts SD card
    File dataFile = SD.open(currentFile);//Creates or Opens File for Datalogging
    dataFile.println("Timestamp, Voltage(mV)");//Writes simple CSV header
    dataFile.close();//Closes SD card file
}

void loop() 
{
  //Sleep Mode Configuration
   //Configures USB port to shutdown properly
    USBCON |= _BV(FRZCLK);// Disable USB clock 
    PLLCSR &= ~_BV(PLLE);// Disable USB PLL
    USBCON &= ~_BV(USBE);// Disable USB
   //LowPower Library
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);//Set the Arduino to sleep with ADC off and Black Out Detection off to maximise power efficiency
    //Should be changed to be interrupt driven off of RTC, but currently just sleeps for 8 seconds 
   //Restore USB functionality to Arduino 
    USBDevice.attach(); //Reattaches USB
    
  //Notification LED
    TXLED1;//Primarily used for notifying user that the Arduino is active, will be disabled in deployment to save power
    delay(10000);//Used to ensure PC can connect without needing to enter bootloading mode, comment out for deployment
  
  //Data Collection
    presVoltage=analogRead(voltCathode);//Returns the ADC value from Analog Input
    calculatedVolt=presVoltage*(3300/1024)*1.07527;//Calculates the actual Voltage, using the max voltage divided by number of steps and an calibration offset
    //While there is likely a better method to calibrate the voltmeter than a hard coded multiplier, since the primary purpose is to track trends, precise voltage measurements are secondary to ease of use and power efficiency
    rtc.update();//Pulls time data from RTC
  
  //Data Logging
    File dataFile=SD.open(currentFile);//Opens file created in Setup
    printTime(dataFile);//Prints Time from RTC
    dataFile.println(calculatedVolt);//Prints Voltage from fuel cells
    dataFile.close();//Closes file
    
  //Debugging/Troubleshooting
    //Following Code is used to verify operation of RTC and Voltmeter when connected serially, not used in operation
    Serial.println(String(rtc.hour())+":"+String(rtc.minute())+":"+String(rtc.second()));
    Serial.print("Here is your voltage(mV):");
    Serial.println(calculatedVolt);
    delay(10);
  //Notification LED    
    TXLED0;
}

void printTime(File currentFile)
{
  
 #ifdef PRINT_USA_DATE
  currentFile.print(String(rtc.month()) + "/" +   // Print month
                 String(rtc.date()) + "/");  // Print date
 #else
  currentFile.print(String(rtc.date()) + "/" +    // (or) print date
                 String(rtc.month()) + "/"); // Print month
 #endif
  currentFile.print(String(rtc.year())+" ");        // Print year
  currentFile.print(String(rtc.hour()) + ":"); // Print hour
  if (rtc.minute() < 10)
    currentFile.print('0'); // Print leading '0' for minute
  currentFile.print(String(rtc.minute()) + ":"); // Print minute
  if (rtc.second() < 10)
    currentFile.print('0'); // Print leading '0' for second
  currentFile.print(String(rtc.second())+ ','); // Print second

  if (rtc.is12Hour()) // If we're in 12-hour mode
  {
    // Use rtc.pm() to read the AM/PM state of the hour
    if (rtc.pm()) currentFile.print(" PM"); // Returns true if PM
    else currentFile.print(" AM");
  }
  // Few options for printing the day, pick one:
  //SD.print(rtc.dayStr()); // Print day string
  //SD.print(rtc.dayC()); // Print day character
  //SD.print(rtc.day()); // Print day integer (1-7, Sun-Sat)
}
