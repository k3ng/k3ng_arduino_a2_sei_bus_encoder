/* 


  A2 SEI Bus Encoder Arduino Library

  Version 1.0.2015030901

  Anthony Good, K3NG
  anthony.good@gmail.com
  http://blog.radioartisan.com/

  Project page: http://blog.radioartisan.com/arduino-a2-encoder-library/

  Code repository: https://github.com/k3ng/k3ng_arduino_a2_sei_bus_encoder


*/


#ifndef sei_bus_h
#define sei_bus_h


#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "sei_bus.h"

HardwareSerial *serialport;
int _baudrate = 0;

uint8_t command_in_progress = 0;
uint8_t command_in_progress_address = 0;
uint8_t awaiting_response_bytes = 0;
int last_command[18];
int timeout[18];
uint8_t command_result_ready[18];
uint8_t last_command_status_nibble[18];
uint8_t raw_result_number_of_bytes = 0;
uint8_t raw_result_byte[SEI_BUS_RECEIVE_BYTES_BUFFER];
uint8_t _bus_busy_pin = 0;
uint8_t _bus_send_receive_pin = 0;
unsigned long last_command_time[18];

//-----------------------------------------------------------------------------------------------------

SEIbus::SEIbus(HardwareSerial *serialportin, int baudrate, uint8_t bus_busy_pin, uint8_t bus_send_receive_pin){

  _baudrate = baudrate;
  serialport = serialportin;
  _bus_busy_pin = bus_busy_pin;
  _bus_send_receive_pin = bus_send_receive_pin;

}

//-----------------------------------------------------------------------------------------------------

void SEIbus::clear_active_command(uint8_t timeout_status){

  command_in_progress = 0;
  awaiting_response_bytes = 0;
  timeout[command_in_progress_address] = timeout_status;
  raw_result_number_of_bytes = 0;
  last_command_time[command_in_progress_address] = millis();
  command_result_ready[command_in_progress_address] = 0;
  command_in_progress_address = 255;

}

//-----------------------------------------------------------------------------------------------------

void SEIbus::initialize(){


  /* initialize the serial port and do other stuff */


  int x = 0;

  serialport->begin(_baudrate);
  #ifdef DEBUG_SEI
  Serial.println(F("SEIbus::initialize: PORT_OPEN"));
  #endif //DEBUG_SEI

  pinMode(_bus_send_receive_pin,OUTPUT);
  digitalWrite(_bus_send_receive_pin,SEI_BUS_RECEIVE_STATE);

  pinMode(_bus_busy_pin,INPUT);
  digitalWrite(_bus_busy_pin,LOW);

  command_in_progress = 0;

  for (x = 0; x < 18; x++){
    last_command[x] = 0;
    last_command[x] = 0;
    timeout[x] = 0;
    command_result_ready[x] = 0;
    last_command_status_nibble[x] = 0;    
  }

  current_baud_rate = BAUD_9600;

  
}


//-----------------------------------------------------------------------------------------------------

void SEIbus::service(){

  /* service the serial port and check command timeout */
  /* this should be called by loop() as frequently as possible */

  uint8_t serial_port_byte = 0;
  uint8_t x = 0;

  

  //service serial port
  if (serialport->available() > 0){
    serial_port_byte = serialport->read();   
    if (command_in_progress) {
      if (raw_result_number_of_bytes < SEI_BUS_RECEIVE_BYTES_BUFFER){
        raw_result_byte[raw_result_number_of_bytes] = serial_port_byte;
        raw_result_number_of_bytes++;
      } else {
        #ifdef DEBUG_SEI
        Serial.println(F("SEIbus::service: SEI_BUS_RECEIVE_BYTES_BUFFER exceeded"));
        #endif //DEBUG_SEI
      }

      if (awaiting_response_bytes > 0) {awaiting_response_bytes--;}

      #ifdef DEBUG_SEI_SERIAL_PORT
      Serial.print(F("SEIbus::service: serial_port_byte: "));
      Serial.print(serial_port_byte);
      Serial.print(" busy pin:");
      if (bus_busy()){
        Serial.print("H");  
      } else {
        Serial.print("L");
      }
      Serial.print(" bytes: ");
      Serial.print(raw_result_number_of_bytes);
      Serial.print(" awaiting_response_bytes: ");
      Serial.println(awaiting_response_bytes);
      #endif //DEBUG_SEI_SERIAL_PORT  


    } else {  // no command in progress - this is an unknown byte on the bus 
      #ifdef DEBUG_SEI
      Serial.print(F("SEIbus::service: unknown response byte: "));
      Serial.println(serial_port_byte);
      #endif //DEBUG_SEI 
    }
  }
  

  // do we have something to process from an active command ?
  if (command_in_progress){
    switch(last_command[command_in_progress_address]){
      case SEI_BUS_A2_ENCODER_CMD_WAKEUP:
        // we wait X mS for all the devices on the bus to wakeup
        if ((millis() - last_command_time[command_in_progress_address]) >= SEI_BUS_WAKEUP_TIME_MS){
          clear_active_command(0);
          #ifdef DEBUG_SEI
          Serial.println(F("SEIbus::service: SEI_BUS_WAKEUP_TIME_MS wait time over"));
          #endif //DEBUG_SEI           
        }
        break;
      case SEI_BUS_A2_ENCODER_CMD_READ_POS:
        if ((!bus_busy()) && ((awaiting_response_bytes == 0) || ((millis() - last_command_time[command_in_progress_address]) > SEI_BUS_A2_ENCODER_CMD_READ_POS_TIMEOUT_MS))){ 
            #ifdef DEBUG_SEI
            Serial.print(F("SEIbus::service: SEI_BUS_A2_ENCODER_CMD_READ_POS result raw_bytes: "));
            //Serial.print(raw_result_number_of_bytes);
            for (x = 0;x < raw_result_number_of_bytes;x++){
              Serial.print(" ");
              Serial.print(raw_result_byte[x]);
            }
            Serial.println();
            #endif //DEBUG_SEI 

            switch(raw_result_number_of_bytes){
              case 1: position = raw_result_byte[0]; break;
              case 2: position = (raw_result_byte[0] * 256) + raw_result_byte[1]; break;
              case 4: 
                position = (raw_result_byte[0] * 16777210) + (raw_result_byte[1] * 65536) + (raw_result_byte[2] * 256) + (raw_result_byte[3]);
                position_rollover_compensated = position;
                if (raw_result_byte[0] > 128){
                  position_rollover_compensated = ((raw_result_byte[0]-255)*16777210) + ((raw_result_byte[1]-255)*65536) + ((raw_result_byte[2]-255)*256) + (raw_result_byte[3]-255);
                }
                break;
            }
            command_result_ready[command_in_progress_address] = 1;
            command_in_progress = 0;
            command_in_progress_address = 255;            
        }
        break;


      case SEI_BUS_A2_ENCODER_CMD_READ_POS_STATUS:
        if ((!bus_busy()) && ((awaiting_response_bytes == 0) || ((millis() - last_command_time[command_in_progress_address]) > SEI_BUS_A2_ENCODER_CMD_READ_POS_TIMEOUT_MS))){ 
            #ifdef DEBUG_SEI
            Serial.print(F("SEIbus::service: SEI_BUS_A2_ENCODER_CMD_READ_POS_STATUS result raw_bytes: "));
            //Serial.print(raw_result_number_of_bytes);
            for (x = 0;x < raw_result_number_of_bytes;x++){
              Serial.print(" ");
              Serial.print(raw_result_byte[x]);
            }
            Serial.println();
            #endif //DEBUG_SEI 

            switch(raw_result_number_of_bytes){
              case 2: position = raw_result_byte[0]; status = raw_result_byte[1]; break;
              case 3: position = (raw_result_byte[0] * 256) + raw_result_byte[1]; status = raw_result_byte[2]; break;
              case 5: 
                position = (raw_result_byte[0] * 16777210) + (raw_result_byte[1] * 65536) + (raw_result_byte[2] * 256) + raw_result_byte[3];
                position_rollover_compensated = position;
                if (raw_result_byte[0] > 128){
                  position_rollover_compensated = ((raw_result_byte[0]-255)*16777210) + ((raw_result_byte[1]-255)*65536) + ((raw_result_byte[2]-255)*256) + (raw_result_byte[3]-255);
                }                
                status = raw_result_byte[4];
                break;
            }

            command_result_ready[command_in_progress_address] = 1;
            command_in_progress = 0;
            command_in_progress_address = 255;            
        }
        break;


      case SEI_BUS_A2_ENCODER_CMD_READ_POS_TIME_STATUS:
        if ((!bus_busy()) && ((awaiting_response_bytes == 0) || ((millis() - last_command_time[command_in_progress_address]) > SEI_BUS_A2_ENCODER_CMD_READ_POS_TIMEOUT_MS))){ 
            #ifdef DEBUG_SEI
            Serial.print(F("SEIbus::service: SEI_BUS_A2_ENCODER_CMD_READ_POS_STATUS result raw_bytes: "));
            //Serial.print(raw_result_number_of_bytes);
            for (x = 0;x < raw_result_number_of_bytes;x++){
              Serial.print(" ");
              Serial.print(raw_result_byte[x]);
            }
            Serial.println();
            #endif //DEBUG_SEI 

            switch(raw_result_number_of_bytes){
              case 4: position = raw_result_byte[0]; time = (raw_result_byte[1] * 256) + raw_result_byte[2]; status = raw_result_byte[3]; break;
              case 5: 
                position = (raw_result_byte[0] * 256) + raw_result_byte[1]; 
                time = (raw_result_byte[2] * 256) + raw_result_byte[3];
                status = raw_result_byte[4]; break;
              case 7: 
                position = (raw_result_byte[0] * 16777210) + (raw_result_byte[1] * 65536) + (raw_result_byte[2] * 256) + raw_result_byte[3];
                position_rollover_compensated = position;
                if (raw_result_byte[0] > 128){
                  position_rollover_compensated = ((raw_result_byte[0]-255)*16777210) + ((raw_result_byte[1]-255)*65536) + ((raw_result_byte[2]-255)*256) + (raw_result_byte[3]-255);
                }                
                time = (raw_result_byte[4] * 256) + raw_result_byte[5];
                status = raw_result_byte[6];
                break;
            }
            command_result_ready[command_in_progress_address] = 1;
            command_in_progress = 0;
            command_in_progress_address = 255;            
        }
        break;

      case SEI_BUS_A2_ENCODER_READ_FACTORY_INFO:
        if (awaiting_response_bytes == 0){ 
            #ifdef DEBUG_SEI
            Serial.print(F("SEIbus::service: SEI_BUS_A2_ENCODER_READ_FACTORY_INFO result raw_bytes: "));
            //Serial.print(raw_result_number_of_bytes);
            for (x = 0;x < raw_result_number_of_bytes;x++){
              Serial.print(" ");
              Serial.print(raw_result_byte[x]);
            }

            Serial.println();
            #endif //DEBUG_SEI 

            model_number =  (raw_result_byte[0] * 256) + raw_result_byte[1];
            version_number = raw_result_byte[2] + (raw_result_byte[3]/100.0); 
            configuration_byte_1 = raw_result_byte[4];
            configuration_byte_1 = raw_result_byte[5];
            serial_number = (raw_result_byte[6] * 16777210) + (raw_result_byte[7] * 65536) + (raw_result_byte[8] * 256) + raw_result_byte[9];  
            month = raw_result_byte[10];
            day = raw_result_byte[11];
            year = (raw_result_byte[12] * 256) + raw_result_byte[13];  


            command_result_ready[command_in_progress_address] = 1;
            command_in_progress = 0;
            command_in_progress_address = 255;            
        }      
        break;  
      case SEI_BUS_A2_ENCODER_READ_RESOLUTION:
        if (awaiting_response_bytes == 0){ 
          #ifdef DEBUG_SEI
          Serial.print(F("SEIbus::service: SEI_BUS_A2_ENCODER_READ_RESOLUTION result raw_bytes: "));
          //Serial.print(raw_result_number_of_bytes);
          for (x = 0;x < raw_result_number_of_bytes;x++){
            Serial.print(" ");
            Serial.print(raw_result_byte[x]);
          }
          Serial.println();
          #endif //DEBUG_SEI
          resolution = (raw_result_byte[0] * 256) + raw_result_byte[1];  
          command_result_ready[command_in_progress_address] = 1;
          command_in_progress = 0;
          command_in_progress_address = 255;  

        }
        break;
      case SEI_BUS_A2_ENCODER_CHANGE_RESOLUTION:
        if (awaiting_response_bytes == 0){ 
          #ifdef DEBUG_SEI
          Serial.print(F("SEIbus::service: SEI_BUS_A2_ENCODER_CHANGE_RESOLUTION result raw_bytes: "));
          //Serial.print(raw_result_number_of_bytes);
          for (x = 0;x < raw_result_number_of_bytes;x++){
            Serial.print(" ");
            Serial.print(raw_result_byte[x]);
          }
          Serial.println();
          #endif //DEBUG_SEI  
          command_result_ready[command_in_progress_address] = 1;
          command_in_progress = 0;
          command_in_progress_address = 255;  
        }
        break;
      case SEI_BUS_A2_ENCODER_READ_SERIAL_NUMBER:
        if (awaiting_response_bytes == 0){ 
          #ifdef DEBUG_SEI
          Serial.print(F("SEIbus::service: SEI_BUS_A2_ENCODER_READ_SERIAL_NUMBER result raw_bytes: "));
          //Serial.print(raw_result_number_of_bytes);
          for (x = 0;x < raw_result_number_of_bytes;x++){
            Serial.print(" ");
            Serial.print(raw_result_byte[x]);
          }
          Serial.println();
          #endif //DEBUG_SEI
          serial_number = (raw_result_byte[0] * 16777210) + (raw_result_byte[1] * 65536) + (raw_result_byte[2] * 256) + raw_result_byte[3]; 
          command_result_ready[command_in_progress_address] = 1;
          command_in_progress = 0;
          command_in_progress_address = 255;  

        }
        break;
      case SEI_BUS_A2_ENCODER_CHANGE_MODE_TEMPORARY:
        if (awaiting_response_bytes == 0){
          #ifdef DEBUG_SEI
          Serial.print(F("SEIbus::service: SEI_BUS_A2_ENCODER_CHANGE_MODE_TEMPORARY result raw_bytes: "));
          //Serial.print(raw_result_number_of_bytes);
          for (x = 0;x < raw_result_number_of_bytes;x++){
            Serial.print(" ");
            Serial.print(raw_result_byte[x]);
          }
          Serial.println();
          #endif //DEBUG_SEI
          command_result_ready[command_in_progress_address] = 1;
          command_in_progress = 0;
          command_in_progress_address = 255; 
        } 
        break;


      case SEI_BUS_A2_ENCODER_CHANGE_MODE_POWER_UP:
        if (awaiting_response_bytes == 0){
          #ifdef DEBUG_SEI
          Serial.print(F("SEIbus::service: SEI_BUS_A2_ENCODER_CHANGE_MODE_POWER_UP result raw_bytes: "));
          //Serial.print(raw_result_number_of_bytes);
          for (x = 0;x < raw_result_number_of_bytes;x++){
            Serial.print(" ");
            Serial.print(raw_result_byte[x]);
          }
          Serial.println();
          #endif //DEBUG_SEI
          command_result_ready[command_in_progress_address] = 1;
          command_in_progress = 0;
          command_in_progress_address = 255; 
        } 
        break;

      case SEI_BUS_A2_ENCODER_SET_ORIGIN:
        if (awaiting_response_bytes == 0){
          #ifdef DEBUG_SEI
          Serial.print(F("SEIbus::service: SEI_BUS_A2_ENCODER_SET_ORIGIN result raw_bytes: "));
          //Serial.print(raw_result_number_of_bytes);
          for (x = 0;x < raw_result_number_of_bytes;x++){
            Serial.print(" ");
            Serial.print(raw_result_byte[x]);
          }
          Serial.println();
          #endif //DEBUG_SEI
          command_result_ready[command_in_progress_address] = 1;
          command_in_progress = 0;
          command_in_progress_address = 255; 
        } 
        break;

      case SEI_BUS_A2_ENCODER_SET_ABSOLUTE_POSITION:
        if (awaiting_response_bytes == 0){
          #ifdef DEBUG_SEI
          Serial.print(F("SEIbus::service: SEI_BUS_A2_ENCODER_SET_ABSOLUTE_POSITION result raw_bytes: "));
          //Serial.print(raw_result_number_of_bytes);
          for (x = 0;x < raw_result_number_of_bytes;x++){
            Serial.print(" ");
            Serial.print(raw_result_byte[x]);
          }
          Serial.println();
          #endif //DEBUG_SEI
          command_result_ready[command_in_progress_address] = 1;
          command_in_progress = 0;
          command_in_progress_address = 255; 
        } 
        break;  


      case SEI_BUS_A2_ENCODER_READ_MODE:
        if (awaiting_response_bytes == 0){
          #ifdef DEBUG_SEI
          Serial.print(F("SEIbus::service: SEI_BUS_A2_ENCODER_READ_MODE result raw_bytes: "));
          //Serial.print(raw_result_number_of_bytes);
          for (x = 0;x < raw_result_number_of_bytes;x++){
            Serial.print(" ");
            Serial.print(raw_result_byte[x]);
          }
          Serial.println();
          #endif //DEBUG_SEI
          mode = raw_result_byte[0];
          command_result_ready[command_in_progress_address] = 1;
          command_in_progress = 0;
          command_in_progress_address = 255; 
        } 
        break; 

      case SEI_BUS_A2_ENCODER_CMD_GET_ADDRESS:
        if (awaiting_response_bytes == 0){
          #ifdef DEBUG_SEI
          Serial.print(F("SEIbus::service: SEI_BUS_A2_ENCODER_CMD_GET_ADDRESS result raw_bytes: "));
          //Serial.print(raw_result_number_of_bytes);
          for (x = 0;x < raw_result_number_of_bytes;x++){
            Serial.print(" ");
            Serial.print(raw_result_byte[x]);
          }
          Serial.println();
          #endif //DEBUG_SEI
          returned_address = raw_result_byte[0];
          command_result_ready[command_in_progress_address] = 1;
          command_in_progress = 0;
          command_in_progress_address = 255; 
        } 
        break; 

      case SEI_BUS_A2_ENCODER_CMD_ASSIGN_ADDRESS:
        if (awaiting_response_bytes == 0){
          #ifdef DEBUG_SEI
          Serial.print(F("SEIbus::service: SEI_BUS_A2_ENCODER_CMD_ASSIGN_ADDRESS result raw_bytes: "));
          //Serial.print(raw_result_number_of_bytes);
          for (x = 0;x < raw_result_number_of_bytes;x++){
            Serial.print(" ");
            Serial.print(raw_result_byte[x]);
          }
          Serial.println();
          #endif //DEBUG_SEI
          command_result_ready[command_in_progress_address] = 1;
          command_in_progress = 0;
          command_in_progress_address = 255; 
        } 
        break;

      case SEI_BUS_A2_ENCODER_RESET:
        if (awaiting_response_bytes == 0){
          #ifdef DEBUG_SEI
          Serial.print(F("SEIbus::service: SEI_BUS_A2_ENCODER_RESET result raw_bytes: "));
          //Serial.print(raw_result_number_of_bytes);
          for (x = 0;x < raw_result_number_of_bytes;x++){
            Serial.print(" ");
            Serial.print(raw_result_byte[x]);
          }
          Serial.println();
          #endif //DEBUG_SEI
          mode = raw_result_byte[0];
          command_result_ready[command_in_progress_address] = 1;
          command_in_progress = 0;
          command_in_progress_address = 255; 
        } 
        break;         

      default:
        #ifdef DEBUG_SEI 
        Serial.print("SEIbus::service: unknown command completed #: ");
        Serial.print(last_command[command_in_progress_address]);
        Serial.print(" address: ");
        Serial.println(command_in_progress_address);
        delay(1000);
        #endif //DEBUG_SEI
        break;    
    }
  }

  // check if there's a command in progress and if we should time it out
  
  if ((command_in_progress) && ((millis() - last_command_time[command_in_progress_address]) >= SEI_BUS_COMMAND_TIMEOUT_MS)){
    #ifdef DEBUG_SEI
    Serial.print(F("SEIbus::service: command timeout address: "));
    Serial.print(command_in_progress_address);
    Serial.print(F(" last_command: "));
    switch(last_command[command_in_progress_address]){
      case SEI_BUS_A2_ENCODER_CMD_WAKEUP: Serial.println("SEI_BUS_A2_ENCODER_CMD_WAKEUP");break;
      case SEI_BUS_A2_ENCODER_CMD_READ_POS: Serial.println(F("SEI_BUS_A2_ENCODER_CMD_READ_POS"));break;
      case SEI_BUS_A2_ENCODER_READ_FACTORY_INFO: Serial.println(F("SEI_BUS_A2_ENCODER_READ_FACTORY_INFO"));break;
      case SEI_BUS_A2_ENCODER_READ_RESOLUTION: Serial.println(F("SEI_BUS_A2_ENCODER_READ_RESOLUTION"));break;
      case SEI_BUS_A2_ENCODER_CHANGE_RESOLUTION: Serial.println(F("SEI_BUS_A2_ENCODER_CHANGE_RESOLUTION"));break;
      case SEI_BUS_A2_ENCODER_CHANGE_MODE_TEMPORARY: Serial.println(F("SEI_BUS_A2_ENCODER_CHANGE_MODE_TEMPORARY"));break;
      case SEI_BUS_A2_ENCODER_READ_SERIAL_NUMBER: Serial.println(F("SEI_BUS_A2_ENCODER_READ_SERIAL_NUMBER"));break;
      case SEI_BUS_A2_ENCODER_SET_ABSOLUTE_POSITION: Serial.println(F("SEI_BUS_A2_ENCODER_SET_ABSOLUTE_POSITION"));break;
      case SEI_BUS_A2_ENCODER_SET_ORIGIN: Serial.println(F("SEI_BUS_A2_ENCODER_SET_ORIGIN"));break;
      case SEI_BUS_A2_ENCODER_READ_MODE: Serial.println(F("SEI_BUS_A2_ENCODER_READ_MODE"));break;
      case SEI_BUS_A2_ENCODER_CHANGE_MODE_POWER_UP: Serial.println(F("SEI_BUS_A2_ENCODER_CHANGE_MODE_POWER_UP"));break;
      case SEI_BUS_A2_ENCODER_RESET: Serial.println(F("SEI_BUS_A2_ENCODER_RESET"));break;
      case SEI_BUS_A2_ENCODER_CMD_READ_POS_STATUS: Serial.println(F("SEI_BUS_A2_ENCODER_CMD_READ_POS_STATUS"));break;
      case SEI_BUS_A2_ENCODER_CMD_READ_POS_TIME_STATUS: Serial.println(F("SEI_BUS_A2_ENCODER_CMD_READ_POS_TIME_STATUS"));break;
      case SEI_BUS_A2_ENCODER_CMD_GET_ADDRESS: Serial.println(F("SEI_BUS_A2_ENCODER_CMD_GET_ADDRESS"));break;
      case SEI_BUS_A2_ENCODER_CMD_ASSIGN_ADDRESS: Serial.println(F("SEI_BUS_A2_ENCODER_CMD_ASSIGN_ADDRESS"));break;
      default: Serial.println(F("unknown")); break;
    }
    #endif //DEBUG_SEI    
    clear_active_command(1);
  }  



}


//-----------------------------------------------------------------------------------------------------
void SEIbus::send_one_byte_command(uint8_t address, uint8_t _awaiting_response_bytes, uint8_t _command){

flush_incoming_buffer();

  send_port_byte(_command | address);
  command_in_progress = 1;
  command_in_progress_address = address;
  command_result_ready[command_in_progress_address] = 0;
  awaiting_response_bytes = _awaiting_response_bytes;
  last_command_time[address] = millis();
  timeout[address] = 0;    
  raw_result_number_of_bytes = 0;
  last_command[address] = _command;

}

//-----------------------------------------------------------------------------------------------------
void SEIbus::send_two_byte_command(uint8_t address, uint8_t _awaiting_response_bytes, uint8_t _command){

flush_incoming_buffer();

  send_port_byte(0xF0 | address);
  send_port_byte(_command);
  command_in_progress = 1;
  command_in_progress_address = address;
  command_result_ready[command_in_progress_address] = 0;
  awaiting_response_bytes = _awaiting_response_bytes;
  last_command_time[address] = millis();
  timeout[address] = 0;    
  raw_result_number_of_bytes = 0;
  last_command[address] = _command;

}

//-----------------------------------------------------------------------------------------------------

uint8_t SEIbus::a2_encoder_read_position(uint8_t address){

  if ((!command_in_progress) && (address < 0x10)){
    send_one_byte_command(address,4,SEI_BUS_A2_ENCODER_CMD_READ_POS);
  } else {
  	return(0);
  }

  return(1);
}

//-----------------------------------------------------------------------------------------------------

uint8_t SEIbus::a2_encoder_read_position_and_status(uint8_t address){

  if ((!command_in_progress) && (address < 0x10)){
    send_one_byte_command(address,5,SEI_BUS_A2_ENCODER_CMD_READ_POS_STATUS);
  } else {
    return(0);
  }

  return(1);
}



//-----------------------------------------------------------------------------------------------------

uint8_t SEIbus::a2_encoder_read_position_and_time_and_status(uint8_t address){

  if ((!command_in_progress) && (address < 0x10)){
    send_one_byte_command(address,5,SEI_BUS_A2_ENCODER_CMD_READ_POS_TIME_STATUS);
  } else {
    return(0);
  }

  return(1);
}

//-----------------------------------------------------------------------------------------------------

uint8_t SEIbus::a2_encoder_read_serial_number(uint8_t address){

  if ((!command_in_progress) && (address < 0x10)){
    send_two_byte_command(address,4+CHECKSUM_BYTES,SEI_BUS_A2_ENCODER_READ_SERIAL_NUMBER);
  } else {
    return(0);
  }

  return(1);



}

//-----------------------------------------------------------------------------------------------------

uint8_t SEIbus::a2_encoder_read_factory_info(uint8_t address){

  if ((!command_in_progress) && (address < 0x10)){
    send_two_byte_command(address,14+CHECKSUM_BYTES,SEI_BUS_A2_ENCODER_READ_FACTORY_INFO);
  } else {
    return(0);
  }

  return(1);



}

//-----------------------------------------------------------------------------------------------------

uint8_t SEIbus::a2_encoder_read_resolution(uint8_t address){

  if ((!command_in_progress) && (address < 0x10)){
    send_two_byte_command(address,2+CHECKSUM_BYTES,SEI_BUS_A2_ENCODER_READ_RESOLUTION);
  } else {
    return(0);
  }

  return(1);

}




//-----------------------------------------------------------------------------------------------------

uint8_t SEIbus::a2_encoder_reset(uint8_t address){

  if ((!command_in_progress) && (address < 0x10)){
    send_two_byte_command(address,CHECKSUM_BYTES,SEI_BUS_A2_ENCODER_RESET);
  } else {
    return(0);
  }

  return(1);

}

//-----------------------------------------------------------------------------------------------------
uint8_t SEIbus::a2_encoder_change_resolution(uint8_t address,unsigned int resolution){

  uint8_t byte_to_send = 0;

  if ((!command_in_progress) && (address < 0x10)){
    flush_incoming_buffer();
    send_port_byte(0xF0 | address);
    send_port_byte(SEI_BUS_A2_ENCODER_CHANGE_RESOLUTION);
    byte_to_send = highByte(resolution);
    send_port_byte(byte_to_send);
    byte_to_send = lowByte(resolution);
    send_port_byte(byte_to_send);
    command_in_progress = 1;
    command_in_progress_address = address;
    command_result_ready[command_in_progress_address] = 0;
    awaiting_response_bytes = CHECKSUM_BYTES;
    last_command_time[address] = millis();
    timeout[address] = 0;    
    raw_result_number_of_bytes = 0;
    last_command[address] = SEI_BUS_A2_ENCODER_CHANGE_RESOLUTION;
  } else {
    return(0);
  }

  return(1);
}
//-----------------------------------------------------------------------------------------------------
uint8_t SEIbus::a2_encoder_change_mode_temporary(uint8_t address,uint8_t mode){

  if ((!command_in_progress) && (address < 0x10)){
    flush_incoming_buffer();
    send_port_byte(0xF0 | address);
    send_port_byte(SEI_BUS_A2_ENCODER_CHANGE_MODE_TEMPORARY);
    send_port_byte(mode);
    command_in_progress = 1;
    command_in_progress_address = address;
    command_result_ready[command_in_progress_address] = 0;
    awaiting_response_bytes = CHECKSUM_BYTES;
    last_command_time[address] = millis();
    timeout[address] = 0;    
    raw_result_number_of_bytes = 0;
    last_command[address] = SEI_BUS_A2_ENCODER_CHANGE_MODE_TEMPORARY;
  } else {
    return(0);
  }

  return(1);


}




//-----------------------------------------------------------------------------------------------------
uint8_t SEIbus::a2_encoder_change_mode_power_up(uint8_t address,uint8_t mode){

  if ((!command_in_progress) && (address < 0x10)){
    flush_incoming_buffer();
    send_port_byte(0xF0 | address);
    send_port_byte(SEI_BUS_A2_ENCODER_CHANGE_MODE_POWER_UP);
    send_port_byte(mode);
    command_in_progress = 1;
    command_in_progress_address = address;
    command_result_ready[command_in_progress_address] = 0;
    awaiting_response_bytes = CHECKSUM_BYTES;
    last_command_time[address] = millis();
    timeout[address] = 0;    
    raw_result_number_of_bytes = 0;
    last_command[address] = SEI_BUS_A2_ENCODER_CHANGE_MODE_POWER_UP;
  } else {
    return(0);
  }

  return(1);


}

//-----------------------------------------------------------------------------------------------------
uint8_t SEIbus::a2_encoder_set_absolute_position_single_turn(uint8_t address,unsigned int position){

  uint8_t byte_to_send = 0;

  if ((!command_in_progress) && (address < 0x10)){
    flush_incoming_buffer();
    send_port_byte(0xF0 | address);
    send_port_byte(SEI_BUS_A2_ENCODER_SET_ABSOLUTE_POSITION);
    byte_to_send = highByte(position);
    send_port_byte(byte_to_send);
    byte_to_send = lowByte(position);
    send_port_byte(byte_to_send);
    command_in_progress = 1;
    command_in_progress_address = address;
    command_result_ready[command_in_progress_address] = 0;
    awaiting_response_bytes = CHECKSUM_BYTES;
    last_command_time[address] = millis();
    timeout[address] = 0;    
    raw_result_number_of_bytes = 0;
    last_command[address] = SEI_BUS_A2_ENCODER_SET_ABSOLUTE_POSITION;
  } else {
    return(0);
  }

  return(1);


}

//-----------------------------------------------------------------------------------------------------
uint8_t SEIbus::a2_encoder_set_absolute_position_multi_turn(uint8_t address,unsigned long position){

  uint8_t byte_to_send = 0;

  if ((!command_in_progress) && (address < 0x10)){
    flush_incoming_buffer();
    send_port_byte(0xF0 | address);
    send_port_byte(SEI_BUS_A2_ENCODER_SET_ABSOLUTE_POSITION);

    byte_to_send = (position & 0xff000000UL) >> 24;
    send_port_byte(byte_to_send);
    byte_to_send = (position & 0x00ff0000UL) >> 16;
    send_port_byte(byte_to_send);
    byte_to_send = (position & 0x0000ff00UL) >>  8;
    send_port_byte(byte_to_send);
    byte_to_send = (position & 0x000000ffUL);
    send_port_byte(byte_to_send);

    command_in_progress = 1;
    command_in_progress_address = address;
    command_result_ready[command_in_progress_address] = 0;
    awaiting_response_bytes = CHECKSUM_BYTES;
    last_command_time[address] = millis();
    timeout[address] = 0;    
    raw_result_number_of_bytes = 0;
    last_command[address] = SEI_BUS_A2_ENCODER_SET_ABSOLUTE_POSITION;
  } else {
    return(0);
  }

  return(1);


}


//-----------------------------------------------------------------------------------------------------
uint8_t SEIbus::a2_encoder_set_origin(uint8_t address){

  if ((!command_in_progress) && (address < 0x10)){
    send_two_byte_command(address,CHECKSUM_BYTES,SEI_BUS_A2_ENCODER_SET_ORIGIN);
  } else {
    return(0);
  }

  return(1);


}

//-----------------------------------------------------------------------------------------------------
uint8_t SEIbus::a2_encoder_read_mode(uint8_t address){

  if ((!command_in_progress) && (address < 0x10)){
    send_two_byte_command(address,1+CHECKSUM_BYTES,SEI_BUS_A2_ENCODER_READ_MODE);
  } else {
    return(0);
  }

  return(1);


}

//-----------------------------------------------------------------------------------------------------
uint8_t SEIbus::a2_encoder_strobe(uint8_t address){

  if ((!command_in_progress) && (address < 0x10)){
    flush_incoming_buffer();
    send_port_byte(SEI_BUS_A2_ENCODER_CMD_STROBE|address);
    command_in_progress = 0;  // no need to have the command in progress
    command_in_progress_address = 0;
    command_result_ready[command_in_progress_address] = 0;
    awaiting_response_bytes = 0;
    last_command_time[address] = millis();
    last_command[address] = SEI_BUS_A2_ENCODER_CMD_STROBE;
    timeout[address] = 0;
    raw_result_number_of_bytes = 0;
  } else {
  	return(0);
  }

  return(1);
}

//-----------------------------------------------------------------------------------------------------

uint8_t SEIbus::a2_encoder_sleep(uint8_t address){

  /* this issues the sleep command */

  if ((!command_in_progress) && (address < 0x10)){
    flush_incoming_buffer();
    send_port_byte(SEI_BUS_A2_ENCODER_CMD_SLEEP|address);
    command_in_progress = 0;  // no need to have the command in progress
    command_in_progress_address = 0;
    command_result_ready[command_in_progress_address] = 0;
    awaiting_response_bytes = 0;
    last_command_time[address] = millis();
    last_command[address] = SEI_BUS_A2_ENCODER_CMD_SLEEP;
    timeout[address] = 0;
    raw_result_number_of_bytes = 0;
  } else {
  	return(0);
  }

  return(1);



}

//-----------------------------------------------------------------------------------------------------

uint8_t SEIbus::a2_encoder_wakeup(){

  if (!command_in_progress){
    flush_incoming_buffer();
    send_port_byte(SEI_BUS_A2_ENCODER_CMD_WAKEUP);
    command_in_progress = 1;
    command_in_progress_address = 0xF;
    command_result_ready[0xF] = 0;
    awaiting_response_bytes = 0;
    last_command_time[0xF] = millis();
    last_command[0xF] = SEI_BUS_A2_ENCODER_CMD_WAKEUP;
    timeout[0xF] = 0;
    raw_result_number_of_bytes = 0;
  } else {
  	return(0);
  }

  return(1);
  

}

//-----------------------------------------------------------------------------------------------------

void SEIbus::send_port_byte(uint8_t portbyte){

  if (_bus_send_receive_pin) {digitalWrite(_bus_send_receive_pin,SEI_BUS_SEND_STATE);}
  delay(PORT_SEND_RECEIVE_LEAD_TIME_MS);
  serialport->write(portbyte);
  serialport->flush();
  delay(PORT_SEND_RECEIVE_TAIL_TIME_MS);
  if (_bus_send_receive_pin) {digitalWrite(_bus_send_receive_pin,SEI_BUS_RECEIVE_STATE);}

}
//-----------------------------------------------------------------------------------------------------

void SEIbus::flush_incoming_buffer(){

  unsigned long entry_time = millis();

  while((millis() - entry_time) < FLUSH_BUFFER_MS){
    if (serialport->available()){
      serialport->read();
      serialport->flush();      
    }
  }



}

//-----------------------------------------------------------------------------------------------------

uint8_t SEIbus::bus_busy(){

  if(_bus_busy_pin){
    if(digitalRead(_bus_busy_pin)) {return 1;}
  }

  return 0;

}


//-----------------------------------------------------------------------------------------------------

uint8_t SEIbus::a2_encoder_loopback_test(uint8_t address){

  int x = 0;
  unsigned long last_byte_sent_time = 0;
  uint8_t serial_byte_in = 0;

  if (!command_in_progress){
    flush_incoming_buffer();
    send_port_byte(0xF0|address);
    send_port_byte(SEI_BUS_A2_ENCODER_LOOPBACK_MODE);
    for (x = 0; x < 256; x++){
      send_port_byte(x);
      last_byte_sent_time = millis();
      while ((serialport->available() == 0) && ((millis() - last_byte_sent_time) < 100)) {}  // wait until we get a byte back or timeout
      if (serialport->available()){
        serial_byte_in = serialport->read();
        if (serial_byte_in != x){
          Serial.print("SEIbus::a2_encoder_loopback_test: echo byte mismatch; sent: ");
          Serial.print(x);
          Serial.print(" received: ");
          Serial.println(serial_byte_in);
          delay(360);
          return(0);          
        }
      } else {
        // no byte received
        Serial.print("SEIbus::a2_encoder_loopback_test: echo byte not received: ");
        Serial.println(x);
        delay(360);
        return(0);
      }

    }
    delay(360);

  } else {
    return(0);
  }

  return(1);  


}

//-----------------------------------------------------------------------------------------------------

uint8_t SEIbus::a2_encoder_change_baud_rate(uint8_t address, uint8_t baud){

  unsigned long last_byte_sent_time = 0;

  if (!command_in_progress){
    flush_incoming_buffer();
    send_port_byte(0xF0|address);
    send_port_byte(SEI_BUS_A2_ENCODER_CHANGE_BAUD_RATE);
    send_port_byte(baud);
    last_byte_sent_time = millis();
    while ((serialport->available() == 0) && ((millis() - last_byte_sent_time) < 100)) {}  // wait until we get a byte back or timeout
    if (serialport->available()){
      serialport->read();
      current_baud_rate = baud;
      serialport->end();
      switch(baud){
        case BAUD_115200: serialport->begin(115200); break;
        case BAUD_57600: serialport->begin(57600); break;
        case BAUD_38400: serialport->begin(38400); break;
        case BAUD_19200: serialport->begin(19200); break;
        case BAUD_9600: serialport->begin(9600); break;
        case BAUD_4800: serialport->begin(4800); break;
        case BAUD_2400: serialport->begin(2400); break;
        case BAUD_1200: serialport->begin(1200); break;
      }         
    } else {
      // no byte received
      return(0);
    }
  } else {
    return(0);
  }

  return(1);

}

//-----------------------------------------------------------------------------------------------------
uint8_t SEIbus::a2_encoder_get_address(uint8_t address,unsigned long serial_number){

  uint8_t byte_to_send = 0;

  if ((!command_in_progress) && (address < 0x10)){
    flush_incoming_buffer();
    send_port_byte(0xF0 | address);
    send_port_byte(SEI_BUS_A2_ENCODER_CMD_GET_ADDRESS);

    byte_to_send = (serial_number & 0xff000000UL) >> 24;
    send_port_byte(byte_to_send);
    byte_to_send = (serial_number & 0x00ff0000UL) >> 16;
    send_port_byte(byte_to_send);
    byte_to_send = (serial_number & 0x0000ff00UL) >>  8;
    send_port_byte(byte_to_send);
    byte_to_send = (serial_number & 0x000000ffUL);
    send_port_byte(byte_to_send);

    command_in_progress = 1;
    command_in_progress_address = address;
    command_result_ready[command_in_progress_address] = 0;
    awaiting_response_bytes = 1+CHECKSUM_BYTES;
    last_command_time[address] = millis();
    timeout[address] = 0;    
    raw_result_number_of_bytes = 0;
    last_command[address] = SEI_BUS_A2_ENCODER_CMD_GET_ADDRESS;
  } else {
    return(0);
  }

  return(1);

}
//-----------------------------------------------------------------------------------------------------
uint8_t SEIbus::a2_encoder_assign_address(uint8_t address,unsigned long serial_number, uint8_t new_address){


  uint8_t byte_to_send = 0;

  if ((!command_in_progress) && (address < 0x10)){
    flush_incoming_buffer();
    send_port_byte(0xF0 | address);
    send_port_byte(SEI_BUS_A2_ENCODER_CMD_ASSIGN_ADDRESS);

    byte_to_send = (serial_number & 0xff000000UL) >> 24;
    send_port_byte(byte_to_send);
    byte_to_send = (serial_number & 0x00ff0000UL) >> 16;
    send_port_byte(byte_to_send);
    byte_to_send = (serial_number & 0x0000ff00UL) >>  8;
    send_port_byte(byte_to_send);
    byte_to_send = (serial_number & 0x000000ffUL);
    send_port_byte(byte_to_send);

    send_port_byte(new_address);

    command_in_progress = 1;
    command_in_progress_address = address;
    command_result_ready[command_in_progress_address] = 0;
    awaiting_response_bytes = CHECKSUM_BYTES;
    last_command_time[address] = millis();
    timeout[address] = 0;    
    raw_result_number_of_bytes = 0;
    last_command[address] = SEI_BUS_A2_ENCODER_CMD_ASSIGN_ADDRESS;
  } else {
    return(0);
  }

  return(1);

}
//-----------------------------------------------------------------------------------------------------

#endif //sei_bus_h


