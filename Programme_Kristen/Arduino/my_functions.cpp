#include "my_functions.h"

int init_AS7341 (Adafruit_AS7341 as7341){
    // Initialize the color sensor
    if (as7341.begin()){
        as7341.setATIME(myATIME);
        as7341.setASTEP(myASTEP);
        integrationTime = (myATIME + 1) * (myASTEP + 1) * 2.78 / 1000;    
        as7341.setGain(myGAIN);     
        as7341.enableLED(false);
        as7341.startReading();    // For non-blocking readings
        return 0;
    }
    return 3;
}
  
/*
 * @brief Reads air temperature when the probe is above water (as it should be in normal use-case).
 * 
 * @param
 * 
 * @return Temeprature value in float format.
 */
float AirTemperature(TSYS01 tsensor)
{  //Determines air temperature if the user turns the unit on above water.
  tsensor.read();
  return tsensor.temperature();
}

/*
 * @brief Reads the temperature, and calculate in various units. Temperature is stored in global variables.
 * 
 * @param 
 * 
 * @return
 */
float get_temperature(TSYS01 tsensor)
{ 
  tsensor.read();
  return tsensor.temperature();
  /*Fahrenheit = Celsius*1.8+32;     
  Kelvin = Celsius+273.15; */    
}

/*
 * @brief Calculate atmospheric pressure when the prove is above water (as it should be in normal use-case).
 * 
 * @param
 * 
 * @return
 */
float PressureZero(MS5837 psensor){  //Determines atmospheric pressure if the user turns the unit on above water.
  psensor.read();
  return psensor.pressure();
}

/*
 * @brief IConversion from pressure to depth. See AN69 by Seabird Scientific.
 * 
 * @param None
 * 
 * @return
 */
float get_pressure_depth(MS5837 psensor)
{  
  psensor.read();  //Read the pressure sensor.
  //A modifier pour simplifier
  AbsPressure = psensor.pressure();
  return (AbsPressure + 21); //Offset for this particular pressure sensor. Measured against baro sensor on RV Sikuliaq. Verified by CE02SHSM baro sensor.
  /*
  Decibars = (psensor.pressure()- AtmP)/100;   
  x = sin(latitude / 57.29578);
  x = x * x;
  gr = 9.780318 * (1.0 + (5.2788e-3 + 2.36e-5 * x) * x) + 1.092e-6 * Decibars;
  Meters = ((((-1.82e-15 * Decibars + 2.279e-10) * Decibars - 2.2512e-5) * Decibars + 9.72659) * Decibars)/gr; //Depth in meters.
  Feet = Meters * 3.28084;
  Fathoms = Feet/6;
  */
}

float get_depth_meters(float AbsPressure, float AtmP, float latitude)
{
    float decibars = ((AbsPressure - 21) - AtmP) / 100;
    float x = sin(latitude / 57.29578);
    x = x * x;
    float gr = 9.780318 * (1.0 + (5.2788e-3 + 2.36e-5 * x) * x) + 1.092e-6 * decibars;
    Meters = ((((-1.82e-15 * decibars + 2.279e-10) * decibars - 2.2512e-5) * decibars + 9.72659) * decibars)/gr; //Depth in meters.
}

/*
 * @brief Reads battery voltage (if there is one connected).d
 * 
 * @param
 * 
 * @return
 */
float get_voltage(){
  float vbatt = analogRead(9);
  vbatt *= 2;
  vbatt *= 3.3;
  vbatt /= 1024;
  return vbatt;
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
 * @brief Prints a header line to the CSV file for variables identification.
 * 
 * @param
 * 
 * @return
 */
void PrintHeaders(File datafile, bool USE_ATLAS)
{ //Prints a header line to the CSV for column identification.
    if(datafile){
        datafile.print("Date,PST,abspres,dbars,meters,degC,");
        if (USE_ATLAS){ 
            datafile.print("ec,EZOsal,PSS-78,EZO_EC,EZO_pH,EZO_ORP,EZO_DO,");
        }
        datafile.print("F1_415nm,F2_445nm,F3_480nm,F4_515nm,F5_555nm,F6_590nm,F7_630nm,F8_680nm,F9_Clear,F10_NIR,NB samples,");
        datafile.println("vbatt");  
        datafile.flush();
    }
}

int print_info(int data, File datafile, float AbsPressure, float Decibars, float Meters, float Celsius, float average_color_readings[], uint8_t nbSamples)
{
    if(data = 1){
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
            return 0;
        }else{
            return 2;
        }else if(data = 2){
            if(datafile){
                // Print the Basic color readings instead of RAW data (takes gain and integration time into account).
                for(int j=0;j<10;j++){
                    //datafile.print(as7341.toBasicCounts(RAW_color_readings[colorList[j]]), 7);
                    datafile.print(average_color_readings[colorList[j]], 7);
                    datafile.print(",");
                }

                datafile.print(nbSamples);
                datafile.print(",");    
                    
                datafile.println(get_voltage());
                datafile.flush();   //Close the file.
                return 0;
            }else{
                return 2;
            }
        }
        return 1;
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
void AS7341increaseGain(as7341_gain_t myGAIN){
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
void AS7341decreaseGain(as7341_gain_t myGAIN){
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
float AS7341fullScale(Adafruit_AS7341 as7341){
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
void AS7341gainControl(uint16_t RAW_color_readings[]){
  //uint16_t clearRAWcounts = as7341.getChannel(AS7341_CHANNEL_CLEAR);
  float clearRAWcounts = RAW_color_readings[10];

  if(float(clearRAWcounts) < (0.1f*float(AS7341fullScale()))){
    AS7341increaseGain();
  }

  if(float(clearRAWcounts) > (0.9f*float(AS7341fullScale()))){
    AS7341decreaseGain();
  }
}

