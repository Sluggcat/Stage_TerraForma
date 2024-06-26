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
#include "config.h"
  RTC_PCF8523 rtc; 

// Initializing TerraForma sensors
  #if USE_BLE
    Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
    uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
  #endif

  #if USE_ATLAS
      Ezo_board EC  = Ezo_board(100, "EC");      //create an EC circuit object who's address is 100 and name is "EC"
      Ezo_board PH  = Ezo_board(99, "PH");       //create a PH circuit object, who's address is 99 and name is "PH"
      Ezo_board ORP = Ezo_board(98, "ORP");     //create an ORP circuit object who's address is 98 and name is "ORP"
      Ezo_board DO  = Ezo_board(97, "DO");       //create an DO circuit object who's address is 97 and name is "DO"
  #endif

  #if USE_OLED 
      // Declare the OLED display  
      #include <Adafruit_SH110X.h>
      Adafruit_SH1107 oled = Adafruit_SH1107(64, 128, &Wire);
      
      /*
      #include <Adafruit_SSD1306.h>  
      Adafruit_SSD1306 oled = Adafruit_SSD1306(128, 32, &Wire);
      */
  #endif

  Adafruit_AS7341 as7341;
  TSYS01 tsensor;
  MS5837 psensor;

//Sensor variables------------------------
float Salinity;
float AirTemp, Celsius, Fahrenheit, Kelvin;
float AtmP, AbsPressure, Decibars, Meters, Feet, Fathoms;
float vbatt;
//---------------------------------------
  
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);
extern uint8_t packetbuffer[];
char buf[60];
File datafile; File recentfile;

char ec_data[48];   
byte in_char = 0, i = 0 ;                             
char *ec, *tds, *sal, *sg;                      
float ec_float, tds_float, sal_float, sg_float;
String BROADCAST_CMD = String("AT+GAPDEVNAME=" + BROADCAST_NAME);




// AS7341 related variables --------------
  /* ATIME and ASTEP are registers that sets the integration time of the AS7341 according to the following:
    t_int = (ATIME + 1) x (ASTEP + 1) x 2.78 µs
  */

  uint16_t myATIME = 29; //599; 
  uint16_t myASTEP = 599; //20;

  float integrationTime = 0;
  uint8_t nbSamples = 0;

  as7341_gain_t myGAIN = AS7341_GAIN_32X;

  uint16_t RAW_color_readings[12];  // Contains the last RAW readings for the 12 channels of the AS7341
  float average_color_readings[12]; // Average calculation to improve the spectrum quality.

  uint8_t colorList[10] = {0, 1, 2, 3, 6, 7, 8, 9, 10, 11}; // Monotonous indexing for channels, makes it easier to loop !

  float Offset_corrected_readings[12]; // Offset compensation based on Basic counts
  float Basic_count_offset[12] = {0, 0,0,0,0,0,0,0,0,0,0,0}; // Offset values for each channel

  bool reading_request_phase = true;        //selects our phase
  uint32_t next_poll_time = 0;              //holds the next time we receive a response, in milliseconds
  const unsigned int response_delay = 2000; //how long we wait to receive a response, in milliseconds
//---------------------------------------

/*
 * @brief Prints a header line to the CSV file for variables identification.
 * 
 * @param
 * 
 * @return
 */
void PrintHeaders(){ //Prints a header line to the CSV for column identification.
   if(datafile){ 
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
/*
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
*/

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
  PressureZero(psensor);
  AirTemperature(tsensor);

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
    Salinity = calc_salinity(ec_float, Celsius, Decibars);
    
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
    get_temperature(tsensor, &Celsius, &Fahrenheit, &Kelvin);          // Get the temp and perform conversions.
    get_pressure_depth(psensor, &Decibars, &Meters, &Feet, &Fathoms, &AtmP, &AbsPressure);       // Get the pressure and perform conversions.

    #if USE_ATLAS
      EC.send_read_with_temp_comp(Celsius); // Get the conductivity with temperature compensation.
      PH.send_read_cmd();
      ORP.send_read_cmd();
      DO.send_read_cmd(); 
    #endif                     

    AS7341gainControl(as7341, myGAIN, RAW_color_readings);
    
    next_poll_time = millis() + response_delay;   //set when the response will arrive
    reading_request_phase = false;                //switch to the receiving phase
  }
  else {                               // Receiving phase
    if (millis() >= next_poll_time) {  //and its time to get the response
      #if USE_ATLAS
      float ec_val = receive_reading(EC);             //get the reading from the EC circuit
      float ph_val = receive_reading(PH);             //get the reading from the PH circuit
      float orp_val = receive_reading(ORP);             //get the reading from the ORP circuit
      float do_val = receive_reading(DO);             //get the reading from the DO circuit
    
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
        /*  
        outString = "EC:" + String(ec_val) + ",";
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
          CommandMode( ble,  as7341,  myGAIN,  datafile,  recentfile, integrationTime,  rtc,  AtmP,  AirTemp);          //...continue to collect data and wait for several command options from user.
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