#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <string>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    while (true) {
        SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(1280);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
            cout << "Не удалось подключиться к серверу." << endl;
            closesocket(client_socket);
            WSACleanup();
            continue;
        }

        int choice;
        cout << "\nВыберите действие:\n";
        cout << "1 - Просмотреть всех студентов\n";
        cout << "2 - Добавить студента\n";
        cout << "3 - Изменить студента\n";
        cout << "4 - Удалить студента\n";
        cout << "5 - Просмотреть студентов без оценки 3\n";
        cout << "0 - Выход\n";
        cout << "Ваш выбор: ";
        if (!(cin >> choice)) {
            cin.clear();
            string dummy;
            getline(cin, dummy);
            closesocket(client_socket);
            continue;
        }
        cin.ignore();

        string command;

        switch (choice) {
        case 1:
            command = "VIEWALL";
            break;

        case 2: {
            string fio, group;
            double stipend;
            cout << "Введите ФИО: ";
            getline(cin, fio);
            cout << "Введите номер группы: ";
            getline(cin, group);
            cout << "Введите размер стипендии: ";
            cin >> stipend;
            cin.ignore();
            cout << "Введите оценки через пробел: ";
            string grades;
            getline(cin, grades);

            replace(fio.begin(), fio.end(), ' ', '_');
            // Убираем лишние пробелы в grades (трим)
            command = "ADD " + fio + " " + group + " " + to_string(stipend);
            if (!grades.empty()) command += " " + grades;
            break;
        }

        case 3: {
            string fio, group;
            double stipend;
            cout << "Введите ФИО студента для изменения: ";
            getline(cin, fio);
            cout << "Введите новый номер группы: ";
            getline(cin, group);
            cout << "Введите новую стипендию: ";
            cin >> stipend;
            cin.ignore();
            cout << "Введите новые оценки через пробел: ";
            string grades;
            getline(cin, grades);

            replace(fio.begin(), fio.end(), ' ', '_');
            command = "EDIT " + fio + " " + group + " " + to_string(stipend);
            if (!grades.empty()) command += " " + grades;
            break;
        }

        case 4: {
            string fio;
            cout << "Введите ФИО студента для удаления: ";
            getline(cin, fio);
            replace(fio.begin(), fio.end(), ' ', '_');
            command = "DELETE " + fio;
            break;
        }

        case 5:
            command = "VIEW_NO3";
            break;

        case 0:
            cout << "Выход из программы." << endl;
            closesocket(client_socket);
            WSACleanup();
            return 0;

        default:
            cout << "Неверный выбор.\n";
            closesocket(client_socket);
            continue;
        }

        // Отправка команды
        send(client_socket, command.c_str(), (int)command.length(), 0);

        // Получение ответа
        char response[4096] = {};
        int bytes = recv(client_socket, response, sizeof(response) - 1, 0);
        if (bytes > 0) {
            response[bytes] = '\0';
            cout << "\nОтвет сервера:\n" << response << endl;
        }
        else {
            cout << "Нет ответа от сервера.\n";
        }

        closesocket(client_socket);
    }

    WSACleanup();
    return 0;
}
