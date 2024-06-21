#include "atlas.h"


void init_atlas (Ezo_board EC, Ezo_board PH, Ezo_board ORP, Ezo_board DO)
{
EC  = Ezo_board(100, "EC");     //create an EC circuit object who's address is 100 and name is "EC"
PH  = Ezo_board(99, "PH");      //create a PH circuit object, who's address is 99 and name is "PH"
ORP = Ezo_board(98, "ORP");     //create an ORP circuit object who's address is 98 and name is "ORP"
DO  = Ezo_board(97, "DO");      //create an DO circuit object who's address is 97 and name is "DO"
}

void mesure_atlas(Ezo_board EC, Ezo_board PH, Ezo_board ORP, Ezo_board DO)
{
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
}

void send_read_atlas(Ezo_board EC, Ezo_board PH, Ezo_board ORP, Ezo_board DO)
{
    EC.send_read_with_temp_comp(Celsius); // Get the conductivity with temperature compensation.
    PH.send_read_cmd();
    ORP.send_read_cmd();
    DO.send_read_cmd();
}

void serial_value_atlas(Ezo_board EC, Ezo_board PH, Ezo_board ORP, Ezo_board DO)
{
    float ec_val  = receive_reading(EC);            //get the reading from the EC circuit
    float ph_val  = receive_reading(PH);            //get the reading from the PH circuit
    float orp_val = receive_reading(ORP);           //get the reading from the ORP circuit
    float do_val  = receive_reading(DO);            //get the reading from the DO circuit
    Serial.print("EC: ");Serial.println(ec_val);
    Serial.print("PH: ");Serial.println(ph_val);
    Serial.print("OR: ");Serial.println(orp_val);
    Serial.print("DO: ");Serial.println(do_val);
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

