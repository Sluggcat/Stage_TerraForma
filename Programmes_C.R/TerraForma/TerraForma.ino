/*
 * Code adapted for the Terra Forma demo:
 * - switch from Adafruit Feather M0 BLE to Adafruit Feather M0 LoRa
 * 01-10-2021: Add an option to disable Atlas probes.
 * 28-09-2021: Change the AS7341 reading method from blocking to non-blocking calls. Based on "reading_while_looping"
 * Adafruit library's example.
 * 
 * Remarks regarding acquisition frequency : Atlas probes+EZO have a typical processing time of one second, hence the acquisition frequency must be > to 1000 ms.
 * 
 * TODO 20-10-2020: Add conversion function for conductivity based on the calculations in get_conductivity()
 * 
 * BLUE ROBOTICS wiring:
 * Red +3.3V
 * Green SCL
 * White SDA
 * Black GND
 *
 * This sketch allows you to collect conductivity, temperature, and depth data and stream it to your phone via the Adafruit Bluefruit phone application.
 * The application is available for both Android and iOS devices.
 * This sketch does not consider efficiency and will be updated as new functions and commands are implemented.
 * Some of this code is repurposed from sketches created by Adafruit, Atlas Scientific, and Blue Robotics. 
 * If building your own sensor, please support them by purchasing parts from their online stores.
 *
 * For questions or comments regarding this sketch, send an email to : 
 *
 * Don't forget to check out these other open source CTD variants!
 * Arduino-based Sonde  https://github.com/glockridge/MooredApplication
 * OpenCTD https://github.com/OceanographyforEveryone/OpenCTD
 * PiCTD https://github.com/haanhouse/pictd
 * Conduino https://github.com/kpdangelo/OpenCTDwithConduino
*/
#include "config.h"

// TerraForma objects
  PCA9540B pca9540b; // I²C-bus multiplexer
  Measure_sender Terra_sender(9600, 18);

  #if USE_OLED
    #include <Adafruit_SH110X.h>
    Adafruit_SH1107 oled = Adafruit_SH1107(64, 128, &Wire);
  #endif

  #if USE_BLE
    Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
    uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
    char buf[60];
    String BROADCAST_CMD = String("AT+GAPDEVNAME=" + BROADCAST_NAME);
  #endif

  #if USE_ATLAS
    Ezo_board EC = Ezo_board(100, "EC");   // address 100, electric conductivity sensor
    Ezo_board PH = Ezo_board(99, "PH");    // address 99, Ph sensor
    Ezo_board ORP = Ezo_board(98, "ORP");  // address 98, oxydo-redux.potential sensor
    Ezo_board DO = Ezo_board(97, "DO");    // address 97, dissolved O² sensor
  #endif
  Adafruit_AS7341 as7341; // color sensor
  TSYS01 tsensor; // temp. sensor
  MS5837 psensor; // pressure sensor


//---
//Sensor variables
  float Salinity;
  float AirTemp, Celsius, Fahrenheit, Kelvin;
  float AtmP, AbsPressure, Decibars, Meters, Feet, Fathoms;
  float vbatt;
  float ec_val, ph_val, do_val, orp_val;

  // AS7341 related variables
    /* ATIME and ASTEP are registers that sets the integration time of the AS7341 according to the following:
      t_int = (ATIME + 1) x (ASTEP + 1) x 2.78 µs
    */
    uint16_t myATIME = 29;   //599;
    uint16_t myASTEP = 599;  //20;

    float integrationTime = 0;
    uint8_t nbSamples = 0;

    as7341_gain_t myGAIN = AS7341_GAIN_32X;

    uint16_t RAW_color_readings[12];   // Contains the last RAW readings for the 12 channels of the AS7341
    float average_color_readings[12];  // Average calculation to improve the spectrum quality.

    uint8_t colorList[10] = { 0, 1, 2, 3, 6, 7, 8, 9, 10, 11 };  // Monotonous indexing for channels, makes it easier to loop !
    float Offset_corrected_readings[12];                                    // Offset compensation based on Basic counts
    float Basic_count_offset[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  // Offset values for each channel


    bool reading_request_phase = true;         // switch from acquisition phase to transmition phase
    uint32_t next_poll_time = 0;               
    const unsigned int response_delay = 5000;  
    uint16_t cycle = 0;
//---

/** 
 * @brief Initialize all peripherals
 */
void setup() {  
  #if USE_ATLAS
    // the OLED display won't work after a reset without this
    digitalWrite(PIN_EC, HIGH);
    digitalWrite(PIN_PH, HIGH);
    digitalWrite(PIN_DO, HIGH);
    digitalWrite(PIN_ORP,HIGH);
  #else
    digitalWrite(PIN_EC, LOW);
    digitalWrite(PIN_PH, LOW);
    digitalWrite(PIN_DO, LOW);
    digitalWrite(PIN_ORP,LOW);
  #endif

  #if DEBUG_SERIALPRINT
    Serial.begin(115200);
    while(!Serial);
    Serial.println(F("\n==="));
  #else delay(500);
  #endif

  //Initialize OLED display
    #if USE_OLED
      pca9540b.selectChannel(-1);
      #if DEBUG_SERIALPRINT
        Serial.print(F("OLED CONFIG \t"));
      #endif
      delay(20); //display from previous run cycle may remain so wait a bit to avoid a mess
      oled.begin(0x3C, true);
      oled.display();
      delay(500);
      // Clear the buffer.
      oled.clearDisplay();
      oled.display();
      oled.setRotation(1);

      // text display tests
      oled.setTextSize(1);
      oled.setTextColor(SH110X_WHITE);
      oled.setCursor(2, 0);

      pinMode(BUTTON_A, INPUT_PULLUP);
      pinMode(BUTTON_B, INPUT_PULLUP);
      pinMode(BUTTON_C, INPUT_PULLUP);
    
      #if DEBUG_SERIALPRINT
        Serial.println(F("DONE"));
      #endif
    #endif

  // Initialize the color sensor 
    #if DEBUG_SERIALPRINT
      Serial.print(F("Sensors CONFIG \t"));
    #endif
    if (as7341.begin()) { 
      as7341.setATIME(myATIME);
      as7341.setASTEP(myASTEP);
      integrationTime = (myATIME + 1) * (myASTEP + 1) * 2.78 / 1000;
      as7341.setGain(myGAIN);
      as7341.enableLED(false);
      as7341.startReading();  // For non-blocking readings
    } else {
      #if DEBUG_SERIALPRINT
          Serial.println(F("Could not find AS7341"));
      #endif 
      #if USE_OLED
        oled.clearDisplay();
        oled.setCursor(0, 0);
        oled.println("Could not find AS7341");
        oled.display();
      #endif
    }
      pca9540b.selectChannel(0);
      delay(1);
      tsensor.init();  //Initialize the temp sensor.
      delay(250);
      psensor.init();  //Initialize the pressure sensor.
      delay(250);
      psensor.setModel(MS5837::MS5837_30BA);  //Define the pressure sensor model.
      psensor.setFluidDensity(1028);          //Set the approximate fluid density of deployment. Global ocean average is 1035. Coastal/estuarine waters between 1010-1035 kg/m^3.
      PressureZero(psensor);
      AirTemperature(tsensor);
    #if DEBUG_SERIALPRINT
      Serial.println(F("\tDONE"));
    #endif
    #if USE_OLED
      oled.println(F("Sensors OK"));
      oled.display();
    #endif

  #if USE_BLE
    #if DEBUG_SERIALPRINT
      Serial.print(F("BLE CONFIG \t"));
    #endif
    ble.begin();      //Set up bluetooth connectivity.
    ble.echo(false);  //Turn off echo.
    ble.verbose(false);
    BROADCAST_CMD.toCharArray(buf, 60);
    ble.sendCommandCheckOK(buf);
    delay(200);
    ble.setMode(BLUEFRUIT_MODE_DATA);  //Set to data mode.
    delay(200);
    #if USE_BLYNK
      Blynk.begin(auth, ble);
    #endif
    #if DEBUG_SERIALPRINT
      Serial.println("\tDONE");
    #endif
    #if USE_OLED
      oled.println(F("BLE OK"));
      oled.display();
    #endif
  #endif 

  // Initialize Serial comm.
  Terra_sender.begin();

  #if DEBUG_SERIALPRINT
    Serial.println(F("\tend setup\n==="));
  #endif
  #if USE_OLED
      oled.println(F("SETUP DONE"));
      oled.display();
  #endif
}

/** 
 * @brief Run the probe
 */
void loop() {  
  if(reading_request_phase){
    // Acquisition phase
      AS7341gainControl(as7341, myGAIN, RAW_color_readings);
      pca9540b.selectChannel(0); // set channel to temp.&pressure
      get_temperature(tsensor, &Celsius, &Fahrenheit, &Kelvin);                               // Get the temp and perform conversions.
      get_pressure_depth(psensor, &Decibars, &Meters, &Feet, &Fathoms, &AtmP, &AbsPressure);  // Get the pressure and perform conversions.
      #if USE_ATLAS
        // Get Ezo sensors measures, 2 by 2
        Ezo_2by2(pca9540b, EC, PH, DO, ORP, Celsius, &ec_val, &ph_val, &do_val, &orp_val);
      #endif
    // End of acquisition

    // Checking measures for Debug
      #if DEBUG_SERIALPRINT
        #if USE_ATLAS
          Serial.print(F("EC: "));
          Serial.println(ec_val);
          Serial.print(F("PH: "));
          Serial.println(ph_val);
          Serial.print(F("ORP: "));
          Serial.println(orp_val);
          Serial.print(F("DO: "));
          Serial.println(do_val);
          #endif
          Serial.print(F("T°: "));
          Serial.print(Celsius);
          Serial.println(F(" °C"));
          Serial.print(F("P: "));
          Serial.print(AbsPressure);
          Serial.println(F(" hPa\n==="));
        #endif
        cycle++;
        #if USE_OLED
          oled.clearDisplay();
          oled.setCursor(0, 0);
          /*
          for (int k = 0; k < 10; k++) {
            oled.print("F");
            oled.print(colorList[k] + 1);
            oled.print(" ");
            oled.println(RAW_color_readings[colorList[k]]);
          }
          */

          oled.print(F("N : ")); oled.println(cycle); oled.println("");
          oled.print(F("EC: "));
          oled.println(ec_val);
          oled.print(F("PH: "));
          oled.println(ph_val);
          oled.print(F("OR: "));
          oled.println(orp_val);
          oled.print(F("DO: "));
          oled.println(do_val);
          
          oled.print(F(" T: "));
          oled.print(Celsius);
          oled.println(F(" C"));
          oled.print(F(" P: "));
          oled.print(AbsPressure);
          oled.display();
        #endif
      //---

    next_poll_time = millis() + response_delay;
    reading_request_phase = false ;
  }
  else if(millis()>=next_poll_time){
    // Transmission phase
    nbSamples = 0;  // Zeroes the AS7341 number of samples and prepare for next acquisition cycle.
    for (int j = 0; j < 10; j++) {
        average_color_readings[colorList[j]] = 0;
    }
    // SENDING DATAS HERE
      float data[18] = {-1, -1, -1, -1, Celsius, AbsPressure,
                  RAW_color_readings[colorList[0]], RAW_color_readings[colorList[1]], RAW_color_readings[colorList[2]], RAW_color_readings[colorList[3]],
                  RAW_color_readings[colorList[4]], RAW_color_readings[colorList[5]], RAW_color_readings[colorList[6]], RAW_color_readings[colorList[7]],
                  RAW_color_readings[colorList[8]], RAW_color_readings[colorList[9]], RAW_color_readings[colorList[10]], RAW_color_readings[colorList[11]]};
      #if USE_ATLAS
        data[0] = ec_val;
        data[1] = ph_val;
        data[2] = orp_val;
        data[3] = do_val;
      #endif
      if( RAW_color_readings[colorList[9]]!= 0)
        Terra_sender.sendData(data);
    // ---
    reading_request_phase = true ;
  }
  
  // Non-blocking reading for AS7341. Done in the main loop to increase sample numbers and do some averaging.
    bool timeOutFlag = yourTimeOutCheck();
    if (as7341.checkReadingProgress() || timeOutFlag) {
      if (timeOutFlag) {}
      as7341.getAllChannels(RAW_color_readings);
      nbSamples++;

      // Here we do the averaging based on the number of samples already acquired. Average calculation is based on Basic 
      // counts instead of RAW counts because of the AGC.
      for (int j = 0; j < 10; j++) {
        average_color_readings[colorList[j]] = (average_color_readings[colorList[j]] + as7341.toBasicCounts(RAW_color_readings[colorList[j]])) / nbSamples;
      }
      as7341.startReading();
    }

  #if USE_BLE
    #if USE_BLYNK
      Blynk.run();
    #endif
  #endif
}

