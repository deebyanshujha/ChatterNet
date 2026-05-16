#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <mutex>
#include<vector>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct AcceptedSocket {
    SOCKET acceptedSocketFD;
    sockaddr_in address;
    bool acceptedSuccessfully;
    int error;
    string name;
};

vector<AcceptedSocket> acceptedSockets;
mutex acceptedSocketsMutex;

AcceptedSocket* acceptIncomingConnection(SOCKET serverSocketFD);

void startAcceptingIncomingConnections(SOCKET serverSocketFD);

void receiveAndPrintIncomingData(SOCKET socketFD);

void receiveAndPrintIncomingDataOnSeparateThread(AcceptedSocket* clientSocket);

void broadcastMessage(string message);

void sendReceivedMessageToTheOtherClients(char* buffer, SOCKET socketFD);

void removeClient(SOCKET socketFD);

int main() {
    // Initialize Winsock
    WSADATA ws;
    if(WSAStartup(MAKEWORD(2,2), &ws) != 0){
        cout<<"WSAStartup failed!";
        return 1;
    }

    // Create server socket
    SOCKET serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocketFD == INVALID_SOCKET){
        cout << "Socket creation failed\n";
        cout << "Error code: "<< WSAGetLastError()<< endl;
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

    if(result == SOCKET_ERROR){
        cout << "Bind failed\n";
        cout << "Error code: "<< WSAGetLastError()<< endl;
        return 1;
    }else{
        cout<<"Bounded successfully!"<<endl;
    }

    // Start listening
    int listenResult = listen(serverSocketFD, 10);

    if(listenResult == SOCKET_ERROR){
        cout << "listen failed\n";
        cout << "Error code: "<< WSAGetLastError()<< endl;
        return 1;
    }else{
        cout << "Server listening on port 2000\n";
    }

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
        acceptedSocketsMutex.lock();
        acceptedSockets.push_back(*clientSocket);
        acceptedSocketsMutex.unlock();
        receiveAndPrintIncomingDataOnSeparateThread(clientSocket);
    }
}

void receiveAndPrintIncomingDataOnSeparateThread(AcceptedSocket* clientSocket) {
    thread clientThread(receiveAndPrintIncomingData, clientSocket->acceptedSocketFD );
    clientThread.detach();
}

void receiveAndPrintIncomingData(SOCKET socketFD) {
    char buffer[1024];
    bool nameReceived = false;
    string clientName;
    while(true){
        int amountReceived = recv(
            socketFD,
            buffer,
            1024,
            0
        );
        if(amountReceived > 0){
            buffer[amountReceived] = '\0';
            // First message = username
            if(!nameReceived){
                clientName = buffer;
                // Store name in acceptedSockets
                acceptedSocketsMutex.lock();
                for(int i = 0; i < acceptedSockets.size();i++){
                    if(acceptedSockets[i].acceptedSocketFD == socketFD){
                        acceptedSockets[i].name = clientName;
                    }
                }
                acceptedSocketsMutex.unlock();
                string joinMessage =clientName + " joined the chat!";
                cout << joinMessage << endl;
                broadcastMessage(joinMessage);
                nameReceived = true;
                continue;
            }
            cout << buffer << endl;
            sendReceivedMessageToTheOtherClients(buffer,socketFD);
        }
        if(amountReceived == 0){
            string leaveMessage = clientName + " left the chat!";
            cout << leaveMessage << endl;
            broadcastMessage(leaveMessage);
            removeClient(socketFD);
            break;
        }
    }

    closesocket(socketFD);
}

void sendReceivedMessageToTheOtherClients(char* buffer,SOCKET socketFD) {
    acceptedSocketsMutex.lock();
    for(int i = 0;i < acceptedSockets.size(); i++){
        if( acceptedSockets[i].acceptedSocketFD != socketFD ){
            send(
                acceptedSockets[i].acceptedSocketFD,
                buffer,
                strlen(buffer),
                0
            );
        }
    }
    acceptedSocketsMutex.unlock();
}

AcceptedSocket* acceptIncomingConnection(SOCKET serverSocketFD){
    sockaddr_in clientAddress;
    int clientAddressSize = sizeof(sockaddr_in);
    SOCKET clientSocketFD = accept(
        serverSocketFD,
        (sockaddr*)&clientAddress,
        &clientAddressSize
    );
    if(clientSocketFD == INVALID_SOCKET){
        cout << "Accept failed\n";
        cout << "Error code: "<< WSAGetLastError()<< endl;
    }
    AcceptedSocket* acceptedSocket = new AcceptedSocket;
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = clientSocketFD != INVALID_SOCKET;
    acceptedSocket->error = clientSocketFD;
    return acceptedSocket;
}

void broadcastMessage(string message){
    acceptedSocketsMutex.lock();
    for(int i = 0;i < acceptedSockets.size();i++){
        send(
            acceptedSockets[i].acceptedSocketFD,
            message.c_str(),
            message.length(),
            0
        );
    }
    acceptedSocketsMutex.unlock();
}

void removeClient(SOCKET socketFD){
    acceptedSocketsMutex.lock();
    acceptedSockets.erase(
        remove_if(
            acceptedSockets.begin(),
            acceptedSockets.end(),
            [socketFD](AcceptedSocket& socket){
                return socket.acceptedSocketFD == socketFD;
            }
        ),
        acceptedSockets.end()
    );
    acceptedSocketsMutex.unlock();
}