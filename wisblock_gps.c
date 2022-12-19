#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "D:\\Pico\\pico-sdk\\src\\rp2_common\\pico_multicore\\include\\pico\\multicore.h"

#define LED_GREEN 23
#define LED_BLUE 24

#define UART1TX 4
#define UART1Rx 5

#define GPS_1PPS 6  // MAX-7 timepulse output 1 pulse per second
#define GPS_RESET 22

#define UART_ID uart1
#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

char buffer[255];
int buf_ptr = 0;

char utc_time[40];
char utc_date[40];
char longitude[40];
char latitude[40];
char altitude[40];
char satellites[40];

int hexchar2int(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

int hex2int(char *c)
{
	int value;

	value = hexchar2int(c[0]);
	value = value << 4;
	value += hexchar2int(c[1]);

	return value;
}

int checksum_valid(char *string)
{
	char *checksum_str;
	int checksum;
	unsigned char calculated_checksum = 0;

	// Checksum is postcede by *
	checksum_str = strchr(string, '*');
	if (checksum_str != NULL){
		// Remove checksum from string
		*checksum_str = '\0';
		// Calculate checksum, starting after $ (i = 1)
		for (int i = 1; i < strlen(string); i++) {
			calculated_checksum = calculated_checksum ^ string[i];
		}
		checksum = hex2int((char *)checksum_str+1);
		//printf("Checksum Str [%s], Checksum %02X, Calculated Checksum %02X\r\n",(char *)checksum_str+1, checksum, calculated_checksum);
		if (checksum == calculated_checksum) {
			//printf("Checksum OK");
			return 1;
		}
	} else {
		//printf("Error: Checksum missing or NULL NMEA message\r\n");
		return 0;
	}
	return 0;
}

int parse_comma_delimited_str(char *string, char **fields, int max_fields)
{
	int i = 0;
	fields[i++] = string;

	while ((i < max_fields) && NULL != (string = strchr(string, ','))) {
		*string = '\0';
		fields[i++] = ++string;
	}

	return --i;
}

int SetTime(char *date, char *time)
{
	struct timespec ts;
	struct tm gpstime;
	time_t secs;
	char tempbuf[2];
	int ret;

	printf("GPS    UTC_Date %s, UTC_Time %s\r\n",date, time);
	// GPS date has format of ddmmyy
	// GPS time has format of hhmmss.ss

	if ((strlen(date) != 6) | (strlen(time) != 9)) {
		printf("No date or time fix. Exiting\r\n");
		return 1;
	}

	// Parse day:
	strncpy(tempbuf, (char *)date, 2);
	tempbuf[2] = '\0';
	gpstime.tm_mday = atoi(tempbuf);

	// Parse month:
	strncpy(tempbuf, (char *)date+2, 2);
	tempbuf[2] = '\0';
	gpstime.tm_mon = atoi(tempbuf) - 1;

	// Parse year:
	strncpy(tempbuf, (char *)date+4, 2);
	tempbuf[2] = '\0';
	gpstime.tm_year = atoi(tempbuf) + 100;

	// Parse hour:
	strncpy(tempbuf, (char *)time, 2);
	tempbuf[2] = '\0';
	gpstime.tm_hour = atoi(tempbuf);

	// Parse minutes:
	strncpy(tempbuf, (char *)time+2, 2);
	tempbuf[2] = '\0';
	gpstime.tm_min = atoi(tempbuf);

	// Parse seconds:
	strncpy(tempbuf, (char *)time+4, 2);
	tempbuf[2] = '\0';
	gpstime.tm_sec = atoi(tempbuf);

	printf("Converted UTC_Date %02d%02d%02d, UTC_Time %02d%02d%02d.00\r\n",gpstime.tm_mday,(gpstime.tm_mon)+1,(gpstime.tm_year)%100, gpstime.tm_hour, gpstime.tm_min, gpstime.tm_sec);

	ts.tv_sec = mktime(&gpstime);
	// Apply GMT offset to correct for timezone
//	ts.tv_sec += gpstime.tm_gmtoff;

	printf("Number of seconds since Epoch %ld\r\n",ts.tv_sec);

	// ts.tv_nsec = 0;
	// ret = clock_settime(CLOCK_REALTIME, &ts);
	// if (ret)
	// 	perror("Set Clock");

	//clock_gettime(CLOCK_REALTIME, &ts);
	//printf("Number of seconds since Epoch %ld\r\n",ts.tv_sec);
	//gpstime = gmtime(&ts.tv_sec);
	//printf("System UTC_Date %02d%02d%02d, ",gpstime->tm_mday,(gpstime->tm_mon)+1,(gpstime->tm_year)%100);
	//printf("UTC_Time %02d%02d%02d.00\r\n", gpstime->tm_hour, gpstime->tm_min, gpstime->tm_sec);
	printf("\r\n");
}

void decode_nmea() {
    int i;
    char *field[20];

    if (checksum_valid(buffer)) {
        if ((strncmp(buffer, "$GP", 3) == 0) |
            (strncmp(buffer, "$GN", 3) == 0)) {

            if (strncmp(&buffer[3], "GLL", 3) == 0) {
                i = parse_comma_delimited_str(buffer, field, 20);

                if (strlen(field[1]) > 4)
                	sprintf(latitude, "%c%c %s %s", field[1][0], field[1][1], &field[1][2], field[2]);
                if (strlen(field[3]) > 4)
                	sprintf(longitude, "%c%c%c %s %s", field[3][0], field[3][1], field[3][2], &field[3][3], field[4]);
                if (strlen(field[5]) > 5)
                    sprintf(utc_time, "%c%c:%c%c:%c%c", field[5][0], field[5][1], field[5][2], field[5][3], field[5][4], field[5][5]);
            }

            if (strncmp(&buffer[3], "GGA", 3) == 0) {
                i = parse_comma_delimited_str(buffer, field, 20);

                if (strlen(field[1]) > 5)
                    sprintf(utc_time, "%c%c:%c%c:%c%c", field[1][0], field[1][1], field[1][2], field[1][3], field[1][4], field[1][5]);
				if (strlen(field[2]) > 4)
                	sprintf(latitude, "%c%c %s %s", field[2][0], field[2][1], &field[2][2], field[3]);
                if (strlen(field[2]) > 5)
                	sprintf(longitude, "%c%c%c %s %s", field[4][0], field[4][1], field[4][2], &field[4][3], field[5]);
                strcpy(altitude, field[9]);
                strcpy(satellites, field[7]);
            }

			if (strncmp(&buffer[3], "RMC", 3) == 0) {
				i = parse_comma_delimited_str(buffer, field, 20);

				if (strlen(field[1]) > 5)
                    sprintf(utc_time, "%c%c:%c%c:%c%c", field[1][0], field[1][1], field[1][2], field[1][3], field[1][4], field[1][5]);
                if (strlen(field[3]) > 4)
                	sprintf(latitude, "%c%c %s %s", field[3][0], field[3][1], &field[3][2], field[4]);
                if (strlen(field[5]) > 4)
                	sprintf(longitude, "%c%c%c %s %s", field[5][0], field[5][1], field[5][2], &field[5][3], field[6]);
				strcpy(utc_date, field[9]);

				//SetTime(field[9],field[1]);
			}
        }
    }

}

// RX interrupt handler
void on_uart_rx() {
    while (uart_is_readable(uart1)) {
        uint8_t ch = uart_getc(uart1);
        if (ch == '$') {
            buffer[buf_ptr] = '\0';
            decode_nmea();
            buf_ptr = 0;
        }
        buffer[buf_ptr] = ch;
        buf_ptr++;
    }
}

int main() {
    stdio_init_all();

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);

    uart_init(uart1, 9600);
    gpio_set_function(UART1TX, GPIO_FUNC_UART);
    gpio_set_function(UART1Rx, GPIO_FUNC_UART);
 
    gpio_init(GPS_RESET);
    gpio_set_dir(GPS_RESET, GPIO_OUT);
    gpio_put(GPS_RESET, 0);
    sleep_ms(1000);
    gpio_put(GPS_RESET, 1);


    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

    while (1) {
        sleep_ms(1000);

		// Clear terminal 
		printf("\e[1;1H\e[2J");
		printf("Latitude   %s\r\n", latitude);
		printf("Longitude  %s\r\n", longitude);
		printf("Altitude   %s\r\n", altitude);
		printf("Satellites %s\r\n", satellites);
		printf("UTC Time   %s\r\n", utc_time);
		printf("Date       %s\r\n", utc_date);
	}
}
