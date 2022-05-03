#ifndef LEFORUM_LIB_COMMUNICATION_H_
#define LEFORUM_LIB_COMMUNICATION_H_

#include <stdint.h>
#include <sys/socket.h>

#define MAX_PACKET_SIZE      64 * 1024

/**
 * @brief Human-readable IPv4 address. 
 * 
 */
struct HAddress {
	char     addr[32];
	uint16_t port;
};
typedef struct HAddress HAddress;

/**
 * @brief Does the same thing as printf() except it
 * prints result to the socket. 
 * 
 * @param fd Socket file descriptor, where the data will be sent 
 * @param str Format string 
 * @param ... Format arguments 
 * @return The number of bytes sent 
 */
ssize_t sendf(int fd, char *str, ...);

#endif