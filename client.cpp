#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

void listenAndPrint(SOCKET socketFD);
void startListeningAndPrintMessagesOnNewThread(SOCKET socketFD);
void readConsoleEntriesAndSendToServer(SOCKET socketFD);

int main() {

    // Initialize Winsock
    WSADATA ws;
    WSAStartup(MAKEWORD(2,2), &ws);

    // Create socket
    SOCKET socketFD = socket(AF_INET, SOCK_STREAM, 0);

    if(socketFD == INVALID_SOCKET){
        cout << "Socket creation failed\n";
        return 1;
    }

    // Server address
    sockaddr_in serverAddress;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(2000);
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    // Connect to server
    int result = connect(
        socketFD,
        (sockaddr*)&serverAddress,
        sizeof(serverAddress)
    );

    if(result == 0)
        cout << "Connection was successful\n";

    // Start listening thread
    startListeningAndPrintMessagesOnNewThread(socketFD);

    // Read user input and send messages
    readConsoleEntriesAndSendToServer(socketFD);

    // Close socket
    closesocket(socketFD);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}

#include <ctime>
#include <iomanip>
#include <sstream>

void readConsoleEntriesAndSendToServer(SOCKET socketFD) {

    string name;

    cout << "Please enter your name:\n";
    getline(cin, name);

    cout << "Type messages (type exit to quit)\n";

    while(true){

        string line;
        getline(cin, line);

        if(line == "exit")
            break;

        // Get current time
        time_t now = time(0);
        tm* localTime = localtime(&now);

        // Format time
        stringstream timeStream;

        timeStream << setfill('0')
                   << setw(2) << localTime->tm_hour //setw --> set width
                   << ":"
                   << setw(2) << localTime->tm_min;

        string currentTime = timeStream.str();

        // Final message
        string message =
            "[" + currentTime + "] "
            + name + ": "
            + line;

        send(
            socketFD,
            message.c_str(),
            message.length(),
            0
        );
    }
}

void startListeningAndPrintMessagesOnNewThread(SOCKET socketFD) {
    thread listeningThread(listenAndPrint, socketFD);
    listeningThread.detach();  //detach from main thread and run independently
}

void listenAndPrint(SOCKET socketFD) {
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
            cout << "Response was: " << buffer << endl;
        }if(amountReceived == 0)
            break;
    }

    closesocket(socketFD);
}