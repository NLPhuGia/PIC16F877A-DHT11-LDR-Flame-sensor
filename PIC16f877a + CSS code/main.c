#include <16F877A.h>
#device ADC=10
#fuses HS,NOWDT,NOPROTECT,NOLVP,NOBROWNOUT
#use i2c(Master,Fast=100000,sda=PIN_C4,scl=PIN_C3,force_sw)
#use delay(crystal=12000000)
#use fast_io(A)
#include <lcd_i2c.c>
#include <string.h>
#include <math.h>
#include <stdlib.h> 
#define DHT11_PIN PIN_A1    // connection pin between DHT11 and mcu
#define FLAME_SENSOR_PIN PIN_A2

char message1[] = "T=00.0 C";
char message2[] = "RH=00.0%";

short Time_out;
unsigned int8 T_byte1, T_byte2, RH_byte1, RH_byte2, CheckSum ;
unsigned int16 LDR_value;
//float temperature = 0.0; // variable to store temperature value
//float humidity = 0.0;    // variable to store humidity value
float temperature, humidity;


void start_signal(){
  output_drive(DHT11_PIN);    // configure connection pin as output
  output_low(DHT11_PIN);      // connection pin output low
  delay_ms(25);
  output_high(DHT11_PIN);     // connection pin output high
  delay_us(30);
  output_float(DHT11_PIN);    // configure connection pin as input
}

short check_response(){
  delay_us(40);
  if(!input(DHT11_PIN)){      // read and test if connection pin is low
    delay_us(80);
    if(input(DHT11_PIN)){     // read and test if connection pin is high
      delay_us(50);
      return 1;
    }
  }
}
   
unsigned int8 Read_Data(){
  unsigned int8 i, k, _data = 0;        // k is used to count 1 bit reading duration
  if(Time_out)
    break;
  for(i = 0; i < 8; i++){
    k = 0;
    while(!input(DHT11_PIN)){           // Wait until DHT11 pin get raised
      k++;
      if(k > 100){
        Time_out = 1;
        break;
      }
      delay_us(1);
    }
    delay_us(30);
    if(!input(DHT11_PIN))
      bit_clear(_data, (7 - i));        // Clear bit (7 - i)
    else{
      bit_set(_data, (7 - i));          // Set bit (7 - i)
      while(input(DHT11_PIN)){          // Wait until DHT11 pin goes low
        k++;
        if(k > 100){
        Time_out = 1;
        break;
      }
      delay_us(1);}
    }
  }
  return _data;
}

 void read_dht11(){
  Time_out = 0;
  Start_signal();
 if(check_response()){                     // If there is a response from sensor
      RH_byte1 = Read_Data();                 // read RH byte1
      RH_byte2 = Read_Data();                 // read RH byte2
      T_byte1 = Read_Data();                  // read T byte1
      T_byte2 = Read_Data();                  // read T byte2
      Checksum = Read_Data();                 // read checksum
if(Time_out){                           // If reading takes long time
        lcd_putc('\f');                       // LCD clear
        lcd_gotoxy(5, 1);                     // Go to column 5 row 1
        lcd_putc("Time out!");
      }
      else{
           
       if(CheckSum == ((RH_Byte1 + RH_Byte2 + T_Byte1 + T_Byte2) & 0xFF)){
        message1[2] = ' ';
        message1[2]  = T_Byte1/10  + 48;
        message1[3]  = T_Byte1%10  + 48;
        message1[5] =  T_Byte2/10  + 48;
        message2[3]  = RH_Byte1/10 + 48;
        message2[4]  = RH_Byte1%10 + 48;
        message2[6]  = RH_Byte2/10 + 48;
        message1[6] =  223;        
 
      
  temperature = ((T_byte1 * 10) + (T_byte2 % 10));
  humidity = ((RH_byte1 * 10) + (RH_byte2 % 10));
     delay_us(10);
    
       
       }
     
      }
    }
 
 }

void main() {
     lcd_init();                                 // Initialize LCD module
     lcd_putc('f');    
     set_adc_channel(0); // Select AN0 as the ADC channel
     setup_adc(ADC_CLOCK_INTERNAL);
     set_tris_a(0xFF);      // Set all pins of port A as input 
     setup_adc_ports(AN0);
 
     // Configure PORTC Pin 3 as output
  set_tris_c(0b00000000);

  set_tris_d(get_tris_a() | (1 << FLAME_SENSOR_PIN));

  set_tris_d(0b11111100);
     int flameData;

   
   while(TRUE) { // Infinite loop
         
    read_dht11();
        
        if (humidity > 85) {
   output_high(PIN_D0); // turn on the LED
    } else {
    output_low(PIN_D0); // turn off the LED
   }                     
           delay_us(30);
                          
   if (temperature > 30) {
     output_high(PIN_C7); // turn on the LED
     } else {
     output_low(PIN_C7); // turn off the LED
     }    
     delay_us(30);
           
          
         lcd_gotoxy(1, 1);                     // Go to column 1 row 1
         printf(lcd_putc, message1);           // Display message1
         lcd_gotoxy(1, 2);                     // Go to column 1 row 2
         printf(lcd_putc, message2);    // Display message2       
     
          
     LDR_value = read_adc();
     if (LDR_value > 780) {
            output_high(PIN_C5); // Turn on the LED connected to PORTC Pin 3
        } else {
            output_low(PIN_C5); // Turn off the LED connected to PORTC Pin 3
        } 
           
         delay_ms(10);    
         
     lcd_gotoxy(11, 1);                   // Go to column 2 row 1
    // Display message3
     printf(lcd_putc,"L=%04ld",LDR_value);     
     
 
    
   flameData = input(FLAME_SENSOR_PIN);
  
   if(flameData==1)
   {
   
   
      output_low(PIN_D2); 
     lcd_gotoxy(11,2);
     printf(lcd_putc,"CHAY:1");
         delay_ms(10);   
     
    
   }  
  else{
   for(int i=0; i<3 ;i++)
    {  
       output_toggle(PIN_D2);
     delay_ms(250);
      lcd_gotoxy(11,2);
      printf(lcd_putc,"CHAY:0");
    }
  }  
    }
}



