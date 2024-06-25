#include "ble.h"
using namespace std;

bool USE_BLYNK = false;
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "EfKrQaamWBdiUzC7vkBl8ZCwT8sQ5NqM";

void init_BLE(Adafruit_BLE ble)
{
    #define BLUEFRUIT_SPI_CS  8
    #define BLUEFRUIT_SPI_IRQ 7
    #define BLUEFRUIT_SPI_RST 4 
    Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
    uint8_t readPacket(*ble, uint16_t timeout);
    
}

void init_BLYNK(Adafruit_BLE ble)
{
    
    #define BLYNK_USE_DIRECT_CONNECT
    #define BLYNK_PRINT Serial
    // You should get Auth Token in the Blynk App.
    // Go to the Project Settings (nut icon).
    USE_BLYNK = true;
}

void start_BLE(Adafruit_BLE ble, char buf[60])
{
    ble.begin(); //Set up bluetooth connectivity.
    ble.echo(false);  //Turn off echo.
    ble.verbose(false);
    BROADCAST_CMD.toCharArray(buf, 60);
    ble.sendCommandCheckOK(buf);
    delay(500);
    ble.setMode(BLUEFRUIT_MODE_DATA); //Set to data mode.
    delay(500);
    if (USE_BLYNK)
        Blynk.begin(auth, ble);
}

void print_color_BLE(Adafruit_BLE ble, float average_color_readings[12])
{
    // THIS IS WHAT IS SENT TO THE PLOTTER IN THE BLUEFRUIT APP.  
    ble.println(average_color_readings[0], 10); // Print color sensor channel BLUE    
    ble.println(average_color_readings[8], 10); // Print color sensor channel CLEAR
}

void write_BLE(Adafruit_BLE ble, float Celsius, float AbsPressure, Ezo_board EC, Ezo_board PH, Ezo_board ORP, Ezo_board DO, int USE_ATLAS)
{
    if (USE_BLYNK){
        Blynk.virtualWrite(V0, Celsius);
        Blynk.virtualWrite(V1, AbsPressure);
        if (USE_ATLAS == 1){
            Blynk.virtualWrite(V2,EC.get_last_received_reading());
            Blynk.virtualWrite(V3,PH.get_last_received_reading());
            Blynk.virtualWrite(V4,ORP.get_last_received_reading());
            Blynk.virtualWrite(V5,DO.get_last_received_reading()); 
        }
    }
}

void run_blynk_BLE(Adafruit_BLE ble)
{
    if (USE_BLYNK)
        Blynk.run();
}

void CommandMode(Adafruit_BLE ble){
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

