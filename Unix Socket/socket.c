
/* 
 * File:   socket.c
 * Author: raslanove
 *
 * Created on October 10, 2021, 12:58 PM
 */

// See: https://stackoverflow.com/a/2760267/1942069

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

// For error printing,
#include <errno.h>
#include <stdarg.h>

// Test condition, if true, print the message,
static void check(int condition, const char * message, ...) {
    
    // perror() is fine, but I sometimes I want to provide my own formatting.
    
    if (condition) {
        va_list args;
        va_start(args, message);
        vfprintf(stderr, message, args);
        va_end(args);
        fprintf(stderr, "\n");
        exit(EXIT_FAILURE);
    }
}

// Prepares a socket address in the Linux "abstract namespace".
// Note: the socket code doesn't require null termination on the filename, but
// we do it anyway so string functions work. One less byte isn't much anyway.
int initializeSocketAddress(const char* socketName, struct sockaddr_un* socketAddress, socklen_t* socketLength) {
    
    // Abstract namespace addresses start with a 0 (?). Make sure the provided name
    // fits within the maximum allowed length,    
    int nameLen = strlen(socketName);
    if (nameLen >= (int) sizeof(socketAddress->sun_path) -1) return -1;
    
    // Denote this as belonging to the abstract namespace,
    socketAddress->sun_path[0] = '\0';
    
    // Set the name,    
    strcpy(socketAddress->sun_path+1, socketName);
    
    // Unix socket,
    socketAddress->sun_family = AF_LOCAL;
    
    // Compute the socket address size (the structure up to the end of the address),
    *socketLength = 1 + nameLen + offsetof(struct sockaddr_un, sun_path);
    
    return 0;
}

#define SOCKET_NAME "com.example.socket"

int main(int argc, char** argv) {
    static const char* message = "besm Allah :)\n";
    struct sockaddr_un socketAddress;
    socklen_t socketLength;
    
    if (argc != 2 || (argv[1][0] != 'c' && argv[1][0] != 's')) {
        printf("Usage: ./socket.o <c|s>   # c: client, s: server\n");
        return 2;  // See: https://www.shellscript.sh/exitcodes.html
    }
    int server = argv[1][0] == 's';

    // Prepare an abstract address,
    check(initializeSocketAddress(SOCKET_NAME, &socketAddress, &socketLength), "Socket name %s is too long: %s", SOCKET_NAME, strerror(errno));
    
    // Create socket,
    int socketFileDescriptor = socket(AF_LOCAL, SOCK_STREAM, PF_UNIX);
    check(socketFileDescriptor==-1, "Socket creation failed: %s", strerror(errno));
    
    int result = EXIT_FAILURE;        
    if (server) {
        
        // Greeting,
        printf("SERVER %s\n", socketAddress.sun_path+1);
        
        // Wait for client,
        if (bind(socketFileDescriptor, (const struct sockaddr*) &socketAddress, socketLength) < 0) { perror("Server bind()"); goto finish; }
        if (listen(socketFileDescriptor, 5) < 0) { perror("Server listen()"); goto finish; }        
        int clientSocket = accept(socketFileDescriptor, NULL, NULL);        
        if (clientSocket < 0) { perror("Server accept"); goto finish; }
        
        // Read a message and close,
        char buffer[64];
        int count = read(clientSocket, buffer, sizeof(buffer));
        close(clientSocket);
        if (count < 0) { perror("Server read"); goto finish; }

        // Do something with message,
        printf("GOT: '%s'\n", buffer);
    
    } else { // Client.
        
        // Greeting,
        printf("CLIENT %s\n", socketAddress.sun_path+1);

        // Connect to server,
        if (connect(socketFileDescriptor, (const struct sockaddr*) &socketAddress, socketLength) < 0) { perror("Client connect()"); goto finish; }
        
        // Send a message,
        if (write(socketFileDescriptor, message, strlen(message)+1) < 0) { perror("Client write()"); goto finish; }
    }

    // Reaching this point means no errors,
    result = EXIT_SUCCESS;

finish:
    
    // Clean up,
    close(socketFileDescriptor);
    return result;
}
