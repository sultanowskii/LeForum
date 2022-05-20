#ifndef LEFORUM_LIB_COMMUNICATION_H_
#define LEFORUM_LIB_COMMUNICATION_H_

#include <stdint.h>
#include <sys/socket.h>

#define MAX_PACKET_SIZE 1024 * 1024

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
 * @brief Safe wrapper over ssend(). 
 * Sends large amount of bytes by counting how many bytes are sent.
 * For small data chunks, use send().
 * 
 * @param sockfd Socket FD 
 * @param buf Pointer to the data to send 
 * @param size Size of the data to send 
 * @param flags Flags being passed to send() function. Refer to man send 
 * @return Size of sent data 
 */
ssize_t s_send(int sockfd, void *buf, size_t size, int flags); 

/**
 * @brief Safe wrapper over recv(). 
 * Receives large amount of bytes by counting how many bytes are received.
 * For small data chunks, use recv().
 * 
 * @param sockfd Socket FD 
 * @param buf Pointer to the place where received data will be stored 
 * @param size Size of the data to receive 
 * @param flags Flags being passed to recv() function. Refer to man recv 
 * @return Size of received data 
 */
ssize_t s_recv(int sockfd, void *buf, size_t size, int flags);

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
