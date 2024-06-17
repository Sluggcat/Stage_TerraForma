/*
 * Code adapted for the Terra Forma demo:
 * - switch from Adafruit Feather M0 BLE to Adafruit Feather M0 LoRa
 * - 
 * 
 * 01-10-2021: Add an option to disable Atlas probes.
 * 
 * 28-09-2021: Change the AS7341 reading method from blocking to non-blocking calls. Based on "reading_while_looping"
 * Adafruit library's example.
 * 
 * Remarks regarding acquisition frequency : Atlas probes+EZO have a typical processing time of one second,
 * hence the acquisition frequency must be > to 1000 ms.
 * 
 * TODO 20-10-2020: Add conversion function for conductivity based on the calculations in get_conductivity()
 * TODO 20-10-2020: Turn off LED lights for Atlas probes.
 * TODO 20-10-2020: Use OFF pins from Atlas probes for energy saving.
 * 
 * 
 * 
 * BLUE ROBOTICS wiring
 * Red +3.3V
 * Green SCL
 * White SDA
 * Black GND
This sketch allows you to collect conductivity, temperature, and depth data and stream it to your phone via the Adafruit Bluefruit phone application.
The application is available for both Android and iOS devices.
This sketch does not consider efficiency and will be updated as new functions and commands are implemented.
Some of this code is repurposed from sketches created by Adafruit, Atlas Scientific, and Blue Robotics. 
If building your own sensor, please support them by purchasing parts from their online stores.

For questions or comments regarding this sketch or the Arduino-based CTD project, send an email to Ian Black (blackia@oregonstate.edu).

Don't forget to check out these other open source CTD variants!
Arduino-based Sonde  https://github.com/glockridge/MooredApplication
OpenCTD https://github.com/OceanographyforEveryone/OpenCTD
PiCTD https://github.com/haanhouse/pictd
Conduino https://github.com/kpdangelo/OpenCTDwithConduino
*/

/*----------Stuff you might want to change.---------*/
#define DEBUG_SERIALPRINT   1

#define USE_LORA            0     // If we use Adafruit Feather M0 LoRa
#define USE_BLE             1     // If we use Adafruit Feather M0 Bluefruit
#define USE_BLYNK           0     // Only valid with the Bluefruit.

#define LOW_POWER_MODE      0     // TODO
#define USE_ATLAS           1
#define USE_OLED            1
            

/*----------Initialization---------*/
#if USE_LORA
  #include <RH_RF95.h>

  /* for feather m0 */
  #define RFM95_CS 8
  #define RFM95_RST 4
  #define RFM95_INT 3
  // Change to 433.0 or other frequency, must match RX's freq!
  #define RF95_FREQ 869.5
  
  // Singleton instance of the radio driver
  RH_RF95 rf95(RFM95_CS, RFM95_INT);
#endif

#if USE_BLE
  #include <Adafruit_BLE.h> 
  #include <Adafruit_BluefruitLE_SPI.h>
  #include "Adafruit_BluefruitLE_UART.h"
#endif
  
#include "RTClib.h" 
#include <string.h>
#include <SD.h>  
#include <Wire.h>

#if USE_ATLAS
  #include <Ezo_i2c.h> //include the EZO I2C library from https://github.com/Atlas-Scientific/Ezo_I2c_lib
#endif

#include <Adafruit_AS7341.h>
#include "TSYS01.h" 
#include "MS5837.h"
#include <avr/dtostrf.h> //For temperature value concatenation.
#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

#if USE_OLED
  #include <Wire.h>
  #include <Adafruit_GFX.h>

  // Declare the OLED display  
  #include <Adafruit_SH110X.h>
  Adafruit_SH1107 oled = Adafruit_SH1107(64, 128, &Wire);

  //#include <Adafruit_SSD1306.h>  
  //Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
  
  
  // OLED FeatherWing buttons map to different pins depending on board:
  #if defined(ESP8266)
    #define BUTTON_A  0
    #define BUTTON_B 16
    #define BUTTON_C  2
  #elif defined(ESP32)
    #define BUTTON_A 15
    #define BUTTON_B 32
    #define BUTTON_C 14
  #elif defined(ARDUINO_STM32_FEATHER)
    #define BUTTON_A PA15
    #define BUTTON_B PC7
    #define BUTTON_C PC5
  #elif defined(TEENSYDUINO)
    #define BUTTON_A  4
    #define BUTTON_B  3
    #define BUTTON_C  8
  #elif defined(ARDUINO_NRF52832_FEATHER)
    #define BUTTON_A 31
    #define BUTTON_B 30
    #define BUTTON_C 27
  #else // 32u4, M0, M4, nrf52840 and 328p
    #define BUTTON_A  9
    #define BUTTON_B  6
    #define BUTTON_C  5
  #endif
#endif


#define latitude 45.00    //This is your deployment latitude. It is used in the pressure to depth conversion.

Ezo_board EC  = Ezo_board(100, "EC");     //create an EC circuit object who's address is 100 and name is "EC"
Ezo_board PH  = Ezo_board(99, "PH");      //create a PH circuit object, who's address is 99 and name is "PH"
Ezo_board ORP = Ezo_board(98, "ORP");     //create an ORP circuit object who's address is 98 and name is "ORP"
Ezo_board DO  = Ezo_board(97, "DO");      //create an DO circuit object who's address is 97 and name is "DO"

bool reading_request_phase = true;        //selects our phase
uint32_t next_poll_time = 0;              //holds the next time we receive a response, in milliseconds
const unsigned int response_delay = 2000; //how long we wait to receive a response, in milliseconds

String BROADCAST_NAME = "Econect Mk1";  //You can name your CTD anything!
/*---------------------------------------*/

#if USE_BLE
  #define BLUEFRUIT_SPI_CS  8
  #define BLUEFRUIT_SPI_IRQ 7
  #define BLUEFRUIT_SPI_RST 4 
  Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
  uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
#endif
  
        float parsefloat(uint8_t *buffer);
        void printHex(const uint8_t * data, const uint32_t numBytes);   //Pas Utilisé dans le reste du programme, utile ??
        extern uint8_t packetbuffer[];


char buf[60];
File datafile; File recentfile;
RTC_PCF8523 rtc; 
TSYS01 tsensor;
MS5837 psensor;

float AirTemp, Celsius, Fahrenheit, Kelvin;                             //A voir si on a besoin de toute les unités ?
float AtmP, x, gr, AbsPressure, Decibars, Meters, Feet, Fathoms;        //Pareil, besoin de toute les unités ?

        char ec_data[48];                                               //Pas utilisé, utile ??
        byte in_char = 0, i = 0 ;                                       //Pas utilisé, utile ??

char *ec, *tds, *sal, *sg;                      
float ec_float, tds_float, sal_float, sg_float;                 
String BROADCAST_CMD = String("AT+GAPDEVNAME=" + BROADCAST_NAME);

/*----------Salinty calculation parameters---------*/
float SalA1=2.070e-5, SalA2=-6.370e-10, SalA3=3.989e-15;
float SalB1=3.426e-2, SalB2=4.464e-1, SalB3=4.215e-1, SalB4=-3.107e-3;
float Salc0=6.766097e-1, Salc1=2.00564e-2, Salc2=1.104259e-4, Salc3=-6.9698e-7, Salc4=1.0031e-9;
float Sala0= 0.0080, Sala1= -0.1692, Sala2= 25.3851, Sala3= 14.0941, Sala4= -7.0261, Sala5= 2.7081;
float Salb0= 0.0005, Salb1= -0.0056, Salb2= -0.0066, Salb3 =-0.0375, Salb4= 0.0636, Salb5= -0.0144;
float Salk =0.0162, SalCStandard=42.914;
float R, RpNumerator, RpDenominator, Rp, rT, RT, Salinity;
/*---------------------------------------*/

float vbatt;

/*----------AS7341 related variables---------*/
Adafruit_AS7341 as7341;

// ATIME and ASTEP are registers that sets the integration time of the AS7341 according to the following:
// t_int = (ATIME + 1) x (ASTEP + 1) x 2.78 µs
uint16_t myATIME = 29; //599; 
uint16_t myASTEP = 599; //20;

float integrationTime = 0;
uint8_t nbSamples = 0;

as7341_gain_t myGAIN = AS7341_GAIN_32X;

uint16_t RAW_color_readings[12];  // Contains the last RAW readings for the 12 channels of the AS7341
float average_color_readings[12]; // Average calculation to improve the spectrum quality.
uint8_t colorList[10] = {0, 1, 2, 3, 6, 7, 8, 9, 10, 11}; // Monotonous indexing for channels, makes it easier to loop !

        float Offset_corrected_readings[12]; // Offset compensation based on Basic counts           //Pas utilisé dans le reste du programme
        float Basic_count_offset[12] = {0, 0,0,0,0,0,0,0,0,0,0,0}; // Offset values for each channel
/*---------------------------------------*/

/*----------BLYNK---------*/
#if USE_BLE
  #if USE_BLYNK
    #define BLYNK_USE_DIRECT_CONNECT
    #define BLYNK_PRINT Serial
    #include <BlynkSimpleSerialBLE.h>
    // You should get Auth Token in the Blynk App.
    // Go to the Project Settings (nut icon).
    char auth[] = "EfKrQaamWBdiUzC7vkBl8ZCwT8sQ5NqM";
  #endif
#endif
/*------------------------*/


/*----------Functions---------*/
/*
 * @brief Calculate atmospheric pressure when the prove is above water (as it should be in normal use-case).
 * 
 * @param
 * 
 * @return
 */
float PressureZero(){  //Determines atmospheric pressure if the user turns the unit on above water.
  psensor.read();
  AtmP = psensor.pressure();
  return AtmP;
}


/*
 * @brief Reads air temperature when the probe is above water (as it should be in normal use-case).
 * 
 * @param
 * 
 * @return Temeprature value in float format.
 */
float AirTemperature(){  //Determines air temperature if the user turns the unit on above water.
  tsensor.read();
  AirTemp = tsensor.temperature();
  return AirTemp;
}


/*
 * @brief Prints a header line to the CSV file for variables identification.
 * 
 * @param
 * 
 * @return
 */
void PrintHeaders(){ //Prints a header line to the CSV for column identification.
   if(datafile){
   //Voir pour faire un seul print, avantageux ou non ?! 
    datafile.print("Date");  
    datafile.print(",");
    datafile.print("PST");  
    datafile.print(",");
    datafile.print("abspres"); 
    datafile.print(",");   
    datafile.print("dbars"); 
    datafile.print(",");
    datafile.print("meters");
    datafile.print(",");
    datafile.print("degC"); 
    datafile.print(",");

  #if USE_ATLAS  
  //Pareil  
    datafile.print("ec");
    datafile.print(",");
    datafile.print("EZOsal");
    datafile.print(",");
    datafile.print("PSS-78");
    datafile.print(",");
    datafile.print("EZO_EC");
    datafile.print(",");
    datafile.print("EZO_pH");
    datafile.print(",");
    datafile.print("EZO_ORP");
    datafile.print(",");
    datafile.print("EZO_DO");
    datafile.print(",");
  #endif
  //Pareil
    datafile.print("F1_415nm");
    datafile.print(",");
    datafile.print("F2_445nm");
    datafile.print(",");
    datafile.print("F3_480nm");
    datafile.print(",");
    datafile.print("F4_515nm");
    datafile.print(",");
    datafile.print("F5_555nm");
    datafile.print(",");
    datafile.print("F6_590nm");
    datafile.print(",");
    datafile.print("F7_630nm");
    datafile.print(",");
    datafile.print("F8_680nm");
    datafile.print(",");
    datafile.print("F9_Clear");
    datafile.print(",");
    datafile.print("F10_NIR");
    datafile.print(",");    
    datafile.print("NB samples");
    datafile.print(",");    
    
    datafile.println("vbatt");  
    datafile.flush();
  }
}


/*
 * @brief Setup all the system
 * 
 * @param
 * 
 * @return
 */
void setup(){  
  #if DEBUG_SERIALPRINT
  Serial.begin(115200);
  delay(100);
  #endif
  
  Wire.begin();
  delay(100);
  rtc.begin();
  delay(100);
  pinMode(10, OUTPUT);
  
  //Uncomment this line to set time to computer time. Recomment and reupload to maintain RTC time.
  rtc.adjust(DateTime(__DATE__, __TIME__));  
  DateTime now = rtc.now();

  #if USE_OLED
    oled.begin(0x3C, true);
    //oled.setBatteryVisible(true);
    oled.display();
    delay(1000);

    // Clear the buffer.
    oled.clearDisplay();
    oled.display();
    oled.setRotation(1);

    // text display tests
    oled.setTextSize(1);
    oled.setTextColor(SH110X_WHITE);
    oled.setCursor(0,0);

    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);
    pinMode(BUTTON_C, INPUT_PULLUP);  
  #endif

//Module LoRa à concerver ??
  #if USE_LORA
    // ----------- Init the LoRa radio module ----------- //
    // Manual reset of the LoRa radio
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);
  
    while (!rf95.init()) {
      #if DEBUG_SERIALPRINT
      Serial.println("LoRa radio init failed");
      #endif

      #if USE_OLED
      oled.clearDisplay();
      oled.setCursor(0,0);
      oled.println("LoRa radio init failed");
      oled.display();
      #endif      
  
      while (1);
    }
  
    #if DEBUG_SERIALPRINT
    Serial.println("LoRa radio init OK!");
    #endif

    #if USE_OLED
    oled.clearDisplay();
    oled.setCursor(0,0);
    oled.println("LoRa radio init OK!");
    oled.display();
    #endif      
          
    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (!rf95.setFrequency(RF95_FREQ)) {
      #if DEBUG_SERIALPRINT
      Serial.println("setFrequency failed");
      #endif

      #if USE_OLED
      oled.clearDisplay();
      oled.setCursor(0,0);
      oled.println("setFrequency failed");
      oled.display();
      #endif      
        
      while (1);
    }
    #if DEBUG_SERIALPRINT
    Serial.print("Set Freq to: ");
    Serial.println(RF95_FREQ);
    #endif

    #if USE_OLED
    oled.clearDisplay();
    oled.setCursor(0,0);
    oled.print("Set Freq to: ");
    oled.println(RF95_FREQ);    
    oled.display();
    #endif      
    
    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  
    // The default transmitter power is 13dBm, using PA_BOOST.
    // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
    // you can set transmitter powers from 5 to 23 dBm:
    //rf95.setTxPower(23, false);
    // ----------- End of: Init the LoRa radio module ----------- //
  #endif  
  
  delay(250);
  if (SD.begin(10)) { //Create a file with the current month, day, hour, and minute as the file name.
    char filename[] = "00000000.CSV";   
      filename[0] = now.month()/10 + '0'; 
      filename[1] = now.month()%10 + '0'; 
      filename[2] = now.day()/10 + '0'; 
      filename[3] = now.day()%10 + '0'; 
      filename[4] = now.hour()/10 + '0'; 
      filename[5] = now.hour()%10 + '0'; 
      filename[6] = now.minute()/10 + '0'; 
      filename[7] = now.minute()%10 + '0';
    //Pourquoi, fichier tampon ??
    SD.remove("PLOT.CSV"); //This is a temporary file that is created to send data to the mobile device.
    delay (250);       
    if(!SD.exists(filename)){ 
      SdFile::dateTimeCallback(SDCardDateTimeCallback);         // Set date time callback function: this is required so the creation date of the file is correct.
      datafile=SD.open(filename,FILE_WRITE); //Create a new file if the file doesn't already exist.
      recentfile=SD.open("PLOT.CSV",FILE_WRITE); 
    }
  }

  PrintHeaders(); //Print the header line(s) to the file.

  tsensor.init();   //Initialize the temp sensor.
  delay(250);     
  psensor.init();   //Initialize the pressure sensor.
  delay(250);
  psensor.setModel(MS5837::MS5837_30BA);    //Define the pressure sensor model.
  psensor.setFluidDensity(1028);            //Set the approximate fluid density of deployment. Global ocean average is 1035. Coastal/estuarine waters between 1010-1035 kg/m^3.
  PressureZero();
  AirTemperature();

  // Initialize the color sensor
  if (as7341.begin()){
    as7341.setATIME(myATIME);
    as7341.setASTEP(myASTEP);
    integrationTime = (myATIME + 1) * (myASTEP + 1) * 2.78 / 1000;    
    as7341.setGain(myGAIN);     
    as7341.enableLED(false);
    as7341.startReading();    // For non-blocking readings
  }
  else{
    #if DEBUG_SERIALPRINT
      Serial.println("Could not find AS7341");
    #endif    

    #if USE_OLED
    oled.clearDisplay();
    oled.setCursor(0,0);
    oled.println("Could not find AS7341");
    oled.display();
    #endif      
  } 
  
  #if USE_BLE
    ble.begin(); //Set up bluetooth connectivity.
    ble.echo(false);  //Turn off echo.
    ble.verbose(false);
    BROADCAST_CMD.toCharArray(buf, 60);
    ble.sendCommandCheckOK(buf);
    delay(500);
    ble.setMode(BLUEFRUIT_MODE_DATA); //Set to data mode.
    delay(500);

    #if USE_BLYNK
    Blynk.begin(auth, ble);
    #endif
  #endif

  #if LOW_POWER_MODE  
  // Turn off the EZO LEDs.
  PH.send_cmd("L,0");
  ORP.send_cmd("L,0");
  EC.send_cmd("L,0");
  DO.send_cmd("L,0");
  #endif
}

/*
 * @brief IConversion from pressure to depth. See AN69 by Seabird Scientific.
 * 
 * @param None
 * 
 * @return
 */
void get_pressure_depth(){  
  psensor.read();  //Read the pressure sensor.
  //A modifier pour simplifier
  AbsPressure = psensor.pressure();
  AbsPressure = AbsPressure + 21; //Offset for this particular pressure sensor. Measured against baro sensor on RV Sikuliaq. Verified by CE02SHSM baro sensor.
  Decibars = (psensor.pressure()- AtmP)/100;   
  x = sin(latitude / 57.29578);
  x = x * x;
  gr = 9.780318 * (1.0 + (5.2788e-3 + 2.36e-5 * x) * x) + 1.092e-6 * Decibars;
  Meters = ((((-1.82e-15 * Decibars + 2.279e-10) * Decibars - 2.2512e-5) * Decibars + 9.72659) * Decibars)/gr; //Depth in meters.
  Feet = Meters * 3.28084;
  Fathoms = Feet/6;
}

/*
 * @brief Reads the temperature, and calculate in various units. Temperature is stored in global variables.
 * 
 * @param 
 * 
 * @return
 */
void get_temperature(){ 
  tsensor.read();
  Celsius = tsensor.temperature();
  Fahrenheit = Celsius*1.8+32;     
  Kelvin = Celsius+273.15;     
}


/*
 * @brief Calculates salinity according to the Practical Salinity Scale (PSS-78 ).
 * 
 * @param
 * 
 * @return
 */
void calc_salinity(){  //PSS-78
   R = ((ec_float/1000)/SalCStandard);   
   RpNumerator = ( SalA1*Decibars)*( SalA2*pow(Decibars,2))+( SalA3*pow(Decibars,3));
   RpDenominator = 1*( SalB1*Celsius)+( SalB2*pow(Celsius,2))+( SalB3*R)+( SalB4*Celsius*R);
   Rp = 1+(RpNumerator/RpDenominator);
   rT =  Salc0 +( Salc1*Celsius)+( Salc2*pow(Celsius,2))+( Salc3*pow(Celsius,3))+( Salc4*pow(Celsius,4));
   RT=R/(rT*Rp);
   Salinity = ( Sala0+( Sala1*pow(RT,0.5))+( Sala2*RT)+( Sala3*pow(RT,1.5))+( Sala4*pow(RT,2))+( Sala5*pow(RT,2.5)))+((Celsius-15)/(1+ Salk*(Celsius-15)))*( Salb0+( Salb1*pow(RT,0.5))+( Salb2*RT)+( Salb3*pow(RT,1.5))+( Salb4*pow(RT,2))+( Salb5*pow(RT,2.5)));
}


/*
 * @brief Reads battery voltage (if there is one connected).d
 * 
 * @param
 * 
 * @return
 */
void get_voltage(){
  vbatt = analogRead(9);
  vbatt *= 2;
  vbatt *= 3.3;
  vbatt /= 1024;
}


/*
 * @brief Callback function to ensure proper file creation date timestamps.
 * This funny function allows the sd-library to set the correct file created & modified dates for all
 * sd card files (As would show up in the file explorer on your computer)
 * 
 * @param Input pointers date, time.
 * 
 * @return
 */
/*void dateTime(uint16_t* date, uint16_t* time) {
 DateTime now = rtc.now();
 // Return date using FAT_DATE macro to format fields
 *date = FAT_DATE(now.year(), now.month(), now.day());

 // Return time using FAT_TIME macro to format fields
 *time = FAT_TIME(now.hour(), now.minute(), now.second());
}*/

void SDCardDateTimeCallback(uint16_t* date, uint16_t* time) 
{
  DateTime now = rtc.now();
  *date = FAT_DATE(now.year(), now.month(), now.day());
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}


/*
 * @brief Print data in two files : datafile for the logging file, and recentfile for data plotting with the phone's app.
 * 
 * @param
 * 
 * @return
 */
void PrintToFile(){  //Function for printing data to the SD card and a serial monitor.
  DateTime now = rtc.now();  //Get the current date and time.
  if(datafile){ //If the file created earlier does in fact exist...
    datafile.print(now.month(),DEC);    //Print month to SD card.
    datafile.print("/");
    datafile.print(now.day(),DEC);   //Print date to SD card.
    datafile.print("/");
    datafile.print(now.year(),DEC); //Print year to SD card.
    datafile.print(",");   //Comma delimited.
    datafile.print(now.hour(),DEC);   //Print hour to SD card.
    datafile.print(":");
    datafile.print(now.minute(),DEC);
    datafile.print(":");
    datafile.print(now.second(),DEC); //Print date to SD card.
    datafile.print(",");
    datafile.print(AbsPressure,7);
    datafile.print(",");
    datafile.print(Decibars,7); //Options: Decibars, Meters, Feet, Fathoms
    datafile.print(",");
    datafile.print(Meters,7);
    datafile.print(",");
    datafile.print(Celsius,7);   //Options: Celsius, Fahrenheit, Kelvin
    datafile.print(",");
    
  #if USE_ATLAS    
    ec_float = EC.get_last_received_reading();
    calc_salinity();
    
    datafile.print(ec_float,7);
    datafile.print(",");
    datafile.print(sal_float,7);
    datafile.print(",");
    datafile.print(Salinity, 7);   //Options: ec_float, Salinity <- PSS-78 derived, sal_float <- EC EZO derived
    datafile.print(",");
    datafile.print(EC.get_last_received_reading(),7);
    datafile.print(",");
    datafile.print(PH.get_last_received_reading(),7);
    datafile.print(",");
    datafile.print(ORP.get_last_received_reading(),7);
    datafile.print(",");
    datafile.print(DO.get_last_received_reading(),7);
    datafile.print(",");
  #endif    
    
    // Print the Basic color readings instead of RAW data (takes gain and integration time into account).
    for(int j=0;j<10;j++){
      //datafile.print(as7341.toBasicCounts(RAW_color_readings[colorList[j]]), 7);
      datafile.print(average_color_readings[colorList[j]], 7);
      datafile.print(",");
    }

    datafile.print(nbSamples);
    datafile.print(",");    
        
    datafile.println(vbatt);
    datafile.flush();   //Close the file.
    
    #if USE_BLE
      // THIS IS WHAT IS SENT TO THE PLOTTER IN THE BLUEFRUIT APP.  
      ble.println(average_color_readings[0], 10); // Print color sensor channel BLUE    
      ble.println(average_color_readings[8], 10); // Print color sensor channel CLEAR
    #endif
  }
  
 if(recentfile){  //For the phone plotted file...
  recentfile.print(Decibars);  
  recentfile.print(",");
  recentfile.print(Celsius);   
  recentfile.print(",");
  recentfile.print(average_color_readings[colorList[8]], 7);
  recentfile.println();
  recentfile.flush();  
  }
}

/*
 * @brief Main loop
 * 
 * @param
 * 
 * @return
 */
void loop(){

  #if USE_LORA
    String outString;
    char data[220] = "";
  #endif

  if (reading_request_phase) {   // Ask for reading
    //send a read command. we use this command instead of PH.send_cmd("R"); 
    //to let the library know to parse the reading
    get_temperature();          // Get the temp and perform conversions.
    get_pressure_depth();       // Get the pressure and perform conversions.

    #if USE_ATLAS
      EC.send_read_with_temp_comp(Celsius); // Get the conductivity with temperature compensation.
      PH.send_read_cmd();
      ORP.send_read_cmd();
      DO.send_read_cmd(); 
    #endif                     

    AS7341gainControl();
    
    next_poll_time = millis() + response_delay;   //set when the response will arrive
    reading_request_phase = false;                //switch to the receiving phase
  }
  else {                               // Receiving phase
    if (millis() >= next_poll_time) {  //and its time to get the response
      #if USE_ATLAS
      float ec_val  = receive_reading(EC);            //get the reading from the EC circuit
      float ph_val  = receive_reading(PH);            //get the reading from the PH circuit
      float orp_val = receive_reading(ORP);           //get the reading from the ORP circuit
      float do_val  = receive_reading(DO);            //get the reading from the DO circuit
    
        #if DEBUG_SERIALPRINT    
        Serial.print("EC: ");Serial.println(ec_val);
        Serial.print("PH: ");Serial.println(ph_val);
        Serial.print("OR: ");Serial.println(orp_val);
        Serial.print("DO: ");Serial.println(do_val);
        #endif
      #endif
    
      #if DEBUG_SERIALPRINT
      Serial.print("T°: ");
      Serial.print(Celsius);
      Serial.println(" °C");

      Serial.print("P: ");
      Serial.print(AbsPressure);
      Serial.println(" hPa");
      #endif  

      #if USE_BLE
        #if USE_BLYNK
        Blynk.virtualWrite(V0, Celsius);
        Blynk.virtualWrite(V1, AbsPressure);
          #if USE_ATLAS
          Blynk.virtualWrite(V2,EC.get_last_received_reading());
          Blynk.virtualWrite(V3,PH.get_last_received_reading());
          Blynk.virtualWrite(V4,ORP.get_last_received_reading());
          Blynk.virtualWrite(V5,DO.get_last_received_reading()); 
          #endif
        #endif
      #endif

      PrintToFile(); //Save data to file.

      #if USE_LORA
/*        outString = "EC:" + String(ec_val) + ",";
        outString.concat("PH:"+ String(ph_val) + ",");
        outString.concat("OR:"+ String(orp_val) + ",");
        outString.concat("DO:"+ String(do_val) + "\n");        
*/
        outString = String(ec_val) + ",";
        outString.concat(String(ph_val) + ",");
        outString.concat(String(orp_val) + ",");
        outString.concat(String(do_val) + "\n"); 

        if(outString != 0){
          outString.toCharArray(data, outString.length()+1);
          data[strlen(data)] = '\0';      // End the string with a NULL to be sure ...         
          delay(100); // may or may not be needed
          rf95.send((uint8_t *)data, sizeof(data));
          rf95.waitPacketSent();
        }  
      #endif  

      #if USE_OLED
        oled.clearDisplay();
        oled.setCursor(0,0);
        
        for(int k=0;k<10;k++){
          oled.print("F");oled.print(colorList[k]+1);oled.print(" ");
          oled.println(RAW_color_readings[colorList[k]]);
        }
        oled.display();
      #endif

      nbSamples = 0;  // Zeroes the AS7341 number of samples and prepare for next acquisition cycle.
      for(int j=0;j<10;j++){
        average_color_readings[colorList[j]] = 0;
      }      
      
      //delay(100); //Wait 100 milliseconds.

      #if USE_BLE
        if (ble.available() > 0){   //If a connection is made...
          CommandMode();          //...continue to collect data and wait for several command options from user.
        }
      #endif

      reading_request_phase = true;            //switch back to asking for readings
    }
  }

  // Non-blocking reading for AS7341. Done in the main loop to increase sample numbers and do some averaging.
  bool timeOutFlag = yourTimeOutCheck();
  
  if(as7341.checkReadingProgress() || timeOutFlag){
    if(timeOutFlag){}  
  
    as7341.getAllChannels(RAW_color_readings);
    nbSamples++;

    // Here we do the averaging based on the number of samples already acquired. Average calculation is based on Basic 
    // counts instead of RAW counts because of the AGC.
    for(int j=0;j<10;j++){
      average_color_readings[colorList[j]] = (average_color_readings[colorList[j]]+as7341.toBasicCounts(RAW_color_readings[colorList[j]]))/nbSamples;
    }          
    as7341.startReading();
  }
 
  #if USE_BLE
    #if USE_BLYNK
    Blynk.run();
    #endif  
  #endif
}

/*
 * @brief Fill this in to prevent the possibility of getting stuck forever if you missed the result, or whatever.
 * 
 * @param 
 * 
 * @return FALSE if no timing issue. TRUE if timing issue.
 */
bool yourTimeOutCheck(){
  return false;
}

/*
 * @brief Taken from I2C_read_multiple_circuits.ino from Atlas Scientific Instructables.
 * 
 * @param Ezo_board type
 * 
 * @return
 */
float receive_reading(Ezo_board &Sensor) {               // function to decode the reading after the read command was issued
  float result = 0;
  /*#if DEBUG_SERIALPRINT
  Serial.print(Sensor.get_name()); Serial.print(": "); // print the name of the circuit getting the reading
  #endif*/
  
  Sensor.receive_read_cmd();              //get the response data and put it into the [Sensor].reading variable if successful
                                      
  switch (Sensor.get_error()) {             //switch case based on what the response code is.
    case Ezo_board::SUCCESS:        
      /*#if DEBUG_SERIALPRINT
      Serial.println(Sensor.get_last_received_reading());   //the command was successful, print the reading
      #endif*/
      result = Sensor.get_last_received_reading();
      break;

    case Ezo_board::FAIL:          
      /*#if DEBUG_SERIALPRINT
      Serial.println("Failed ");        //means the command has failed.
      #endif*/
      result = -1;
      break;  

    case Ezo_board::NOT_READY:      
      /*#if DEBUG_SERIALPRINT
      Serial.println("Pending ");       //the command has not yet been finished calculating.
      #endif      */
      result = -2;
      break;

    case Ezo_board::NO_DATA:      
      /*#if DEBUG_SERIALPRINT
      Serial.println("No Data ");       //the sensor has no data to send.
      #endif*/
      result = -3;
      break;
  }
  
  return(result);
}

#if USE_BLE
/*
 * @brief Function options for when a bluetooth connection is made.
 * 
 * @param
 * 
 * @return
 */
void CommandMode(){
  while(ble.available()>0){ //While connected via bluetooth...    
    int CMD = ble.read();  //...read any incoming user value.    
        
    switch (CMD){ //Test command.
      case '+':{  // Increase AS7341 gain
        ble.println("Increasing AS7341 gain");
        AS7341increaseGain();
        ble.print("Gain set to: ");
        ble.println(myGAIN);
        break;}

      case '-':{  // Decrease AS7341 gain
        ble.println("Decreasing AS7341 gain");
        AS7341decreaseGain();
        break;}
      
      case 'T':{  //Communication Test Command
        //ble.println("Comms Test Successful");
        ble.print("AS7341 integration time [ms]:  ");
        ble.println(integrationTime);        
        break;}     

      case 'Q':{ //Quit command.
         ble.println("Closing files...");
         datafile.flush();  //Clear leftovers.
         datafile.close();  //Close the main file.
         delay(1000);
         recentfile.flush(); //Clear leftovers.
         recentfile.close(); //Close the temporary file.
         delay(1000);
         ble.println("Files closed. Ready for next command.");
         break;}
        
      case 'V':{  //Queries the board and calculates the battery voltage.
        float measuredvbat = analogRead(9);
        measuredvbat *= 2; // we divided by 2, so multiply back  ???? multiplying ??
        measuredvbat *= 3.3; // Multiply by 3.3V, our reference voltage
        measuredvbat /= 1024; // convert to voltage        
        ble.print("Battery Voltage: " ); 
        ble.print(measuredvbat);
        ble.println(" V");
        if (measuredvbat<3.50 && measuredvbat>3.30){
           ble.println("It is recommended that you recharge or swap the battery soon.");  
        }
        if (measuredvbat<=3.30){
          ble.println("Battery voltage dangerously low."); 
          delay(1000);
          ble.println("Recharge or swap the battery immediately.");
          delay(1000);
          ble.println("No really, if you go any lower you run the risk of damaging your battery.");
        }
        break;}

      case 'S':{  // The send command. Sends data for viewing as a CSV.
        ble.println("Sending data from the PLOT file in 1 second."); 
        File mostrecentfile=SD.open("PLOT.CSV"); //Reopens the temporary file...
        delay(1000);
          if(mostrecentfile){
            while(mostrecentfile.available()){
              ble.write(mostrecentfile.read());  //...and sends it to your phone.
            }
            mostrecentfile.close();  //Close it.
          }
        break;}
        
      case 'P':{ // The plot command. Sends delayed data for viewing as a plot in the Bluefruit App.
        ble.println("Sending data from the most recent file in 20 seconds."); 
        ble.println("Switch to plotter view now.");
        delay(20000);
        File mostrecentfile=SD.open("PLOT.CSV"); //Reopens the temporary file...
          if(mostrecentfile){
            while(mostrecentfile.available()){
              ble.write(mostrecentfile.read());  //...and sends it to your phone.
            }
            mostrecentfile.close();  //Close it.
          }
        break;}

      case 'I':{ //Information command. Gives device name, date, time, atmp, air temp, and previously established latitude.
        DateTime now = rtc.now();
        ble.print("Device ID: ");
        ble.println(BROADCAST_NAME); 
        ble.print("Datetime: ");;
        ble.print(now.month(),DEC);    //Print month to your phone.
        ble.print("/");
        ble.print(now.day(),DEC);   //print date to your phone.
        ble.print("/");
        ble.print(now.year(),DEC); //Print year to your phone.
        ble.print(",");   //Comma delimited.
        ble.print(now.hour(),DEC);   //Print hour to your phone.
        ble.print(":");
        ble.print(now.minute(),DEC);
        ble.print(":");
        ble.println(now.second(),DEC); //Print date to your phone.
        ble.print("Atmospheric Pressure: ");
        ble.print(AtmP);
        ble.println(" mbars");    
        ble.print("Air Temperature: ");
        ble.print(AirTemp);
        ble.println(" degC");  
        ble.print("Pre-defined Latitude: ");
        ble.println(latitude); 
        break;}
    }
  }
}
#endif

/*
 * @brief Increase the AS7341 gain.
 * 
 * @param
 * 
 * @return
 */
void AS7341increaseGain(){
  switch(myGAIN){
    case AS7341_GAIN_0_5X:
      myGAIN = AS7341_GAIN_1X;
      as7341.setGain(myGAIN);
      break;

    case AS7341_GAIN_1X:
      myGAIN = AS7341_GAIN_2X;
      as7341.setGain(myGAIN);      
      break;

    case AS7341_GAIN_2X:
      myGAIN = AS7341_GAIN_4X;
      as7341.setGain(myGAIN);
      break;

    case AS7341_GAIN_4X:
      myGAIN = AS7341_GAIN_8X;
      as7341.setGain(myGAIN);
      break;

    case AS7341_GAIN_8X:
      myGAIN = AS7341_GAIN_16X;
      as7341.setGain(myGAIN);
      break;

    case AS7341_GAIN_16X:
      myGAIN = AS7341_GAIN_32X;
      as7341.setGain(myGAIN);
      break;

    case AS7341_GAIN_32X:
      myGAIN = AS7341_GAIN_64X;
      as7341.setGain(myGAIN);
      break;

    case AS7341_GAIN_64X:
      myGAIN = AS7341_GAIN_128X;
      as7341.setGain(myGAIN);
      break;

    case AS7341_GAIN_128X:
      myGAIN = AS7341_GAIN_256X;
      as7341.setGain(myGAIN);
      break;

    case AS7341_GAIN_256X:
      myGAIN = AS7341_GAIN_512X;
      as7341.setGain(myGAIN);
      break;

    default:
      break;
  }
}

/*
 * @brief Decrease the AS7341 gain.
 * 
 * @param
 * 
 * @return
 */
void AS7341decreaseGain(){
  switch(myGAIN){
    case AS7341_GAIN_1X:
      myGAIN = AS7341_GAIN_0_5X;
      as7341.setGain(myGAIN);      
      break;

    case AS7341_GAIN_2X:
      myGAIN = AS7341_GAIN_1X;
      as7341.setGain(myGAIN);
      break;

    case AS7341_GAIN_4X:
      myGAIN = AS7341_GAIN_2X;
      as7341.setGain(myGAIN);
      break;

    case AS7341_GAIN_8X:
      myGAIN = AS7341_GAIN_4X;
      as7341.setGain(myGAIN);
      break;

    case AS7341_GAIN_16X:
      myGAIN = AS7341_GAIN_8X;
      as7341.setGain(myGAIN);
      break;

    case AS7341_GAIN_32X:
      myGAIN = AS7341_GAIN_16X;
      as7341.setGain(myGAIN);
      break;

    case AS7341_GAIN_64X:
      myGAIN = AS7341_GAIN_32X;
      as7341.setGain(myGAIN);
      break;

    case AS7341_GAIN_128X:
      myGAIN = AS7341_GAIN_64X;
      as7341.setGain(myGAIN);
      break;

    case AS7341_GAIN_256X:
      myGAIN = AS7341_GAIN_128X;
      as7341.setGain(myGAIN);      
      break;

    case AS7341_GAIN_512X:
      myGAIN = AS7341_GAIN_256X;
      as7341.setGain(myGAIN);
      break;

    default:
      break;      
    }
}


/*
 * @brief Calculate the AS7341 full scale based on ATIME and ASTEP register settings.
 * 
 * @param
 * 
 * @return The calculated fullscale as a float.
 */
float AS7341fullScale(){
  float fullScale = (as7341.getATIME()+1) * (as7341.getASTEP()+1);
  if(fullScale > 65535) fullScale = 65535;

  return fullScale;
}


/*
 * @brief Quick and dirty Automatic Gain Control: Check the Clear channel and change the gain according
 *        to the calculated limits (below 10% full scale or above 90% full scale gain is adjusted).
 * 
 * @param
 * 
 * @return The calculated fullscale as a float.
 * 
 * @remarks Could be advantageously replaced by the AS7341 internal functions of AGC.
 */
void AS7341gainControl(){
  //uint16_t clearRAWcounts = as7341.getChannel(AS7341_CHANNEL_CLEAR);
  float clearRAWcounts = RAW_color_readings[10];

  if(float(clearRAWcounts) < (0.1f*float(AS7341fullScale()))){
    AS7341increaseGain();
  }

  if(float(clearRAWcounts) > (0.9f*float(AS7341fullScale()))){
    AS7341decreaseGain();
  }
}
