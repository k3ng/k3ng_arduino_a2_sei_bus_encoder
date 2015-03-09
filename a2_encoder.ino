/* 

  Example Code

  A2 SEI Bus Encoder Arduino Library

  Anthony Good, K3NG
  anthony.good@gmail.com
  http://blog.radioartisan.com/

  Project page: http://blog.radioartisan.com/arduino-a2-encoder-library/

  Code repository: https://github.com/k3ng/k3ng_arduino_a2_sei_bus_encoder
  

*/

#define pin_sei_bus_busy 24
#define pin_sei_bus_send_receive 22

#include "sei_bus.h"

#define RESOLUTION 32767 //36000
#define ENCODER_ADDRESS 0x00


SEIbus SEIbus1(&Serial1,9600,pin_sei_bus_busy,pin_sei_bus_send_receive);
//             (Serial Port,Baud Rate,Busy Pin,Send/Receive Pin)

unsigned long last_bus_busy_print_time = 0;

unsigned long last_comand_submit_time = 0;

byte submitted_command = 0;
byte executed_reset = 0;
byte executed_factory_read_info = 0;
byte executed_read_serial_number = 0;
byte executed_read_resolution = 0;
byte executed_change_resolution = 0;
byte executed_change_mode_temporary = 0;
byte executed_change_mode_power_up = 0;
byte executed_set_absolute_position_single_turn = 0;
byte executed_set_absolute_position_multi_turn = 0;
byte executed_set_origin = 0;
byte executed_read_mode = 0;
byte executed_read_position_and_status = 0;
byte executed_read_position_and_time_and_status = 0;

float normalized_position = 0;


void setup() {

  delay(500);
  Serial.begin(9600);
  Serial.print("K3NG A2 Encoder Library Version ");
  Serial.println(SEI_BUS_A2_LIBRARY_VERSION);
  Serial.println("A2 Encoder Test Code Starting...");
  delay(100);
  SEIbus1.initialize();
  Serial.println("Done initializing...");
  delay(250);

  /*
  // do a baud rate change (This is not stored in encoder EEPROM.)
  Serial.print("Changing baud rate to 115200...");
  if (SEIbus1.a2_encoder_change_baud_rate(ENCODER_ADDRESS,BAUD_115200)){
    Serial.println("successful!");
  } else {
    Serial.println("failed :-(");
  }
  delay(250);
  */

  Serial.print("Starting loopback test...");
  if (SEIbus1.a2_encoder_loopback_test(ENCODER_ADDRESS)){
    Serial.println("completed successfully!");
  } else {
    Serial.println("failed!");
  }
  delay(500);
  
}

void loop() {

  // service the SEI bus
  SEIbus1.service();

/*
  // execute a reset(1 time)
  if ((!executed_reset) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_reset(ENCODER_ADDRESS)){
      submitted_command = 1;
      executed_reset = 1;
    } else {
      Serial.println("a2_encoder_reset unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }
*/

  // execute a change resolution command (1 time)
  if ((!executed_change_resolution) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_change_resolution(ENCODER_ADDRESS,RESOLUTION)){
      submitted_command = 1;
      executed_change_resolution = 1;
    } else {
      Serial.println("a2_encoder_change_resolution unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }

  // execute a read resolution command (1 time)
  if ((!executed_read_resolution) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_read_resolution(ENCODER_ADDRESS)){
      submitted_command = 1;
      executed_read_resolution = 1;
    } else {
      Serial.println("a2_encoder_read_resolution unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }

  // execute a read serial number command (1 time)
  if ((!executed_read_serial_number) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_read_serial_number(ENCODER_ADDRESS)){
      submitted_command = 1;
      executed_read_serial_number = 1;
    } else {
      Serial.println("a2_encoder_read_serial_number unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }

  // execute a read factory info command (1 time)
  if ((!executed_factory_read_info) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_read_factory_info(ENCODER_ADDRESS)){
      submitted_command = 1;
      executed_factory_read_info = 1;
    } else {
      Serial.println("a2_encoder_read_factory_info unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }

/*
  // execute a set absolute position to 0 degrees command (1 time)
  if ((!executed_set_absolute_position_single_turn) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_set_absolute_position_single_turn(ENCODER_ADDRESS,0)){
      submitted_command = 1;
      executed_set_absolute_position_single_turn = 1;
    } else {
      Serial.println("a2_encoder_set_absolute_position_single_turn unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }
*/

/*
  // execute a change mode temporarily command (1 time)
  if ((!executed_change_mode_temporary) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_change_mode_temporary(ENCODER_ADDRESS,MODE_TWO_BYTE_POSITION|MODE_MULTITURN)){
      submitted_command = 1;
      executed_change_mode_temporary = 1;
    } else {
      Serial.println("a2_encoder_change_mode_temporary unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }
*/


  // execute a change mode power up command (1 time)
  if ((!executed_change_mode_power_up) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_change_mode_power_up(ENCODER_ADDRESS,/*MODE_TWO_BYTE_POSITION|*/MODE_MULTITURN)){
      submitted_command = 1;
      executed_change_mode_power_up = 1;
    } else {
      Serial.println("a2_encoder_change_mode_power_up unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }


  // execute a set absolute position (multi turn mode) to 0 degrees command (1 time)
  if ((!executed_set_absolute_position_multi_turn) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_set_absolute_position_multi_turn(ENCODER_ADDRESS,0)){
      submitted_command = 1;
      executed_set_absolute_position_multi_turn = 1;
    } else {
      Serial.println("a2_encoder_set_absolute_position_multi_turn unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }

 
/*

  // execute a set origin command (1 time)
  if ((!executed_set_origin) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_set_origin(ENCODER_ADDRESS)){
      submitted_command = 1;
      executed_set_origin = 1;
    } else {
      Serial.println("a2_encoder_set_origin unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }

*/



  // execute a read mode command (1 time)
  if ((!executed_read_mode) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_read_mode(ENCODER_ADDRESS)){
      submitted_command = 1;
      executed_read_mode = 1;
    } else {
      Serial.println("a2_encoder_read_mode unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }

/*
  // execute a read position and status command (1 time)
  if ((!executed_read_position_and_status) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_read_position_and_status(ENCODER_ADDRESS)){
      submitted_command = 1;
      executed_read_position_and_status = 1;
    } else {
      Serial.println("a2_encoder_read_position_and_status unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }
*/

  // execute a read position and time and status command (1 time)
  if ((!executed_read_position_and_time_and_status) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_read_position_and_time_and_status(ENCODER_ADDRESS)){
      submitted_command = 1;
      executed_read_position_and_time_and_status = 1;
    } else {
      Serial.println("a2_encoder_read_position_and_time_and_status unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }

  // read position (every 250 mS)
  if (((millis() - last_comand_submit_time) >= 250) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_read_position(ENCODER_ADDRESS)){
      submitted_command = 1;
    } else {
      Serial.println("a2_encoder_read_position unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }



  // if there are command results available, process them
  if ((SEIbus1.command_result_ready[0] == 1) && (submitted_command)){
    switch(SEIbus1.last_command[0]){
      case SEI_BUS_A2_ENCODER_READ_FACTORY_INFO:
        Serial.print("Model:");
        Serial.print(SEIbus1.model_number);
        Serial.print(" Version:");
        Serial.print(SEIbus1.version_number);
        Serial.print(" Serial:");
        Serial.print(SEIbus1.serial_number);
        Serial.print(" ");
        Serial.print(SEIbus1.year);
        Serial.print("-");
        Serial.print(SEIbus1.month);
        Serial.print("-");
        Serial.println(SEIbus1.day);    
        break;
      case SEI_BUS_A2_ENCODER_CMD_READ_POS_TIME_STATUS:
        Serial.print("Time: ");
        Serial.println(SEIbus1.time);
      case SEI_BUS_A2_ENCODER_CMD_READ_POS_STATUS:
        Serial.print("Status: ");
        switch(SEIbus1.status & B11110000){
          case STATUS_NO_ERROR: Serial.println("OK"); break;
          case STATUS_NOT_ENOUGH_LIGHT: Serial.println("NOT_ENOUGH_LIGHT"); break;
          case STATUS_TOO_MUCH_LIGHT: Serial.println("TOO_MUCH_LIGHT"); break;
          case STATUS_MISALIGNMENT_OR_DUST_1: Serial.println("MISALIGNMENT_OR_DUST_1"); break;
          case STATUS_MISALIGNMENT_OR_DUST_2: Serial.println("MISALIGNMENT_OR_DUST_2"); break;
          case STATUS_MISALIGNMENT_OR_DUST_3: Serial.println("MISALIGNMENT_OR_DUST_3"); break;
          case STATUS_HARDWARE_PROBLEM: Serial.println("HARDWARE_PROBLEM"); break;
          case STATUS_FAST_MODE_ERROR: Serial.println("FAST_MODE_ERROR"); break;
          case STATUS_MULTITURN_NOT_INIT: Serial.println("MULTITURN_NOT_INIT"); break;
        }
      case SEI_BUS_A2_ENCODER_CMD_READ_POS:
        Serial.print("Position Raw: ");
        Serial.print(SEIbus1.position);
        Serial.print("\tNormalized: ");
        normalized_position = (SEIbus1.position/(float(RESOLUTION)/360.0));
        Serial.print(normalized_position);
        Serial.print("\tRollover Compensated: ");
        normalized_position = (SEIbus1.position_rollover_compensated/(float(RESOLUTION)/360.0));
        Serial.println(normalized_position);
        break;
      case SEI_BUS_A2_ENCODER_READ_RESOLUTION:
        Serial.print("Resolution: ");
        Serial.println(SEIbus1.resolution);
        break;
      case SEI_BUS_A2_ENCODER_CHANGE_RESOLUTION:
        Serial.println("Resolution set.");
        break;      
      case SEI_BUS_A2_ENCODER_READ_SERIAL_NUMBER:
        Serial.print("Serial number is ");
        Serial.println(SEIbus1.serial_number);
        break;      
      case SEI_BUS_A2_ENCODER_SET_ABSOLUTE_POSITION:
        Serial.println("Set absolute position.");
        break;
      case SEI_BUS_A2_ENCODER_SET_ORIGIN:
        Serial.println("Set origin executed.");
        break;
      case SEI_BUS_A2_ENCODER_CHANGE_MODE_TEMPORARY:
        Serial.println("Changed mode temporarily.");
        break;
      case SEI_BUS_A2_ENCODER_CHANGE_MODE_POWER_UP:
        Serial.println("Changed power up mode.");
        break;      
      case SEI_BUS_A2_ENCODER_READ_MODE:
        Serial.print("Modes set: ");
        if (SEIbus1.mode & MODE_REVERSE){Serial.print("MODE_REVERSE ");}
        if (SEIbus1.mode & MODE_STROBE){Serial.print("MODE_STROBE ");}
        if (SEIbus1.mode & MODE_MULTITURN){Serial.print("MODE_MULTITURN ");}
        if (SEIbus1.mode & MODE_TWO_BYTE_POSITION){Serial.print("MODE_TWO_BYTE_POSITION ");}
        if (SEIbus1.mode & MODE_INCREMENTAL){Serial.print("MODE_INCREMENTAL ");}
        if (SEIbus1.mode & MODE_DIVIDE_BY_256){Serial.print("MODE_DIVIDE_BY_256 ");}
        Serial.println();
        break;
      case SEI_BUS_A2_ENCODER_RESET:
        Serial.println("Completed reset.");
        break;
      default:
        Serial.println("Unknown command completed.");
        break;
    }

    submitted_command = 0;
  }

  // if a command has been in progress for 6 seconds with no result, give up on the command
  if (((millis() - last_comand_submit_time) > 6000) && (submitted_command)) {submitted_command = 0;}

}

