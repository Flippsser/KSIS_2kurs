#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h> 
#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
    setlocale(LC_ALL, "ru");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cerr << "Ошибка инициализации Winsock" << endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Ошибка создания сокета" << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        cerr << "Ошибка преобразования IP-адреса" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    char buffer[1024];

    while (true) {
        string input;
        cout << "Введите строку: ";
        if (!getline(cin, input)) break;

        int len = static_cast<int>(input.size());
        int sent = sendto(clientSocket, input.c_str(), len, 0,
            reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
        if (sent == SOCKET_ERROR) {
            cerr << "Ошибка sendto: " << WSAGetLastError() << endl;
            continue;
        }

        int serverAddrLen = sizeof(serverAddr);
        int recvLen = recvfrom(clientSocket, buffer, sizeof(buffer) - 1, 0,
            reinterpret_cast<sockaddr*>(&serverAddr), &serverAddrLen);
        if (recvLen == SOCKET_ERROR) {
            cerr << "Ошибка recvfrom: " << WSAGetLastError() << endl;
            continue;
        }

        buffer[recvLen] = '\0';
        cout << "Ответ сервера:\n" << buffer << endl;
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
