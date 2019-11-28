// Name: Kaleb Meeks
// NetID: krm534

// Libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[]) {

// Variables
int port = strtol(argv[2], NULL, 10);
string file = argv[3];
int r_port;
FILE* fp;
string fileString;
int charCounter = 0;
char c;

// *** Declare TCP socket ***
int tcpsocket = 0;
tcpsocket = socket(AF_INET, SOCK_STREAM, 0);
if (tcpsocket == -1) {
    cerr << "Can't create a socket";
    return 1;
}

// Get host IP address
struct hostent *s;
s = gethostbyname(argv[1]);

// Setting destination info
struct sockaddr_in server;
memset((char *) &server, 0, sizeof(server));
server.sin_family = AF_INET;
server.sin_port = htons(port);
bcopy((char *) s->h_addr, (char *) &server.sin_addr.s_addr, s->h_length);

// Connect to server
int connectRes = connect(tcpsocket, (sockaddr*)&server, sizeof(server));
if (connectRes == -1) {
    cerr << "Can't connect to socket";
    return 1;
}

// Sending data to server
char payload[512] = "117";
int sendRes = send(tcpsocket, payload, 512, 0);
if (sendRes == -1) {
    cerr << "Could not send to server";
    return 1;
}

// Receive r_port from server
memset(payload, 0, sizeof(payload));
recv(tcpsocket, payload, 512, 0);
r_port = strtol(payload, NULL, 10);
close(tcpsocket);

// *** Declare UDP socket ***
int udpsocket = 0;
udpsocket = socket(AF_INET, SOCK_DGRAM, 0);
if (udpsocket == -1) {
    cerr << "Can't create a socket";
    return 1;
}

// Get host IP address
s = gethostbyname(argv[1]);

// Setting destination info
memset((char *) &server, 0, sizeof(server));
server.sin_family = AF_INET;
server.sin_port = htons(r_port);
bcopy((char *) s->h_addr, (char *) &server.sin_addr.s_addr, s->h_length);

// File manipulation
fp = fopen(file.c_str(), "r");
    while (c != EOF) {
        c = fgetc(fp);
        fileString += c;
        charCounter++;
    }
fclose(fp);

// Remove file character from string
fileString.erase(fileString.length() - 1);

// UDP file transfer
int i = 0, counter = 4;
string dataBuffer;
int k = 0;
bool end = false;
while(true) {
    // Get 4 characters from string
    while (i < counter) {
        dataBuffer += fileString[i];
        i++;
    }

    // Increment counter and i
    counter += 4;

    // Check if payload contains null terminating character
    for(int j = 0; j < 4; j++) {
        if (dataBuffer[j] == '\0') {
            end = true;
        }
    }

    if (counter > charCounter) {
        counter = charCounter;
    }

    // Send characters to server
    socklen_t slen = sizeof(server);
    memset(payload, 0, sizeof(payload));
    strcpy(payload, dataBuffer.c_str());
    dataBuffer.clear();

    sendRes = sendto(udpsocket, payload, 32, 0, (struct sockaddr *) &server, slen);
    if (sendRes == -1) {
        cerr << "Could not send to server";
        return 1;
    }

    // Receive ack from server
    slen = sizeof(server);
    memset(payload, 0, sizeof(payload));
    recvfrom(udpsocket, payload, 32, 0, (sockaddr*)&server, &slen);
    cout << payload << "\n";

    // Check if end
    if (end == true) {
        break;
    }
}
close(udpsocket);
return 0;
}