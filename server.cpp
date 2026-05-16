#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct AcceptedSocket {
    SOCKET acceptedSocketFD;
    sockaddr_in address;
    bool acceptedSuccessfully;
    int error;
};

AcceptedSocket acceptedSockets[10];
int acceptedSocketsCount = 0;

AcceptedSocket* acceptIncomingConnection(SOCKET serverSocketFD);

void startAcceptingIncomingConnections(SOCKET serverSocketFD);

void receiveAndPrintIncomingData(SOCKET socketFD);

void receiveAndPrintIncomingDataOnSeparateThread(
    AcceptedSocket* clientSocket
);

void sendReceivedMessageToTheOtherClients(
    char* buffer,
    SOCKET socketFD
);

int main() {

    // Initialize Winsock
    WSADATA ws;
    WSAStartup(MAKEWORD(2,2), &ws);

    // Create server socket
    SOCKET serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocketFD == INVALID_SOCKET){
        cout << "Socket creation failed\n";
        return 1;
    }

    // Create server address
    sockaddr_in serverAddress;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(2000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    int result = bind(
        serverSocketFD,
        (sockaddr*)&serverAddress,
        sizeof(serverAddress)
    );

    if(result == 0)
        cout << "Socket bound successfully\n";

    // Start listening
    int listenResult = listen(serverSocketFD, 10);

    if(listenResult == 0)
        cout << "Server listening on port 2000\n";

    // Accept incoming clients
    startAcceptingIncomingConnections(serverSocketFD);

    // Close server socket
    closesocket(serverSocketFD);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}

void startAcceptingIncomingConnections(SOCKET serverSocketFD) {
    while(true){
        AcceptedSocket* clientSocket = acceptIncomingConnection(serverSocketFD);
        acceptedSockets[acceptedSocketsCount++] = *clientSocket;
        receiveAndPrintIncomingDataOnSeparateThread(clientSocket);
    }
}

void receiveAndPrintIncomingDataOnSeparateThread(AcceptedSocket* clientSocket) {
    thread clientThread(receiveAndPrintIncomingData, clientSocket->acceptedSocketFD );
    clientThread.detach();
}

void receiveAndPrintIncomingData(SOCKET socketFD) {
    char buffer[1024];
    while(true){
        int amountReceived = recv(
            socketFD,
            buffer,
            1024,
            0
        );
        if(amountReceived > 0){
            buffer[amountReceived] = '\0';
            cout << buffer << endl;
            sendReceivedMessageToTheOtherClients(
                buffer,
                socketFD
            );
        }
        if(amountReceived == 0)
            break;
    }
    closesocket(socketFD);
}

void sendReceivedMessageToTheOtherClients(char* buffer,SOCKET socketFD) {
    for(int i = 0;i < acceptedSocketsCount; i++){
        if( acceptedSockets[i].acceptedSocketFD != socketFD ){
            send(
                acceptedSockets[i].acceptedSocketFD,
                buffer,
                strlen(buffer),
                0
            );
        }
    }
}

AcceptedSocket* acceptIncomingConnection(SOCKET serverSocketFD){
    sockaddr_in clientAddress;
    int clientAddressSize = sizeof(sockaddr_in);
    SOCKET clientSocketFD = accept(
        serverSocketFD,
        (sockaddr*)&clientAddress,
        &clientAddressSize
    );
    AcceptedSocket* acceptedSocket = new AcceptedSocket;
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = clientSocketFD != INVALID_SOCKET;
    acceptedSocket->error = clientSocketFD;
    return acceptedSocket;
}