#include <Arduino.h>
#include "sensors.h"
#include "config.h"

// Classes Methods
/**
 *  @brief PCA9540B class constructor. Initializes the I2C address.
*/
PCA9540B::PCA9540B(uint8_t address) : _address(address) {}

/** 
 * @brief Initialize the I2C bus 
 */
void PCA9540B::begin() {
  Wire.begin();
}

/**
 *  @brief PCA9540B I²C-bus multiPLEXER_DEBUG. 
 *  Select channel.
*/
void PCA9540B::selectChannel(uint8_t channel) {
  Wire.beginTransmission(_address);
  if (channel == 0) {
    Wire.write(CHANNEL_0);
    #if PLEXER_DEBUG
      Serial.println(F("Channel 0 selected"));
    #endif
  } 
  else if (channel == 1) {
    Wire.write(CHANNEL_1);
    #if PLEXER_DEBUG
      Serial.println(F("Channel 1 selected"));
    #endif
  } 
  else {
    Wire.write(DEFAULT);
    #if PLEXER_DEBUG
      Serial.println(F("default state"));
    #endif
  }
  Wire.endTransmission();
}

/*----------Functions---------*/
/**
 * @brief Fill this in to prevent the possibility of getting stuck forever if you missed the result, or whatever.
 * @return FALSE if no timing issue. TRUE if timing issue.
 */
bool yourTimeOutCheck(){
  return false;
}

/**
 * @brief Calculate atmospheric pressure when the prove is above water (as it should be in normal use-case).
 * @param psensor : pressure sensor class
 * @return AtmP (atmospheric pressure above water)
 */
float PressureZero(MS5837 psensor){  //Determines atmospheric pressure if the user turns the unit on above water.
  psensor.read();
  float AtmP = psensor.pressure();
  return AtmP;
}


/** 
 * @brief Reads air temperature when the probe is above water (as it should be in normal use-case).
 * @param tsensor : temperature sensor class
 * @return AirTemp (air temperature above water)
 */
float AirTemperature(TSYS01 tsensor){  //Determines air temperature if the user turns the unit on above water.
  tsensor.read();
  float AirTemp = tsensor.temperature();
  return AirTemp;
}

/** 
 * @brief IConversion from pressure to depth. See AN69 by Seabird Scientific.
 * 
 * @param psensor : pressure sensor class
 * @param Decibars
 * @param Meters
 * @param Feet
 * @param Fathoms
 * @param AtmP
 * @param AbsPressure
 * 
 * @return none
 */
void get_pressure_depth(MS5837 psensor, float* Decibars, float* Meters, float* Feet, float*Fathoms, float* AtmP, float* AbsPressure){  
  psensor.read();  //Read the pressure sensor.
  
  *AbsPressure = psensor.pressure();
  *AbsPressure = *AbsPressure + 21; //Offset for this particular pressure sensor. Measured against baro sensor on RV Sikuliaq. Verified by CE02SHSM baro sensor.
  
  float x = sin(latitude / 57.29578);
  x = x * x;
  float gr = 9.780318 * (1.0 + (5.2788e-3 + 2.36e-5 * x) * x) + 1.092e-6 * (*Decibars);

  *Decibars = (psensor.pressure() - (*AtmP) )/100; 
  *Meters = ((((-1.82e-15 * (*Decibars) + 2.279e-10) * (*Decibars) - 2.2512e-5) * (*Decibars) + 9.72659) * (*Decibars) )/gr; //Depth in meters.
  *Feet = *Meters * 3.28084;
  *Fathoms = *Feet/6;
}

/**
 * @brief Reads the temperature, and calculate in various units. Temperature is stored in global variables.
 * 
 * @param tsensor : temperature sensor class
 * @param Celsius
 * @param Farenheit
 * @param Kelvin
 * 
 * @return none
 */
void get_temperature(TSYS01 tsensor, float* Celsius, float* Fahrenheit, float* Kelvin){ 
  tsensor.read();
  *Celsius = tsensor.temperature();
  *Fahrenheit = *Celsius*1.8+32;     
  *Kelvin = *Celsius+273.15;     
}

/**
 * @brief Calculates salinity according to the Practical Salinity Scale (PSS-78 ).
 * 
 * @param ec_float : electric conductivity of your aquatic medium
 * @param Celsius
 * @param Decibars
 * 
 * @return salinity calculated as a float
 */
float calc_salinity(float ec_float, float Celsius, float Decibars)
{  //PSS-78
  float R, RpNumerator, RpDenominator, Rp, rT, RT, Salinity ;
  R = ((ec_float/1000)/SalCStandard);   
  RpNumerator = ( SalA1*Decibars)*( SalA2*pow(Decibars,2))+( SalA3*pow(Decibars,3));
  RpDenominator = 1*( SalB1*Celsius)+( SalB2*pow(Celsius,2))+( SalB3*R)+( SalB4*Celsius*R);
  Rp = 1+(RpNumerator/RpDenominator);
  rT =  Salc0 +( Salc1*Celsius)+( Salc2*pow(Celsius,2))+( Salc3*pow(Celsius,3))+( Salc4*pow(Celsius,4));
  RT=R/(rT*Rp);
  Salinity = ( Sala0+( Sala1*pow(RT,0.5))+( Sala2*RT)+( Sala3*pow(RT,1.5))+( Sala4*pow(RT,2))+( Sala5*pow(RT,2.5)))+((Celsius-15)/(1+ Salk*(Celsius-15)))*( Salb0+( Salb1*pow(RT,0.5))+( Salb2*RT)+( Salb3*pow(RT,1.5))+( Salb4*pow(RT,2))+( Salb5*pow(RT,2.5)));
  
  return Salinity;
}

/**
 * @brief Calculate the AS7341 full scale based on ATIME and ASTEP register settings.
 * 
 * @param as7341 : color sensor class
 * 
 * @return The calculated fullscale as a float.
 */
float AS7341fullScale(Adafruit_AS7341 as7341){
  float fullScale = (as7341.getATIME()+1) * (as7341.getASTEP()+1);
  if(fullScale > 65535) fullScale = 65535;

  return fullScale;
}

/**
 * @brief Increase the AS7341 color sensor gain.
 * 
 * @param as7341 : color sensor class
 * @param myGAIN
 * 
 * @return
 */
void AS7341increaseGain(Adafruit_AS7341 as7341, as7341_gain_t myGAIN){
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

/**
 * @brief Decrease the AS7341 color sensor gain.
 * 
 * @param as7341 : color sensor class
 * @param myGAIN
 * 
 * @return none
 */
void AS7341decreaseGain(Adafruit_AS7341 as7341, as7341_gain_t myGAIN){
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

/**
 * @brief Quick and dirty Automatic Gain Control: Check the Clear channel and change the gain according
 *        to the calculated limits (below 10% full scale or above 90% full scale gain is adjusted).
 * 
 * @param as7341 : color sensor class
 * @param myGAIN
 * @param RAW_color_readings
 * 
 * @return The calculated fullscale as a float.
 * 
 * @remarks Could be advantageously replaced by the AS7341 internal functions of AGC.
 */
void AS7341gainControl(Adafruit_AS7341 as7341, as7341_gain_t myGAIN, uint16_t RAW_color_readings[12]){
  //uint16_t clearRAWcounts = as7341.getChannel(AS7341_CHANNEL_CLEAR);
  float clearRAWcounts = RAW_color_readings[10];

  if(float(clearRAWcounts) < (0.1f*float(AS7341fullScale(as7341)))){
    AS7341increaseGain(as7341, myGAIN);
  }

  if(float(clearRAWcounts) > (0.9f*float(AS7341fullScale(as7341)))){
    AS7341decreaseGain(as7341, myGAIN);
  }
}

/**
 * @brief Function to decode the reading after the read command was issued.
 * Taken from I2C_read_multiple_circuits.ino from Atlas Scientific Instructables.
 * 
 * @param Ezo_board : the targetted sensor class
 * 
 * @return sensor reading as a float
 */
float receive_reading(Ezo_board &Sensor) {
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

/**
 * @brief Function to issue measuring and reading command to the Atlas sensors. 
 * 2 sensors are working simultaneously to avoid power related issues
 * 
 * @param pca9540b : I²C Bus multiplexer class
 * @param EC : Atlas electric conductivity sensor class
 * @param PH : Atlas PH sensor class
 * @param DO : Atlas dissolved O² sensor class 
 * @param ORP: Atlas oxydo-reduction potential sensor class 
 * @param Celsius : temperature of the aquatic medium
 * @param ec_val : pointer to the variable to store the measure 
 * @param ph_val : pointer to the variable to store the measure 
 * @param do_val : pointer to the variable to store the measure 
 * @param orp_val: pointer to the variable to store the measure 
 *
 * @return none
 */
void Ezo_2by2(PCA9540B pca9540b, Ezo_board EC,Ezo_board PH, Ezo_board DO, Ezo_board ORP, float Celsius, float* ec_val, float* ph_val, float* do_val, float* orp_val){
  pca9540b.selectChannel(1);  // switch to EZO sensors channel
  delay(1);
  digitalWrite(PIN_EC, HIGH);
  digitalWrite(PIN_PH, HIGH);
  digitalWrite(PIN_DO, LOW);
  digitalWrite(PIN_ORP, LOW);
  delay(800);
  EC.send_read_with_temp_comp(Celsius);  // Get the conductivity with temperature compensation.
  PH.send_read_cmd();
  delay(1000);
  *ec_val = receive_reading(EC);    //get the reading from the EC circuit
  *ph_val = receive_reading(PH);    //get the reading from the PH circuit
  delay(1);
  digitalWrite(PIN_EC, LOW);
  digitalWrite(PIN_PH, LOW);
  digitalWrite(PIN_DO, HIGH);
  digitalWrite(PIN_ORP,HIGH);
  delay(800);
  ORP.send_read_cmd();
  DO.send_read_cmd();
  delay(1000);      
  *orp_val = receive_reading(ORP);  //get the reading from the ORP circuit
  *do_val = receive_reading(DO);    //get the reading from the DO circuit
  delay(1);
  digitalWrite(PIN_EC, LOW);
  digitalWrite(PIN_PH, LOW);
  digitalWrite(PIN_DO, LOW);
  digitalWrite(PIN_ORP,LOW);
}

/**
 * @brief Function to issue measuring and reading command to the Atlas sensors. 
 * all sensors activated at once for fastest measurement mode 
 * 
 * @param pca9540b : I²C Bus multiplexer class
 * @param EC : Atlas electric conductivity sensor class
 * @param PH : Atlas PH sensor class
 * @param DO : Atlas dissolved O² sensor class 
 * @param ORP: Atlas oxydo-reduction potential sensor class 
 * @param Celsius : temperature of the aquatic medium
 * @param ec_val : pointer to the variable to store the measure 
 * @param ph_val : pointer to the variable to store the measure 
 * @param do_val : pointer to the variable to store the measure 
 * @param orp_val: pointer to the variable to store the measure 
 *
 * @return none
 */
void Ezo_4once(PCA9540B pca9540b, Ezo_board EC,Ezo_board PH, Ezo_board DO, Ezo_board ORP, float Celsius, float* ec_val, float* ph_val, float* do_val, float* orp_val){
  pca9540b.selectChannel(1); // switch to EZO sensors channel
  digitalWrite(PIN_EC, HIGH);
  digitalWrite(PIN_PH, HIGH);
  digitalWrite(PIN_DO, HIGH);
  digitalWrite(PIN_ORP,HIGH);
  delay(800);
  EC.send_read_with_temp_comp(Celsius);  // Get the conductivity with temperature compensation.
  PH.send_read_cmd();
  ORP.send_read_cmd();
  DO.send_read_cmd();
  delay(800);
  *ec_val = receive_reading(EC);    //get the reading from the EC circuit
  *ph_val = receive_reading(PH);    //get the reading from the PH circuit
  *orp_val = receive_reading(ORP);  //get the reading from the ORP circuit
  *do_val = receive_reading(DO);    //get the reading from the DO circuit
  delay(1);
  digitalWrite(PIN_EC, LOW);
  digitalWrite(PIN_PH, LOW);
  digitalWrite(PIN_DO, LOW);
  digitalWrite(PIN_ORP,LOW);
}