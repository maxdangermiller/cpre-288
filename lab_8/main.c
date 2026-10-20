/**
 * @name Main File
 * @brief Main file for CprE 288 Lab 8
 * @author Max Miller, Miles Davis
 */

#include "handle_data.h"
#include "sensor-data.h"
#include "handle_scan.h"
#include "cyBot_Scan.h"
#include "cyBot_Scan_Cal.h"
#include "open_interface.h"
#include "movement.h"
#include "timer.h"
#include "uart.h"


static SensorData_t sensor_data_ptr[SCAN_STEPS];
static double dist_ahead;
static ObjectInfo_t objects[MAX_OBJECTS];
static oi_t *cyBot;
static char str[50];


static enum { MODE_AUTO = 0, MODE_MAN = 1 } ctrl_mode;
static enum { AMS_SCAN, AMS_MOVE, AMS_DONE } auto_mode_state;

static int running;
static char cmd;


// Interrupt Vars
extern volatile int button_event;
extern volatile int button_num;

extern volatile char uart_data;
extern volatile char uart_flag;

// Functional Prototypes
static void handle_manual_mode();
static void handle_auto_mode();
static void handle_mode_toggle();
static void map(int point_en);
static void move_to_object();
static void get_pos();
static void return_to_home();
static void send_help();

int main(void) {
    timer_init();
    uart_init();

    cyBot = oi_alloc();
    oi_init(cyBot);
    move_init(cyBot);


    sprintf(str, "\r\n\r\nCprE288 Simple Mission\r\n\r\n\r\n");
    uart_sendStr(str);

    
    cyBot_init_Scan_cust(0b111);
    
    // Stuff for servo calibration
    // lcd_init();
    // cyBOT_SERVO_cal();
    
    running = true;
    ctrl_mode = MODE_MAN;
    
    /*
    UART COMMANDS:
    
    T: Toggle control mode between auto and manual
    M: MAP
    C: Move within 5cm of object directly in front
    W: Forward
    A: Left
    S: Backward
    D: Right
    Q: GET CURRENT POSITION
    R: RETURN TO START POINT
    E: EXIT
    */
    send_help();

    while (running) {
        move_update();

        if (uart_flag) {
            uart_flag = 0;
            cmd = uart_data;
    
            sprintf(str, "User Pressed: %c\r\n\r\n", cmd);
            uart_sendStr(str);

            if (cmd == 't') {
                handle_mode_toggle();
            }
    
            handle_manual_mode();
            handle_auto_mode();
        }
    }

    free(sensor_data_ptr);
    free(objects);
    free(cyBot);

}

/**
 * @name Handle Manual Mode
 */
static void handle_manual_mode() {
    if (ctrl_mode != MODE_MAN) { return; }

    switch (cmd) {
        // Map
        case 'm':
            map(true);
            break;


        // Forward
        case 'w':
            move_forward(20);
            break;

        // Left
        case 'a':
            move_turn_ccw(22.5);
            break;

        // running
        case 's':
            move_backward(20);
            break;

        // Right
        case 'd':
            move_turn_cw(22.5);
            break;

        // move to 5 cm from the object directly ahead
        case 'c':
            move_to_object();
            break;

        // Send current position
        case 'q':
            get_pos(cyBot);
            break;

        // Return to start position
        case 'r':
            return_to_home(cyBot);
            break;

        case 'h':
            send_help();
            break;

        // Exit
        case 'e':
            running = false;
            break;

        default:
            break;
    }
}


/**
 * @name Handle Auto Mode 
 */
static void handle_auto_mode() {
    if (ctrl_mode != MODE_AUTO) { return; }

    if (move_get_state() != MS_STOP) { 
        return;
    }

    switch (auto_mode_state) {
        case AMS_SCAN:      // 1) Map & Point
            map(1);
            break;
        
        case AMS_MOVE:      // 2) Move Towards Smallest Object
            move_to_object();
            break;
        case AMS_DONE:
            break;
    }
    map(1);
    // 2) Move Towards Smallest Object
    move_to_object();
}


/**
 * @name Handle Mode Switch 
 */
static void handle_mode_toggle() {
    ctrl_mode = !ctrl_mode;

    switch (ctrl_mode) {
        case MODE_AUTO:
            auto_mode_state = AMS_SCAN;
            sprintf(str, "Switching to AUTO mode!\r\n\r\n");
            break;
        case MODE_MAN:
            sprintf(str, "Switching to MANUAL mode!\r\n\r\n");
            break;
        default:
            sprintf(str, "Switching to an UNKNOWN mode!\r\n\r\n");
    }

    uart_sendStr(str);

    // Stop any movement that is currently happening
    move_abort();
}

/**
 * @name Map
 * @brief Maps objects and points to the smallest one
 */
static void map(int point_en) {
    // turn_abs(cyBot, 0.0);
    do_scan(sensor_data_ptr);
    // load_scan(sensor_data_ptr, SCAN_STEPS);

    // Send Raw Sensor Data
    uart_sendStr("**Raw noisy sensor data**\r\n");
    send_scan(sensor_data_ptr);

    clean_data(sensor_data_ptr, SCAN_STEPS);

    // Send Cleaned Sensor Data
    uart_sendStr("** Cleaned data **\r\n");
    send_scan(sensor_data_ptr);

    // Send Cleaned Sensor Data
    uart_sendStr("** Cleaned data (after trig) **\r\n");
    send_scan(sensor_data_ptr);

    // Determine Object count
    int object_count = find_objects(&(sensor_data_ptr[2]), SCAN_STEPS - 2, objects);

    // Send the object table to PuTTy
    send_object_table(objects, object_count);

    int smallest_index = get_smallest_obj(objects, object_count);
    sprintf(str, "Smallest Object is #%d\r\n", objects[smallest_index].num);
    uart_sendStr(str);

    if (point_en) {
        move_turn_rel((double)objects[smallest_index].mp - 90.0);
    }
}


/**
 * @name move_to_object
 * @brief Moves within 5cm of the closest object
 */
static void move_to_object() {


    scan_ahead(&dist_ahead, false);

	sprintf(str, "Sound Distance(cm)\r\n");
    uart_sendStr(str);
    sprintf(str, "%f\r\n", dist_ahead);
    uart_sendStr(str);

    int buffer = 3; //adjust how much space between Cybot and object, in cm
    move_forward((int)((dist_ahead-buffer)*10)); 


}


/**
 * @name Get Pos
 * @brief Get position and send it over UART
 */
static void get_pos() {
    Pos_t pos = move_get_current_pos();

    sprintf(str, "Current Absolute Position is (%6.2lf, %6.2lf) with a heading of %6.2lf\r\n", pos.x, pos.y, pos.a);
    uart_sendStr(str);
}


/**
 * @name Return to Home
 * @brief Go back to the origin
 */
static void return_to_home() {
    move_set_target(0, 0, 0);
}


static void send_help() {
    /*
    UART COMMANDS:

    M: MAP
    P: MAP & POINT
    W: Forward
    A: Left
    S: Backward
    D: Right
    Q: GET CURRENT POSITION
    R: RETURN TO START POINT
    E: EXIT
    */
    uart_sendStr("\r\nUART COMMANDS: \r\n");
    uart_sendStr("--------------------------------------------------\r\n");
    uart_sendStr("T: Toggle control mode between auto and manual\r\n");
    uart_sendStr("M: MAP\r\n");
    uart_sendStr("C: Move within 5cm of object directly in front\r\n");
    uart_sendStr("W: Forward\r\n");
    uart_sendStr("A: Left\r\n");
    uart_sendStr("S: Backward\r\n");
    uart_sendStr("D: Right\r\n");
    uart_sendStr("Q: GET CURRENT POSITION\r\n");
    uart_sendStr("R: RETURN TO START POINT\r\n");
    uart_sendStr("E: EXIT\r\n\r\n");
}
