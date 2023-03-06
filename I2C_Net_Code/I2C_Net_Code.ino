#include <Wire.h>
HardwareSerial &Terminal = Serial;

// Define Number of Servo Controllers
#define numSCs 1

// Comment out line to stop I2C data being printed to Terminal
#define NOTIFY_I2C_DATA

// Use an LED to show when data is transmitted
#define TRANSMIT_LED 13

// The input pins section is an example
#define INPUT_PIN_0 2 
#define INPUT_PIN_1 3 
#define INPUT_PIN_2 4
#define INPUT_PIN_3 5
#define INPUT_PIN_4 6 
#define INPUT_PIN_5 7
#define INPUT_PIN_6 8
#define INPUT_PIN_7 9
#define INPUT_PIN_8 10
#define INPUT_PIN_9 11
#define INPUT_PIN_10 12
#define INPUT_PIN_11 14

#define numInputs 12

const uint8_t Input_pin[numInputs] = {
             INPUT_PIN_0, 
             INPUT_PIN_1,
             INPUT_PIN_2,
             INPUT_PIN_3,
             INPUT_PIN_4,
             INPUT_PIN_5,
             INPUT_PIN_6,
             INPUT_PIN_7,
             INPUT_PIN_8,
             INPUT_PIN_9,
             INPUT_PIN_10,
             INPUT_PIN_11,
            };

uint16_t Input_state[numSCs] = {0b0000000000000000};          


// Define Servo Controller addresses
#define SERVO_CONTROLLER_ADDRESS_0 2
// #define SERVO_CONTROLLER_ADDRESS_1 3 
// #define SERVO_CONTROLLER_ADDRESS_2 4
// #define SERVO_CONTROLLER_ADDRESS_3 5
// #define SERVO_CONTROLLER_ADDRESS_4 6 
// #define SERVO_CONTROLLER_ADDRESS_5 7
// #define SERVO_CONTROLLER_ADDRESS_6 8
// #define SERVO_CONTROLLER_ADDRESS_7 9
// #define SERVO_CONTROLLER_ADDRESS_8 10
// #define SERVO_CONTROLLER_ADDRESS_9 11
// #define SERVO_CONTROLLER_ADDRESS_10 12
// #define SERVO_CONTROLLER_ADDRESS_11 13
// #define SERVO_CONTROLLER_ADDRESS_12 14
// #define SERVO_CONTROLLER_ADDRESS_13 15
// #define SERVO_CONTROLLER_ADDRESS_14 16
// #define SERVO_CONTROLLER_ADDRESS_15 17

const uint8_t SC12_ADDR[numSCs] = {
             SERVO_CONTROLLER_ADDRESS_0,             
            //  SERVO_CONTROLLER_ADDRESS_1,
            //  SERVO_CONTROLLER_ADDRESS_2,
            //  SERVO_CONTROLLER_ADDRESS_3,
            //  SERVO_CONTROLLER_ADDRESS_4,
            //  SERVO_CONTROLLER_ADDRESS_5,
            //  SERVO_CONTROLLER_ADDRESS_6,
            //  SERVO_CONTROLLER_ADDRESS_7,
            //  SERVO_CONTROLLER_ADDRESS_8,
            //  SERVO_CONTROLLER_ADDRESS_9,
            //  SERVO_CONTROLLER_ADDRESS_10,
            //  SERVO_CONTROLLER_ADDRESS_11,
            //  SERVO_CONTROLLER_ADDRESS_12,
            //  SERVO_CONTROLLER_ADDRESS_13,
            //  SERVO_CONTROLLER_ADDRESS_14,
            //  SERVO_CONTROLLER_ADDRESS_15,            
            };

// Setup variables for storing Servo Controller output data
volatile uint16_t servo_output[numSCs] = {0b0000000000000000};
uint16_t servo_output_prev[numSCs] = {0b0000000000000000};

// Transmit delay - delay time between transmitting to each controller. Can be set to 0 for single controller.
// Has been partially tested at 0, however this is not fully confirmed.
const uint16_t transmitDelay = 10;

// Read the Inputs and save them to the Input_state array
void Handle_inputs(uint8_t SCnum){ 
    for (uint8_t i = 0; i < numInputs; i++){
        uint8_t state = !digitalRead(Input_pin[i]);        
        bitWrite( Input_state[SCnum], i, state);
    }
}

// Read the Inputs from the Input_state array and set them in the relevent Servo Controller array
// This may look superfluous however the input states may not all be local to the Ardunio - e.g
// when an external IO exander is used. 
void Handle_data_SC12(uint8_t SCnum){ 
    servo_output[SCnum] = Input_state[SCnum];
}

//Send data to a servo controller
void Send_data_SC12(uint8_t SCnum){ 
    
    // Turn the LED on to show transmit
    digitalWrite(TRANSMIT_LED, HIGH);

    // Put the servo_output[SCnum] 16-bit value into a 2 8-bit buffer ready to send to the SC
    uint8_t data_byte [2];
    data_byte[0] = ((uint8_t) (servo_output[SCnum] >> 8));
    data_byte[1] = ((uint8_t) (servo_output[SCnum] & 0xff));  
   	
    // Begin transmission to releveant servo controller
    Wire.beginTransmission(SC12_ADDR[SCnum]); 
    // Write the two data bytes
    Wire.write( data_byte, 2 );
    // If we have a sucsessful send (we get an ACK bit)
	if (Wire.endTransmission () == 0){  

		#ifdef NOTIFY_I2C_DATA
            if ( servo_output[SCnum] != servo_output_prev[SCnum]){
            Terminal.println(F("-----------------------------------"));
            Terminal.print(F("Data sent to SC:"));
            Terminal.println(SCnum);
            Terminal.print(F("SC Address:")); Terminal.print(SC12_ADDR[SCnum]);
            Terminal.println();
			Terminal.print(F("output_b1_SC ")); Terminal.print(SCnum); Terminal.print(F(" OLD: ")); for (char b = 0; b < 8; b++) { Terminal.print(bitRead(servo_output_prev[SCnum], b)); }
			Terminal.print(F("\t"));
			Terminal.print(F("output_b2_SC ")); Terminal.print(SCnum); Terminal.print(F(" OLD: ")); for (char b = 8; b < 16; b++) { Terminal.print(bitRead(servo_output_prev[SCnum], b)); }
			Terminal.println();
			Terminal.print(F("output_b1_SC ")); Terminal.print(SCnum); Terminal.print(F(" NEW: ")); for (char b = 0; b < 8; b++) { Terminal.print(bitRead(servo_output[SCnum], b)); }
			Terminal.print(F("\t"));
			Terminal.print(F("output_b2_SC ")); Terminal.print(SCnum); Terminal.print(F(" NEW: ")); for (char b = 8; b < 16; b++) { Terminal.print(bitRead(servo_output[SCnum], b)); }
            Terminal.println();
            }
		#endif
		
        // Update the most recent transmission data    
		servo_output_prev[SCnum] = servo_output[SCnum]; 
	}
    else{   
        // If the send was unsucseful, let us know
        // It is useful to put some form of physical error reporting here,
        // such as a error LED. 
        #ifdef NOTIFY_I2C_DATA
            Terminal.print(F("Unable to connect to SC "));	
            Terminal.print(SCnum);
            Terminal.print(F(" Address: "));
            Terminal.println(SC12_ADDR[SCnum]);   
        #endif       
    }
    digitalWrite(TRANSMIT_LED, LOW);
     // Wait a sec before sending to next controller	
	delay(transmitDelay);
  }


void setup(){

    // Begin the serial Terminal for debug output
    Terminal.begin(115200);

    // Start I2C in 12.5kHz mode for Megapionts Network
    Wire.begin();    
    TWBR = 158;  
    TWSR |= bit (TWPS0);

    pinMode(TRANSMIT_LED, OUTPUT); 

    for (uint8_t  i = 0; i < numInputs; i++){
        pinMode(Input_pin[i], INPUT_PULLUP);
    }

}

void loop(){
    // Read Input states 
    for (uint8_t SCnum = 0; SCnum < numSCs; SCnum++) {
        // Get the data from the inputs and store to a local variable
        Handle_inputs(SCnum);
    }
        
    // Action them to the correct Servo Controller
    for (uint8_t SCnum = 0; SCnum < numSCs; SCnum++) {    
        // Read the data from the local input variables and save it to the output variable    
        Handle_data_SC12(SCnum);
        // If the out data has changed since we last sent it, send the new data.
        // Otherwise, wait for new data. This can be removed - it is in place to prevent 
        // overloading the I2CNet
        if ( servo_output[SCnum] != servo_output_prev[SCnum] ){
            // Send data to relevent Servo Controller
            Send_data_SC12(SCnum); 
        }  
    } 
}

