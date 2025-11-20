#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <sstream>
#include <string>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

long long factorial(int x) {
    if (x < 0) return 0;
    long long res = 1;
    for (int i = 2; i <= x; ++i) {
        res *= i;
    }
    return res;
}

static string trim(const string& s) {
    size_t l = 0, r = s.size();
    while (l < r && isspace((unsigned char)s[l])) ++l;
    while (r > l && isspace((unsigned char)s[r - 1])) --r;
    return s.substr(l, r - l);
}

int main() {
    setlocale(LC_ALL, "ru");

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Ошибка инициализации WinSock\n";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Не удалось создать сокет\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1280);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Ошибка привязки сокета (bind)\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        cerr << "Ошибка listen\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    cout << "Сервер запущен. Ожидание подключения клиента...\n";

    sockaddr_in clientAddr{};
    int clientSize = sizeof(clientAddr);
    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Ошибка при подключении клиента\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Клиент подключен. Можно обрабатывать несколько запросов.\n";

    string recvbuf;
    char tmp[512];
    while (true) {
        recvbuf.clear();
        // читаем до '\n'
        while (true) {
            int bytes = recv(clientSocket, tmp, sizeof(tmp) - 1, 0);
            if (bytes <= 0) { cout << "Клиент отключился.\n"; goto cleanup; }
            tmp[bytes] = '\0';
            recvbuf += tmp;
            if (recvbuf.find('\n') != string::npos) break;
            if (recvbuf.size() > 1000000) break; // защита
        }

        // оставляем только до первого '\n' и тримим
        size_t pos = recvbuf.find('\n');
        if (pos != string::npos) recvbuf.resize(pos);
        recvbuf = trim(recvbuf);
        if (recvbuf == "q") break;

        
        int m = 0, n = 0;
        stringstream ss(recvbuf);
        if (!(ss >> m >> n)) {
            string err = "ERROR: expected two integers\n";
            send(clientSocket, err.c_str(), (int)err.size(), 0);
            cout << "Неправильный формат: \"" << recvbuf << "\"\n";
            continue;
        }
        if (m < 0 || n < 0) {
            string err = "ERROR: expected non-negative integers\n";
            send(clientSocket, err.c_str(), (int)err.size(), 0);
            cout << "Отрицательные значения: \"" << recvbuf << "\"\n";
            continue;
        }

        const int LIMIT = 20; // защита от слишком больших факториалов 
        if (m > LIMIT || n > LIMIT) {
            string err = "ERROR: argument too large\n";
            send(clientSocket, err.c_str(), (int)err.size(), 0);
            cout << "Слишком большое значение: \"" << recvbuf << "\"\n";
            continue;
        }

        long long fm = factorial(m);
        long long fn = factorial(n);
        long long sum = fm + fn;
        string out = to_string(sum) + "\n";
        send(clientSocket, out.c_str(), (int)out.size(), 0);
        cout << "Получено: \"" << recvbuf << "\" -> " << out;
    }

cleanup:
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
