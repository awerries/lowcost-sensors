/**
 * Adam Werries (awerries@cmu.edu)
 *
 * Code based on the Skytraq Venus 6 GPS Receiver Binary Messages document
 * https://www.sparkfun.com/datasheets/GPS/Modules/AN0003_v1.4.14_FlashOnly.pdf
 */
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <math.h>

#define BUFFER_SIZE 500

// Structs for convenience
typedef struct file_log {
    struct timeval start_time;
    struct timeval end_time;
    FILE * file;
} FileLog;

typedef struct packet_buffer {
    int pkt_idx;
    bool got_start_bytes;
    uint8_t last_byte;
    uint8_t pkt_buf[BUFFER_SIZE];
} PacketBuffer;

// Function prototypes
int initialize_serial();
uint8_t checksum(uint8_t *buf, int size);
void process_rx_data(PacketBuffer*, uint8_t*, int, FileLog*);
void process_binary_message(uint8_t *, int, FileLog*);

// Signal handler callback function
volatile sig_atomic_t done = 0;
void sig_handler(int signum) {
    done = 1;
}


int main() {
    // Set up signal handler
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = sig_handler;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    // Prepare new file with timestamp
    FileLog log;
    char filename_buffer[255];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(filename_buffer, "gps_data_%04d-%02d-%02dT%02d%02d%02d.log", 
            tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, 
            tm.tm_hour, tm.tm_min, tm.tm_sec);


    // Initialize buffers for reading serial data and parsing it into packets
    PacketBuffer packet;
    uint8_t rxbuf[BUFFER_SIZE];
    int size = -1;

    // Initialize serial port
    int serial_fd = initialize_serial();
    if (serial_fd > 0) {
        log.file = fopen(filename_buffer, "w");
        fprintf(log.file, "# start_time, end_time, gps_time, lat, long, elev, "
                          "ecef_x, ecef_y, ecef_z, ecef_vx, ecef_vy, ecef_vz, "
                          "gdop, pdop, hdop, vdop, tdop\n");
        // Read sensor data
        while (!done) {
            size = read(serial_fd, rxbuf, BUFFER_SIZE);
            if(size > 0)
                process_rx_data(&packet, rxbuf, size, &log);
        }
        fclose(log.file);
        close(serial_fd);
    }

    return 0;
}

int initialize_serial() {
    int serial_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
    if(serial_fd < 0) {
        printf("Could not open serial port\n");
        return -1;
    }

    struct termios tio;
    memset(&tio, 0, sizeof(tio));
    tio.c_cflag = B115200;
    tio.c_cflag = tio.c_cflag & ~CSIZE & ~CSTOPB & ~PARENB & ~PARODD;
    tio.c_cflag = tio.c_cflag | CS8;
    tio.c_iflag = IGNPAR;
    tio.c_cflag = tio.c_cflag | CLOCAL | CREAD;
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 0;

    tcflush(serial_fd, TCIFLUSH);

    fcntl(serial_fd, F_SETFL, O_NONBLOCK);
    if (tcsetattr(serial_fd, TCSANOW, &tio) < 0) {
        printf("Can't set terminal parameters\n");
        close(serial_fd);
        return -1;
    }

    /* clean I & O device */
    tcflush(serial_fd, TCIOFLUSH);
    signal(SIGIO, SIG_IGN);

    return serial_fd;
}

uint8_t checksum(uint8_t *buf, int size) {
  uint8_t ret = 0x00;
  
  for(int i=0; i<size; i++)
    ret = ret ^ buf[i];

  return ret;
}

void process_rx_data(PacketBuffer *packet, uint8_t *buf, int size, FileLog * log) {
    for(int i=0; i<size; i++) {
        if(!packet->got_start_bytes) {
            // Check for start byte
            if(packet->last_byte == 0xA0 && buf[i] == 0xA1) {
                gettimeofday(&log->start_time, NULL);
                packet->got_start_bytes = true;
                packet->pkt_buf[0] = 0xA0;
                packet->pkt_buf[1] = 0xA1;
                packet->pkt_idx = 2;
                continue;
            }

            packet->last_byte = buf[i];
        } else {
            packet->pkt_buf[packet->pkt_idx] = buf[i];

            // Check for end bytes
            if(packet->pkt_buf[packet->pkt_idx-1] == 0x0D && packet->pkt_buf[packet->pkt_idx] == 0x0A) {
                gettimeofday(&log->end_time, NULL);
                process_binary_message(packet->pkt_buf, packet->pkt_idx+1, log);
                packet->pkt_idx = 0;
                packet->got_start_bytes = false;
                continue;
            }

            packet->pkt_idx++;

            if(packet->pkt_idx > BUFFER_SIZE) {
                printf("RX buffer overflow. Discarding data received so far");
                // Clear buffer, start afresh
                packet->got_start_bytes = false;
                packet->last_byte = 0x00;
                packet->pkt_idx = 0;
            }
        }
    }
}

void process_binary_message(uint8_t *buf, int size, FileLog * log) {
    if(size < 8) {
        printf("Rx packet too small. size=%d\n", size);
        return;
    }

    // check start bytes
    if(buf[0] != 0xA0 || buf[1] != 0xA1) {
        printf("Incorrect start bytes. Expected=0xA0, 0xA1 Received=0x%02X, 0x%02X\n",
                ((int)buf[0] & 0xff), ((int)buf[1] & 0xff));
        return;
    }

    // check end bytes
    if(buf[size-2] != 0x0D || buf[size-1] != 0x0A) {
        printf("Incorrect end bytes. Expected=0x0D, 0x0A Received=0x%02X, 0x%02X\n",
                ((int)buf[size-2] & 0xff), ((int)buf[size-1] & 0xff));
        return;
    }

    int payload_size = buf[3] | (buf[2] << 8);
    // sanity check on payload
    if(payload_size != (size-7)) {
        printf("Incorrect packet payload size. Expected=%d, Received=%d\n");
        return;
    }

    // check checksum
    uint8_t chk = checksum(&buf[4], payload_size);
    if(chk != buf[size-3]) {
        printf("Incorrect checksum. Expected=0x%02X, Received=0x%02X\n",
                chk, buf[size-3]);
        return;
    }

    // print message ID
    uint16_t msg_id = (uint16_t)buf[4];
    printf("Rx MSG ID: 0x%02X,", msg_id & 0xff);


    // Skip header
    uint8_t *payload = &buf[4];

    /********* Parse payload *************/
    int fix_mode = payload[1];
    int num_sat = payload[2];

    // Generate gps-time from the messy sequence
    uint16_t gpsweek = (uint16_t) (payload[3] << 8 | payload[4]);
    uint32_t tow_v = (uint32_t) (payload[5] << 24 | payload[6] << 16 | payload[7] << 8 | payload[8]);
    double tow = (double) tow_v / 100.0;
    long int tow_micro = (tow - (long int)tow) * 1e6;
    struct tm tm;
    struct timeval gps_time;
    time_t dt;
    tm.tm_year = 80;
    tm.tm_mon = 0;
    tm.tm_mday = 6;
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    dt = mktime(&tm);
    gps_time.tv_sec = dt + gpsweek*7*24*60*60 + (long int)tow;
    gps_time.tv_usec = tow_micro;

    // Convert lat/lon/pose to usable degrees, meters, meter/s
    int lat_val = (int) (payload[9] << 24 | payload[10] << 16 | payload[11] << 8 | payload[12]);
    int lon_val = (int) (payload[13] << 24 | payload[14] << 16 | payload[15] << 8 | payload[16]);

    double lat = (double)lat_val / 10000000.0;
    double lon = (double)lon_val / 10000000.0;
    double elev_m = (double) (payload[21] << 24 | payload[22] << 16 | payload[23] << 8 | payload[24]) / 100.0;

    double gdop = (double) (payload[25] << 8 | payload[26]) / 100.0;
    double pdop = (double) (payload[27] << 8 | payload[28]) / 100.0;
    double hdop = (double) (payload[29] << 8 | payload[30]) / 100.0;
    double vdop = (double) (payload[31] << 8 | payload[32]) / 100.0;
    double tdop = (double) (payload[33] << 8 | payload[34]) / 100.0;

    double ecef_x = (double) (payload[35] << 24 | payload[36] << 16 | payload[37] << 8 | payload[38]) / 100.0;
    double ecef_y = (double) (payload[39] << 24 | payload[40] << 16 | payload[41] << 8 | payload[42]) / 100.0;
    double ecef_z = (double) (payload[43] << 24 | payload[44] << 16 | payload[45] << 8 | payload[46]) / 100.0;
    double ecef_vx = (double) (payload[47] << 24 | payload[48] << 16 | payload[49] << 8 | payload[50]) / 100.0;
    double ecef_vy = (double) (payload[51] << 24 | payload[52] << 16 | payload[53] << 8 | payload[54]) / 100.0;
    double ecef_vz = (double) (payload[55] << 24 | payload[56] << 16 | payload[57] << 8 | payload[58]) / 100.0;
    // Write start, end, and gps times of measurement
    double speed = sqrt(ecef_vx*ecef_vx + ecef_vy*ecef_vy + ecef_vz*ecef_vz) * 2.23694;
    printf("lat:%4.6f,long:%4.6f,elev:%4.6f,speedmph:%4.6f\n", lat, lon, elev_m, speed);
    fprintf(log->file,"%ld.%06ld,%ld.%06ld,%ld.%06ld,",
        (long int) log->start_time.tv_sec, (long int) log->start_time.tv_usec, 
        (long int) log->end_time.tv_sec, (long int) log->end_time.tv_usec,
        (long int) gps_time.tv_sec, (long int) gps_time.tv_usec);
    fprintf(log->file,"%d,%d,", fix_mode, num_sat);
    fprintf(log->file,"%4.6f,%4.6f,%4.6f,", lat, lon, elev_m);
    fprintf(log->file,"%4.6f,%4.6f,%4.6f,%4.6f,%4.6f,%4.6f,", ecef_x, ecef_y, ecef_z, ecef_vx, ecef_vy, ecef_vz);
    fprintf(log->file,"%4.6f,%4.6f,%4.6f,%4.6f,%4.6f\n", gdop, pdop, hdop, vdop, tdop);
    fflush(stdout);
}
