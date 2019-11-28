// Name: Kaleb Meeks
// NetID: krm534

// Libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <cstdlib>
#include <time.h>
#include <ctype.h>

using namespace std;

// Variables
int r_port;
FILE* fp;
string conc;

// Generate Random Port
int randomPort() {
    srand(time(NULL));
    return rand() % ((65535 - 1024) + 1) + 1024;
}

// Capitalization Function
string capitalize(char* payload) {
    int i = 0;
    char c;
    string charArr;
    while(payload[i]) {
        c = payload[i];
        charArr += toupper(c);
        i++;
    }
    return charArr;
}

int main(int argc, char* argv[]) {
    
    // Variables
    int port = strtol(argv[1], NULL, 10);
    
    // *** Declare TCP socket ***
    int tcpsocket = 0;
    tcpsocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpsocket == -1) {
        cerr << "Can't create a socket";
        return -1;
    }

    // Receive data
    struct sockaddr_in server;
    memset((char *) &server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(tcpsocket, (struct sockaddr *) &server, sizeof(server)) == -1) {
        cerr << "Can't bind to IP/Port";
        return -2;
    }

    if (listen(tcpsocket, SOMAXCONN) == -1) {
        cerr << "Can't listen";
        return -3;
    }

    struct sockaddr_in client;
    char payload[512];
    socklen_t clen = sizeof(client);
    int clientSocket = accept(tcpsocket, (sockaddr*)&client, &clen);

    if (clientSocket == -1) {
        cerr << "Problem with client connecting";
        return -4;
    }
    recv(clientSocket, payload, 512, 0);
 
    // Check client data
    if (strtol(payload,NULL,10) == 117) {
        r_port = randomPort();
        cout << "Handshake detected. Selected the random port " << r_port << "\n";
        }
    else {
        cout << "Error occurred\n";
    }

    // Return random port
    memset(payload, 0, sizeof(payload));
    sprintf(payload,"%ld",r_port);
    int sendRes = send(clientSocket, payload, 512, 0);
    if (sendRes == -1) {
        cerr << "Could not send to server\n";
        return 1;
    }
    close(clientSocket);
    close(tcpsocket);

    // *** Declare UDP socket ***
    int udpsocket = 0;
    udpsocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpsocket == -1) {
        cerr << "Can't create a socket";
        return -1;
    }

    // Receive data
    memset((char *) &server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(r_port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(udpsocket, (struct sockaddr *) &server, sizeof(server)) == -1) {
        cerr << "Can't bind to IP/Port";
        return -2;
    }

    // UDP ack transfer
    bool end = false;
    while(true) {
        clen = sizeof(client);
        memset(payload, 0, sizeof(payload));
        recvfrom(udpsocket, payload, 32, 0, (sockaddr*)&client, &clen);
        
        // Convert payload to string
        // Check if payload contains null terminating character
        string load;
        for(int j = 0; j < 4; j++) {
            if (payload[j] == '\0') {
                end = true;
            }
            load += payload[j];
            conc += payload[j];
        }

        // Reset load string
        load.clear();

        // Capitalize payload
        load = capitalize(payload);

        // Send ack back to client
        clen = sizeof(client);
        memset(payload, 0, sizeof(payload));
        strcpy(payload, load.c_str());
        load.clear();
        sendRes = sendto(udpsocket, payload, 32, 0, (struct sockaddr *) &client, clen);
        if (sendRes == -1) {
            cerr << "Could not send to server";
            return 1;
        }

        if (end == true) {
            break;
        }
    }

    // File manipulation
    fp = fopen("dataReceived.txt", "w");
    fputs(conc.c_str(), fp);
    fclose(fp);

close(udpsocket);
return 0;
}
