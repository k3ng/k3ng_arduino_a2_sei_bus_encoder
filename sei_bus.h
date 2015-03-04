

//#define DEBUG_SEI
//#define DEBUG_SEI_SERIAL_PORT

//encoder error codes
#define SEI_BUS_NO_ERROR 0
#define SEI_BUS_NOT_ENOUGH_LIGHT 1
#define SEI_BUS_TOO_MUCH_LIGHT 2
#define SEI_BUS_MISALIGNMENT_OR_DUST_1 3
#define SEI_BUS_MISALIGNMENT_OR_DUST_2 4
#define SEI_BUS_MISALIGNMENT_OR_DUST_3 5
#define SEI_BUS_HARDWARE_PROBLEM 6
#define SEI_BUS_FAST_MODE_ERROR 7
#define SEI_BUS_MULTITURN_POS_NOT_INITIALIZED 8

#define SEI_BUS_A2_ENCODER_SET_ORIGIN 0x01
#define SEI_BUS_A2_ENCODER_SET_ABSOLUTE_POSITION 0x02
#define SEI_BUS_A2_ENCODER_READ_SERIAL_NUMBER 0x03
#define SEI_BUS_A2_ENCODER_READ_FACTORY_INFO 0x08
#define SEI_BUS_A2_ENCODER_READ_RESOLUTION 0x09
#define SEI_BUS_A2_ENCODER_CHANGE_RESOLUTION 0x0A
#define SEI_BUS_A2_ENCODER_CHANGE_MODE_TEMPORARY 0x0C

#define SEI_BUS_A2_ENCODER_CMD_READ_POS 0x10
#define SEI_BUS_A2_ENCODER_CMD_STROBE 0x40
#define SEI_BUS_A2_ENCODER_CMD_SLEEP 0x50
#define SEI_BUS_A2_ENCODER_CMD_WAKEUP 0x60

#define SEI_BUS_A2_ENCODER_CMD_READ_POS_STATUS 92
#define SEI_BUS_A2_ENCODER_CMD_READ_POS_TIME_STATUS 93



/* change mode command bits */
#define MODE_REVERSE           B00000001
#define MODE_STROBE            B00000010
#define MODE_MULTITURN         B00000100
#define MODE_TWO_BYTE_POSITION B00001000
#define MODE_INCREMENTAL       B00010000
#define MODE_DIVIDE_BY_256     B01000000




//settings
#define SEI_BUS_COMMAND_TIMEOUT_MS 2000
#define SEI_BUS_A2_ENCODER_CMD_READ_POS_TIMEOUT_MS 1000
#define SEI_BUS_RECEIVE_BYTES_BUFFER 30
#define SEI_BUS_WAKEUP_TIME_MS 5

#define SEI_BUS_SEND_STATE HIGH
#define SEI_BUS_RECEIVE_STATE LOW

#define CHECKSUM_BYTES 1
#define FLUSH_BUFFER_MS 50
#define PORT_SEND_RECEIVE_LEAD_TIME_MS 0
#define PORT_SEND_RECEIVE_TAIL_TIME_MS 0

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
   uint8_t a2_encoder_get_address(unsigned long serial_number);
   uint8_t a2_encoder_assign_address(unsigned long serial_number,uint8_t address);
   uint8_t a2_encoder_read_factory_info(uint8_t address);
   uint8_t a2_encoder_read_resolution(uint8_t address);
   uint8_t a2_encoder_change_resolution(uint8_t address,unsigned int resolution);
   uint8_t a2_encoder_read_mode(uint8_t address);
   uint8_t a2_encoder_change_mode_temporary(uint8_t address,uint8_t mode);
   uint8_t a2_encoder_change_mode_power_up(uint8_t address,uint8_t mode);
   uint8_t a2_encoder_reset(uint8_t address);
   void send_port_byte(uint8_t portbyte);
   uint8_t bus_busy();

   uint8_t command_in_progress;         // current command in progress (0 = no command in progress)
   uint8_t command_in_progress_address; // the address of the device with a current command in progress
   uint8_t awaiting_response_bytes;     // the number of bytes we're waiting to receive in response to a command
   unsigned long last_command_time[18];
   uint8_t last_command[18];
   uint8_t timeout[18];     // 1 = last command timed out
   uint8_t command_result_ready[18];     // 1 = there is a result ready from the last command
   uint8_t raw_result_number_of_bytes;
   uint8_t raw_result_byte[SEI_BUS_RECEIVE_BYTES_BUFFER];
   

   /* query results */

   /* SEI_BUS_A2_ENCODER_CMD_READ_POS */
   unsigned long position;


   /* SEI_BUS_A2_ENCODER_READ_FACTORY_INFO */
   unsigned int model_number;
   float version_number;
   uint8_t configuration_byte_1;
   uint8_t configuration_byte_2;
   unsigned long serial_number;
   uint8_t month;
   uint8_t day;
   unsigned int year;

   /* SEI_BUS_A2_ENCODER_READ_RESOLUTION  */
   unsigned int resolution;

   uint8_t status_nibble[];

private:
   void clear_active_command(uint8_t timeout_status);
   void send_one_byte_command(uint8_t address, uint8_t _awaiting_response_bytes, uint8_t _command);
   void send_two_byte_command(uint8_t address, uint8_t _awaiting_response_bytes, uint8_t _command);
   void flush_incoming_buffer();


};