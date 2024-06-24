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
#include "my_functions.h"
  
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

float AirTemp, Celsius                            //A voir si on a besoin de toute les unités ?
float AtmP, AbsPressure, Decibars, Meters;        //Pareil, besoin de toute les unités ?

        char ec_data[48];                                               //Pas utilisé, utile ??
        byte in_char = 0, i = 0 ;                                       //Pas utilisé, utile ??
        char *ec, *tds, *sal, *sg;
                              
float ec_float, tds_float, sal_float, sg_float;                 
String BROADCAST_CMD = String("AT+GAPDEVNAME=" + BROADCAST_NAME);

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
AtmP PressureZero(psensor);

/*Température ambiante*/
AirTemp = AirTemperature(tsensor);

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
      SdFile::dateTimeCallback(SDCardDateTimeCallback());       // Set date time callback function: this is required so the creation date of the file is correct.
      datafile=SD.open(filename,FILE_WRITE);                    //Create a new file if the file doesn't already exist.
      recentfile=SD.open("PLOT.CSV",FILE_WRITE); 
    }
  }

    if(USE_ATLAS == 1){         //Print the header line(s) to the file.
        PrintHeaders(datafile, true);
    }else{
        PrintHeaders(datafile, false);
    } 

  tsensor.init();   //Initialize the temp sensor.
  delay(250);     
  psensor.init();   //Initialize the pressure sensor.
  delay(250);
  psensor.setModel(MS5837::MS5837_30BA);    //Define the pressure sensor model.
  psensor.setFluidDensity(1028);            //Set the approximate fluid density of deployment. Global ocean average is 1035. Coastal/estuarine waters between 1010-1035 kg/m^3.
  PressureZero();
  AirTemperature();

  if (init_AS7341 (as7341) == 0){
    #if DEBUG_SERIALPRINT
      Serial.println("AS7341 initialized");
    #endif 
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
    start_BLE(ble, buf[60]);
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
Decibars = ((AbsPressure - 21) - AtmP) / 100;
Meters = get_depth_meters(AbsPressure, AtmP, latitude)

/*Recuperation de la température en °C*/
Celsius = get_temperature(tsensor);


/*
 * @brief Print data in two files : datafile for the logging file, and recentfile for data plotting with the phone's app.
 * 
 * @param
 * 
 * @return
 */
void PrintToFile(){  //Function for printing data to the SD card and a serial monitor.
    if(print_info(1, datafile, AbsPressure, Decibars, Meters, Celsius, average_color_readings[], nbSamples) != 0){
        Serial.println("Erreur lors de l'écriture du fichier CSV");
    }
    
    #if USE_ATLAS    
        mesure_atlas(datafile, EC, PH, ORP, DO);
    #endif    
    
    if(print_info(2, datafile, AbsPressure, Decibars, Meters, Celsius, average_color_readings[], nbSamples) != 0){
        Serial.println("Erreur lors de l'écriture du fichier CSV");
    }
    
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

