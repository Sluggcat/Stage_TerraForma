#include "Arduino.h"
#include "config.h"

#if USE_BLE
/**
 * @brief Function options for when a bluetooth connection is made.
 * 
 * @param
 * 
 * @return
 */
void CommandMode(Adafruit_BluefruitLE_SPI ble, Adafruit_AS7341 as7341, as7341_gain_t myGAIN, File datafile, File recentfile, 
                  float integrationTime, RTC_PCF8523 rtc, float AtmP, float AirTemp){
  while(ble.available()>0){ //While connected via bluetooth...    
    int CMD = ble.read();  //...read any incoming user value.    
        
    switch (CMD){ //Test command.
      case '+':{  // Increase AS7341 gain
        ble.println("Increasing AS7341 gain");
        AS7341increaseGain(as7341, myGAIN);
        ble.print("Gain set to: ");
        ble.println(myGAIN);
        break;}

      case '-':{  // Decrease AS7341 gain
        ble.println("Decreasing AS7341 gain");
        AS7341decreaseGain(as7341, myGAIN);
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
        measuredvbat *= 2; // we divided by 2, so multiply back
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

void I2C_probe(){
    byte error, address;
  int nDevices;

  Serial.println("I2C Scanning...");

  nDevices = 0; // Counter for the number of devices found

  for (address = 1; address < 127; address++) { // I2C addresses range from 1 to 126
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) { // No error: device found
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);

      nDevices++;
    } else if (error == 4) { // Unknown error
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  } else {
    Serial.println("Scan done\n");
  }
}
#endif
