#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <map>
#include <set>
#include <sstream>
#include <cctype>
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

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Ошибка создания сокета" << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8888);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Ошибка привязки сокета" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "UDP сервер запущен на порту 8888...\n";

    sockaddr_in clientAddr{};
    char buffer[1024];

    const set<char> letters = { 'W','I','N','D','O','S' };

    while (true) {
        int clientAddrLen = sizeof(clientAddr);
        memset(buffer, 0, sizeof(buffer));

        int recvLen = recvfrom(serverSocket, buffer, sizeof(buffer) - 1, 0,
            reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
        if (recvLen == SOCKET_ERROR) {
            cerr << "Ошибка recvfrom: " << WSAGetLastError() << endl;
            continue;
        }

        buffer[recvLen] = '\0';
        string input(buffer);
        cout << "Получено: " << input << endl;

        map<char, int> counts;
        for (char c : input) {
            unsigned char uc = static_cast<unsigned char>(c);
            char up = static_cast<char>(toupper(uc));
            if (letters.count(up)) {
                counts[up]++; 
            }
        }

        bool allFound = true;
        for (char c : letters) {
            if (counts[c] == 0) {
                allFound = false;
                break;
            }
        }

        ostringstream oss;
        if (allFound) {
            oss << "Все буквы из WINDOWS найдены.\nКоличество вхождений:\n";
        }
        else {
            oss << "Не все буквы из WINDOWS присутствуют в строке.\nКоличество найденных букв:\n";
        }

        for (char c : letters) {
            oss << c << " : " << counts[c] << "\n";
        }

        string response = oss.str();
        int sent = sendto(serverSocket, response.c_str(), static_cast<int>(response.size()), 0,
            reinterpret_cast<sockaddr*>(&clientAddr), clientAddrLen);
        if (sent == SOCKET_ERROR) {
            cerr << "Ошибка sendto: " << WSAGetLastError() << endl;
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
