// Standard libraries
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

// Libraries for I2C and the MPU6050
#include <bcm2835.h>
#include "MPU6050.h"

#define PI 3.14159265359

// Signal handler callback function
volatile sig_atomic_t done = 0;
void sig_handler(int signum) {
    done = 1;
}
int main(int argc, char **argv) {
    // Set up signal handler
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = sig_handler;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    // Create new file with timestamp
    char filename_buffer[255];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(filename_buffer, "imu_data_%04d-%02d-%02dT%02d%02d%02d.log", 
            tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, 
            tm.tm_hour, tm.tm_min, tm.tm_sec);
    FILE *f = fopen(filename_buffer, "w");
    
    // Initialize I2C and the sensor itself
    const float accel_scaling = 4.0/32767.0;
    const float gyro_scaling = 500.0/32767.0*PI/180.0;
    I2Cdev::initialize();
    MPU6050 imu;
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    if ( imu.testConnection() ) 
        printf("MPU6050 connection test successful\n") ;
    else {
        fprintf(stderr, "MPU6050 connection test failed! something maybe wrong, continuing anyway though ...\n");
    }
    imu.initialize();
    // Set gyro sampling rate to 1kHz
    imu.setRate(7);

    // Initialize time
    struct timeval start_time, current_time;

    while(!done) {
        // Read sensor data 
        gettimeofday(&start_time, NULL);
        imu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        gettimeofday(&current_time, NULL);
        // Write start and end times of measurement
        fprintf(f,"%ld.%06ld,%ld.%06ld,",
            (long int) start_time.tv_sec, (long int) start_time.tv_usec, 
            (long int) current_time.tv_sec, (long int) current_time.tv_usec);
        // Write acceleration data in g's
        fprintf(f,"%0.6f,%0.6f,%0.6f,", (float) ax*accel_scaling, (float) ay*accel_scaling, (float) az*accel_scaling);
        // Write gyro data in rad/s
        fprintf(f,"%0.6f,%0.6f,%0.6f\n", (float) gx*gyro_scaling, (float) gy*gyro_scaling, (float) gz*gyro_scaling);
        fflush(stdout);
        bcm2835_delay(1);
    }
    printf("Exiting cleanly...\n");
    fclose(f);
    return 0; 
}
