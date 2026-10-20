/**
 * @name Main File
 * @brief Main file for CprE 288 Lab 5
 * @author Max Miller
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

static double smallest_ang = 0;
static double target_ang = 0;

void point();
void map(int point_en);
void move_to_object();
void get_pos();
void return_to_home();
void send_help();

extern volatile int button_event;
extern volatile int button_num;

extern volatile char uart_data;
extern volatile char uart_flag;


int main(void) {
    timer_init();
    uart_init();

    cyBot = oi_alloc();
    oi_init(cyBot);
    init_movement(cyBot);


    sprintf(str, "\r\n\r\nCprE288 Simple Mission\r\n\r\n\r\n");
    uart_sendStr(str);

    cyBot_init_Scan_cust(0b111);

    // oi_setMotorCalibration(1.0, 1.0);
    // rot_calibrate(cyBot);

    // Stuff for servo calibration
    // lcd_init();
    // cyBOT_SERVO_cal();

    char cmd = 0;
    int running = true;

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

    while (running) {
        movement_update();

        if (uart_flag) {
            uart_flag = 0;

            cmd = uart_data;
    
            sprintf(str, "Pressed: %c\r\n\r\n", cmd);
            uart_sendStr(str);
    
    
            switch (cmd) {
                // Map
                case 'm':
                    map(true);
                    break;
    
                // Map & Point
                case 'p':
                    point();
                    break;
    
    
                // Forward
                case 'w':
                    move_forward(20);
                    break;
    
                // Left
                case 'a':
                    oi_setWheels(0, 0);
                    break;
    
                // running
                case 's':
                    move_backward(20);
                    break;
    
                // Right
                case 'd':
                    // TODO
                    break;
    
                //move to 5 cm from the object directly ahead
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
    }

    free(sensor_data_ptr);
    free(objects);
    free(cyBot);

}


void point() {
    if (fabs(smallest_ang - target_ang) > 0.5) {
        target_ang = smallest_ang;
        // turn_abs(cyBot, target_ang);
    }
}


/**
 * @name Map
 * @brief Maps objects and points to the smallest one
 */
void map(int point_en) {
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

    smallest_ang =  target_ang + (double)objects[smallest_index].mp - 90.0;

    if (point_en) {
        point();
    }
}


/**
 * @name move_to_object
 * @brief Moves within 5cm of the closest object
 */
void move_to_object() {


    scan_ahead(&dist_ahead, false);

    // Send Raw Sensor Data
    uart_sendStr("**Raw sensor data**\r\n");

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
void get_pos() {
    // TODO: Unimplemented
}


/**
 * @name Return to Home
 * @brief Go back to the origin
 */
void return_to_home() {
    // TODO: Unimplemented
}


void send_help() {
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
    uart_sendStr("\r\nUART COMMANDS: \r\n\r\n");
    uart_sendStr("M: MAP\r\n");
    uart_sendStr("P: MAP & POINT\r\n");
    uart_sendStr("W: Forward\r\n");
    uart_sendStr("A: Left\r\n");
    uart_sendStr("S: Backward\r\n");
    uart_sendStr("D: Right\r\n");
    uart_sendStr("Q: GET CURRENT POSITION\r\n");
    uart_sendStr("R: RETURN TO START POINT\r\n");
    uart_sendStr("E: EXIT\r\n");
}
