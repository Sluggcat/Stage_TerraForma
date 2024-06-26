#include "filehandler.h"


/*
 * @brief Prints a header line to the CSV file for variables identification.
 * 
 * @param
 * 
 * @return
 */
void PrintHeaders(File datafile){ //Prints a header line to the CSV for column identification.
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
 * @brief Print data in two files : datafile for the logging file, and recentfile for data plotting with the phone's app.
 * 
 * @param
 * 
 * @return
 */
void PrintToFile(RTC_PCF8523 rtc, File datafile, File recentfile, float AbsPressure, float Decibars, float Meters, float Celsius, float sal_float,
                  int nbSamples, float vbatt, Ezo_board EC, Ezo_board PH, Ezo_board ORP, Ezo_board DO,
                  float average_color_readings[12], uint8_t colorList[10]){  //Function for printing data to the SD card and a serial monitor.
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
    float ec_float = EC.get_last_received_reading();
    float Salinity = calc_salinity(ec_float, Celsius, Decibars);
    
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
  RTC_PCF8523 rtc; 
  DateTime now = rtc.now();
  *date = FAT_DATE(now.year(), now.month(), now.day());
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}



/*
 * @brief Create a file with the current month, day, hour, and minute as the file name.
 * 
 * @param DateTime now, File datafile, File recentfile
 * 
 * @return none
 */
void FileCreate(DateTime now, File datafile, File recentfile){
  if (SD.begin(10)) { 
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
}