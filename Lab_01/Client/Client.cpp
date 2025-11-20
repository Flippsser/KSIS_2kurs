#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

int main() {
    setlocale(LC_ALL, "ru");

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Ошибка инициализации WinSock\n";
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Не удалось создать сокет\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1280);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Не удалось подключиться к серверу\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Подключение к серверу успешно. Введите несколько запросов или 'q' для выхода.\n";

    while (true) {
        cout << "Введите два целых числа m и n через пробел (или q для выхода): ";
        string a, b;
        if (!(cin >> a)) break;
        if (a == "q") break;
        if (!(cin >> b)) break;
        if (b == "q") break;

        string msg = a + " " + b + "\n";                // отправляем с '\n'
        int sent = send(clientSocket, msg.c_str(), (int)msg.size(), 0); 
        if (sent == SOCKET_ERROR) {
            cerr << "Ошибка при отправке\n";
            break;
        }

        // читаем ответ до '\n'
        string resp;
        char buf[256];
        while (true) {
            int bytes = recv(clientSocket, buf, sizeof(buf) - 1, 0);
            if (bytes <= 0) { cerr << "Ошибка при получении ответа или сервер закрыл соединение\n"; break; }
            buf[bytes] = '\0';
            resp += buf;
            if (resp.find('\n') != string::npos) break;
        }
        if (resp.empty()) break;
        size_t p = resp.find('\n');
        if (p != string::npos) resp.resize(p);
        cout << "Ответ сервера: " << resp << endl;
        cout << "-----------------------------\n";
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
