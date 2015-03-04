// a2_encoder.ino

#define pin_sei_bus_busy 24
#define pin_sei_bus_send_receive 22

#include "sei_bus.h"

#define RESOLUTION 3600


SEIbus SEIbus1(&Serial1,9600,pin_sei_bus_busy,pin_sei_bus_send_receive);

unsigned long last_bus_busy_print_time = 0;

unsigned long last_comand_submit_time = 0;

byte submitted_command = 0;
byte executed_factory_read_info = 0;
byte executed_read_serial_number = 0;
byte executed_read_resolution = 0;
byte executed_change_resolution = 0;
byte executed_change_mode = 0;
byte executed_set_absolute_position_single_turn = 0;

void setup() {

  delay(500);
  Serial.begin(9600/*115200*/);
  Serial.println("A2 Encoder Test Code Starting...");
  delay(500);
  SEIbus1.initialize();
  //SEIbus1.a2_encoder_wakeup();
  Serial.println("Done initializing...");
  delay(500);


  
}

void loop() {

  // service the SEI bus
  SEIbus1.service();

  // execute a change resolution command (1 time)
  if ((!executed_change_resolution) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_change_resolution(0x00,RESOLUTION)){
      //Serial.println("a2_encoder_change_resolution submitted");
      submitted_command = 1;
      executed_change_resolution = 1;
    } else {
      Serial.println("a2_encoder_change_resolution unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }

  // execute a read resolution command (1 time)
  if ((!executed_read_resolution) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_read_resolution(0x00)){
      //Serial.println("a2_encoder_read_resolution submitted");
      submitted_command = 1;
      executed_read_resolution = 1;
    } else {
      Serial.println("a2_encoder_read_resolution unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }

  // execute a read serial number command (1 time)
  if ((!executed_read_serial_number) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_read_serial_number(0x00)){
      //Serial.println("a2_encoder_read_serial_number submitted");
      submitted_command = 1;
      executed_read_serial_number = 1;
    } else {
      Serial.println("a2_encoder_read_serial_number unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }

  // execute a read factory info command (1 time)
  if ((!executed_factory_read_info) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_read_factory_info(0x00)){
      //Serial.println("a2_encoder_read_factory_info submitted");
      submitted_command = 1;
      executed_factory_read_info = 1;
    } else {
      Serial.println("a2_encoder_read_factory_info unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }


  // execute a set absolute position to 0 degrees command (one time)
  if ((!executed_set_absolute_position_single_turn) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_set_absolute_position_single_turn(0x00,0)){
      //Serial.println("a2_encoder_read_factory_info submitted");
      submitted_command = 1;
      executed_set_absolute_position_single_turn = 1;
    } else {
      Serial.println("a2_encoder_set_absolute_position_single_turn unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }

  // execute a change mode temporarily command (1 time)
  if ((!executed_change_mode) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_change_mode_temporary(0x00,MODE_TWO_BYTE_POSITION/*|MODE_MULTITURN*/)){
      //Serial.println("a2_encoder_change_resolution submitted");
      submitted_command = 1;
      executed_change_mode = 1;
    } else {
      Serial.println("a2_encoder_change_mode_temporary unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }
  


  // read position (every 250 mS)
  if (((millis() - last_comand_submit_time) >= 250) && (SEIbus1.command_in_progress == 0) && (!submitted_command)) {
    if (SEIbus1.a2_encoder_read_position(0x00)){
      //Serial.println("a2_encoder_read_position submitted");
      submitted_command = 1;
    } else {
      Serial.println("a2_encoder_read_position unsuccesfully submitted");
    }
    last_comand_submit_time = millis();
  }



  // if there are command results read, process them
  if ((SEIbus1.command_result_ready[0] == 1) && (submitted_command)){
    switch(SEIbus1.last_command[0]){
      case SEI_BUS_A2_ENCODER_READ_FACTORY_INFO:
        Serial.print("model:");
        Serial.print(SEIbus1.model_number);
        Serial.print(" version:");
        Serial.print(SEIbus1.version_number);
        Serial.print(" serial:");
        Serial.print(SEIbus1.serial_number);
        Serial.print(" ");
        Serial.print(SEIbus1.year);
        Serial.print("-");
        Serial.print(SEIbus1.month);
        Serial.print("-");
        Serial.println(SEIbus1.day);    
        break;
      case SEI_BUS_A2_ENCODER_CMD_READ_POS:
        Serial.print("position: ");
        Serial.println(SEIbus1.position/(float(RESOLUTION)/360.0));
        break;
      case SEI_BUS_A2_ENCODER_READ_RESOLUTION:
        Serial.print("resolution: ");
        Serial.println(SEIbus1.resolution);
        break;
      case SEI_BUS_A2_ENCODER_CHANGE_RESOLUTION:
        Serial.println("Resolution set...");
        break;      
      case SEI_BUS_A2_ENCODER_READ_SERIAL_NUMBER:
        Serial.print("Serial number is ");
        Serial.println(SEIbus1.serial_number);
        break;      
      case SEI_BUS_A2_ENCODER_SET_ABSOLUTE_POSITION:
        Serial.println("Set absolute position...");
        break;

    }

    submitted_command = 0;
  }

  // if a command has been in progress for 6 seconds with no result, give up on the command
  if (((millis() - last_comand_submit_time) > 6000) && (submitted_command)) {submitted_command = 0;}

}

