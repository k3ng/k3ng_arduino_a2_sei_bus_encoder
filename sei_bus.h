/* 


  A2 SEI Bus Encoder Arduino Library

  Version 1.0.2015030901

  Anthony Good, K3NG
  anthony.good@gmail.com
  http://blog.radioartisan.com/

  Project page: http://blog.radioartisan.com/arduino-a2-encoder-library/

  Code repository: https://github.com/k3ng/k3ng_arduino_a2_sei_bus_encoder



*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

//#define DEBUG_SEI
//#define DEBUG_SEI_SERIAL_PORT

#define SEI_BUS_A2_LIBRARY_VERSION "1.0.2015032801"

#define SEI_BUS_A2_ENCODER_SET_ORIGIN 0x01
#define SEI_BUS_A2_ENCODER_SET_ABSOLUTE_POSITION 0x02
#define SEI_BUS_A2_ENCODER_READ_SERIAL_NUMBER 0x03
#define SEI_BUS_A2_ENCODER_CMD_GET_ADDRESS 0x06
#define SEI_BUS_A2_ENCODER_CMD_ASSIGN_ADDRESS 0x07
#define SEI_BUS_A2_ENCODER_READ_FACTORY_INFO 0x08
#define SEI_BUS_A2_ENCODER_READ_RESOLUTION 0x09
#define SEI_BUS_A2_ENCODER_CHANGE_RESOLUTION 0x0A
#define SEI_BUS_A2_ENCODER_READ_MODE 0x0B
#define SEI_BUS_A2_ENCODER_CHANGE_MODE_TEMPORARY 0x0C
#define SEI_BUS_A2_ENCODER_CHANGE_MODE_POWER_UP 0x0D
#define SEI_BUS_A2_ENCODER_RESET 0x0E
#define SEI_BUS_A2_ENCODER_CHANGE_BAUD_RATE 0x0F
#define SEI_BUS_A2_ENCODER_LOOPBACK_MODE 0x10

#define SEI_BUS_A2_ENCODER_CMD_READ_POS 0x10
#define SEI_BUS_A2_ENCODER_CMD_READ_POS_STATUS 0x20
#define SEI_BUS_A2_ENCODER_CMD_READ_POS_TIME_STATUS 0x30
#define SEI_BUS_A2_ENCODER_CMD_STROBE 0x40
#define SEI_BUS_A2_ENCODER_CMD_SLEEP 0x50
#define SEI_BUS_A2_ENCODER_CMD_WAKEUP 0x60

/* change mode command bits */
#define MODE_REVERSE           B00000001
#define MODE_STROBE            B00000010
#define MODE_MULTITURN         B00000100
#define MODE_TWO_BYTE_POSITION B00001000
#define MODE_INCREMENTAL       B00010000
#define MODE_DIVIDE_BY_256     B01000000

/* status codes */
#define STATUS_NO_ERROR               B00000000
#define STATUS_NOT_ENOUGH_LIGHT       B00010000
#define STATUS_TOO_MUCH_LIGHT         B00100000
#define STATUS_MISALIGNMENT_OR_DUST_1 B00110000
#define STATUS_MISALIGNMENT_OR_DUST_2 B01000000
#define STATUS_MISALIGNMENT_OR_DUST_3 B01010000
#define STATUS_HARDWARE_PROBLEM       B01100000
#define STATUS_FAST_MODE_ERROR        B01110000
#define STATUS_MULTITURN_NOT_INIT     B10000000

/* baud rates */
#define BAUD_115200 0x00
#define BAUD_57600 0x01
#define BAUD_38400 0x10
#define BAUD_19200 0x11
#define BAUD_9600 0x12
#define BAUD_4800 0x13
#define BAUD_2400 0x14
#define BAUD_1200 0x15  

//user-tweakable settings
#define SEI_BUS_COMMAND_TIMEOUT_MS 2000
#define SEI_BUS_A2_ENCODER_CMD_READ_POS_TIMEOUT_MS 1000
#define SEI_BUS_RECEIVE_BYTES_BUFFER 30
#define SEI_BUS_WAKEUP_TIME_MS 5

#define SEI_BUS_SEND_STATE HIGH
#define SEI_BUS_RECEIVE_STATE LOW

#define CHECKSUM_BYTES 1
#define FLUSH_BUFFER_MS 50
#define PORT_SEND_RECEIVE_LEAD_TIME_MS 0  // only adjust this if you're having problems
#define PORT_SEND_RECEIVE_TAIL_TIME_MS 0  // only adjust this if you're having problems

class SEIbus {

public:

  SEIbus(HardwareSerial *serialportin, int baudrate = 9600, uint8_t bus_busy_pin = 0, uint8_t bus_send_receive_pin = 0);
   void initialize();
   void service();
   uint8_t a2_encoder_read_position(uint8_t address);
   uint8_t a2_encoder_read_position_and_status(uint8_t address);
   uint8_t a2_encoder_read_position_and_time_and_status(uint8_t address);
   uint8_t a2_encoder_strobe(uint8_t address);
   uint8_t a2_encoder_sleep(uint8_t address);
   uint8_t a2_encoder_wakeup();
   uint8_t a2_encoder_set_origin(uint8_t address);
   uint8_t a2_encoder_set_absolute_position_single_turn(uint8_t address,unsigned int position);
   uint8_t a2_encoder_set_absolute_position_multi_turn(uint8_t address,unsigned long position);
   uint8_t a2_encoder_read_serial_number(uint8_t address);
   uint8_t a2_encoder_read_factory_info(uint8_t address);
   uint8_t a2_encoder_read_resolution(uint8_t address);
   uint8_t a2_encoder_change_resolution(uint8_t address,unsigned int resolution);
   uint8_t a2_encoder_read_mode(uint8_t address);
   uint8_t a2_encoder_change_mode_temporary(uint8_t address,uint8_t mode);
   uint8_t a2_encoder_change_mode_power_up(uint8_t address,uint8_t mode);
   uint8_t a2_encoder_reset(uint8_t address);
   uint8_t a2_encoder_loopback_test(uint8_t address);
   uint8_t a2_encoder_change_baud_rate(uint8_t address, uint8_t baud);
   uint8_t a2_encoder_get_address(uint8_t address,unsigned long serial_number);
   uint8_t a2_encoder_assign_address(uint8_t address,unsigned long serial_number, uint8_t new_address);
   void send_port_uint8_t(uint8_t portuint8_t);
   uint8_t bus_busy();

   uint8_t command_in_progress;         // current command in progress (0 = no command in progress)
   uint8_t command_in_progress_address; // the address of the device with a current command in progress
   uint8_t awaiting_response_uint8_ts;     // the number of uint8_ts we're waiting to receive in response to a command
   unsigned long last_command_time[18];
   uint8_t last_command[18];
   uint8_t timeout[18];     // 1 = last command timed out
   uint8_t command_result_ready[18];     // 1 = there is a result ready from the last command
   uint8_t raw_result_number_of_uint8_ts;
   uint8_t raw_result_uint8_t[SEI_BUS_RECEIVE_BYTES_BUFFER];
   

   /* query results */

   /* SEI_BUS_A2_ENCODER_CMD_READ_POS, SEI_BUS_A2_ENCODER_CMD_READ_POS_STATUS, SEI_BUS_A2_ENCODER_CMD_READ_POS_TIME_STATUS */
   unsigned long position;
   float position_rollover_compensated;

   /* SEI_BUS_A2_ENCODER_READ_FACTORY_INFO */
   unsigned int model_number;
   float version_number;
   uint8_t configuration_uint8_t_1;
   uint8_t configuration_uint8_t_2;
   uint8_t month;
   uint8_t day;
   unsigned int year;

   /* SEI_BUS_A2_ENCODER_READ_SERIAL_NUMBER, SEI_BUS_A2_ENCODER_READ_FACTORY_INFO */
   unsigned long serial_number;

   /* SEI_BUS_A2_ENCODER_READ_RESOLUTION  */
   unsigned int resolution;

   /* SEI_BUS_A2_ENCODER_READ_MODE */
   uint8_t mode;

   /* SEI_BUS_A2_ENCODER_CMD_READ_POS_STATUS, SEI_BUS_A2_ENCODER_CMD_READ_POS_TIME_STATUS */
   uint8_t status;
 
   /* SEI_BUS_A2_ENCODER_CMD_READ_POS_TIME_STATUS */
   unsigned int time;

   uint8_t current_baud_rate;

   /* SEI_BUS_A2_ENCODER_CMD_GET_ADDRESS */
   uint8_t returned_address;

private:
   void clear_active_command(uint8_t timeout_status);
   void send_one_uint8_t_command(uint8_t address, uint8_t _awaiting_response_uint8_ts, uint8_t _command);
   void send_two_uint8_t_command(uint8_t address, uint8_t _awaiting_response_uint8_ts, uint8_t _command);
   void flush_incoming_buffer();


};
