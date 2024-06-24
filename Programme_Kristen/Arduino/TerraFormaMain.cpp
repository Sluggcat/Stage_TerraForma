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

#define USE_BLE             1     // If we use Adafruit Feather M0 Bluefruit
#define USE_BLYNK           0     // Only valid with the Bluefruit.

#define LOW_POWER_MODE      0     // TODO
#define USE_ATLAS           1
#define USE_OLED            1
            

/*----------Initialization---------*/
#include "ble.h"
#include "atlas.h"
#include "oled.h"
  
#include "RTClib.h" 
#include <string.h>
#include <SD.h>  
#include <Wire.h>

#include <Adafruit_AS7341.h>
#include "TSYS01.h" 
#include "MS5837.h"
#include <avr/dtostrf.h> //For temperature value concatenation.
#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

#if USE_BLE
    Adafruit_BLE ble;
    init_BLE(ble);
#endif

#if USE_OLED
  Adafruit_SH1107 oled
#endif


#define latitude 45.00    //This is your deployment latitude. It is used in the pressure to depth conversion.

Ezo_board EC    //create an EC circuit object
Ezo_board PH    //create a PH circuit object
Ezo_board ORP   //create an ORP circuit object
Ezo_board DO    //create an DO circuit object
init_atlas (EC, PH, ORP, DO)

bool reading_request_phase = true;        //selects our phase
uint32_t next_poll_time = 0;              //holds the next time we receive a response, in milliseconds
const unsigned int response_delay = 2000; //how long we wait to receive a response, in milliseconds

String BROADCAST_NAME = "Econect Mk1";  //You can name your CTD anything!
/*---------------------------------------*/
  
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
#if USE_BLYNK
    init_BLYNK();
#endif
/*------------------------*/


/*----------Functions---------*/
/*Pression atmospherique*/
AtmP PressureZero(psensor)

/*Température ambiante*/
AirTemp = AirTemperature(tsensor);


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
    datafile.print("Date,PST,abspres,dbars,meters,degC,");
  #if USE_ATLAS  
  //Pareil  
    datafile.print("ec,EZOsal,PSS-78,EZO_EC,EZO_pH,EZO_ORP,EZO_DO,");
  #endif
  //Pareil
    datafile.print("F1_415nm,F2_445nm,F3_480nm,F4_515nm,F5_555nm,F6_590nm,F7_630nm,F8_680nm,F9_Clear,F10_NIR,NB samples,");
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
    init_oled(oled);
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
      datafile=SD.open(filename,FILE_WRITE);                    //Create a new file if the file doesn't already exist.
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
        as7341_error_oled(oled);
    #endif      
  } 
  
  #if USE_BLE
    start_BLE(buf[60]);
  #endif

  #if LOW_POWER_MODE  
  // Turn off the EZO LEDs.
  PH.send_cmd("L,0");
  ORP.send_cmd("L,0");
  EC.send_cmd("L,0");
  DO.send_cmd("L,0");
  #endif
}

AbsPressure = get_pressure_depth(psensor);
Meters = get_depth_meters(AbsPressure, AtmP, latitude)

/*Recuperation de la température en °C*/
Celsius = get_temperature(tsensor);


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
    mesure_atlas(EC, PH, ORP, DO);
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
      print_color_BLE(average_color_readings[12]);
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

  if (reading_request_phase) {   // Ask for reading
    //send a read command. we use this command instead of PH.send_cmd("R"); 
    //to let the library know to parse the reading
    get_temperature();          // Get the temp and perform conversions.
    get_pressure_depth();       // Get the pressure and perform conversions.

    #if USE_ATLAS
      send_read_atlas(EC, PH, ORP, DO); 
    #endif                     

    AS7341gainControl();
    
    next_poll_time = millis() + response_delay;   //set when the response will arrive
    reading_request_phase = false;                //switch to the receiving phase
  }
  else {                               // Receiving phase
    if (millis() >= next_poll_time) {  //and its time to get the response
      #if USE_ATLAS
        #if DEBUG_SERIALPRINT    
            serial_value_atlas(EC, PH, ORP, DO)
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
        write_BLE(Celsius, AbsPressure, EC, PH, ORP, DO, USE_ATLAS);
      #endif

      PrintToFile(); //Save data to file. 

      #if USE_OLED
        print_colorlist_oled(oled, colorList, RAW_color_readings)
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
    run_blynk_BLE()  
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
