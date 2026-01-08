#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

struct Student {
    string fio;
    string group;
    double stipend;
    vector<int> grades;
};

vector<Student> students = {
    {"Иванов Иван Иванович", "101", 120.50, {4,5,5,4}},
    {"Петров Петр Петрович", "102", 100.00, {3,4,5,4}},
    {"Сидоров Сидор Сидорович", "103", 150.00, {5,5,5,5}},
    {"Алексеева Анна Сергеевна", "104", 130.00, {4,4,4,4}},
    {"Кузнецов Николай Павлович", "105", 110.00, {5,3,4,5}}
};

enum CommandType { VIEWALL, ADD, EDIT, DELETE_ST, VIEW_NO3, UNKNOWN };

CommandType getCommandType(const string& cmd) {
    if (cmd == "VIEWALL") return VIEWALL;
    if (cmd == "ADD") return ADD;
    if (cmd == "EDIT") return EDIT;
    if (cmd == "DELETE") return DELETE_ST;
    if (cmd == "VIEW_NO3") return VIEW_NO3;
    return UNKNOWN;
}

string viewAllStudents() {
    stringstream result;
    for (const auto& st : students) {
        result << "ФИО: " << st.fio
            << " | Группа: " << st.group
            << " | Стипендия: " << st.stipend
            << " | Оценки: ";
        for (int g : st.grades) result << g << " ";
        result << "\n";
    }
    return result.str();
}

string viewStudentsNo3() {
    stringstream result;
    for (const auto& st : students) {
        if (st.grades.empty()) continue;
        if (all_of(st.grades.begin(), st.grades.end(), [](int g) { return g != 3; })) {
            result << "ФИО: " << st.fio
                << " | Группа: " << st.group
                << " | Стипендия: " << st.stipend
                << " | Оценки: ";
            for (int g : st.grades) result << g << " ";
            result << "\n";
        }
    }
    if (result.str().empty()) return "Нет студентов без оценки 3.\n";
    return result.str();
}

string processCommand(const string& input) {
    istringstream iss(input);
    string cmd;
    if (!(iss >> cmd)) return "Пустая команда.\n";

    CommandType type = getCommandType(cmd);

    switch (type) {
    case VIEWALL:
        return viewAllStudents();

    case VIEW_NO3:
        return viewStudentsNo3();

    case ADD: {
        // Собираем все оставшиеся токены
        vector<string> tokens;
        string t;
        while (iss >> t) tokens.push_back(t);

        if (tokens.size() < 3) return "Ошибка: недостаточно данных для добавления.\n";

        Student st;
        st.fio = tokens[0];
        replace(st.fio.begin(), st.fio.end(), '_', ' ');
        st.group = tokens[1];
        try {
            st.stipend = stod(tokens[2]);
        }
        catch (...) {
            return "Ошибка: неверный формат стипендии.\n";
        }
        for (size_t i = 3; i < tokens.size(); ++i) {
            try {
                st.grades.push_back(stoi(tokens[i]));
            }
            catch (...) {
                // игнорируем некорректные оценки
            }
        }
        students.push_back(st);
        return "Студент добавлен.\n";
    }

    case EDIT: {
        vector<string> tokens;
        string t;
        while (iss >> t) tokens.push_back(t);

        if (tokens.size() < 3) return "Ошибка: недостаточно данных для редактирования.\n";

        string fio = tokens[0];
        replace(fio.begin(), fio.end(), '_', ' ');

        for (auto& st : students) {
            if (st.fio == fio) {
                st.group = tokens[1];
                try {
                    st.stipend = stod(tokens[2]);
                }
                catch (...) {
                    return "Ошибка: неверный формат стипендии.\n";
                }
                st.grades.clear();
                for (size_t i = 3; i < tokens.size(); ++i) {
                    try {
                        st.grades.push_back(stoi(tokens[i]));
                    }
                    catch (...) {
                        // игнорируем некорректные оценки
                    }
                }
                return "Студент обновлён.\n";
            }
        }
        return "Студент не найден.\n";
    }

    case DELETE_ST: {
        string fio_token;
        if (!(iss >> fio_token)) return "Ошибка: укажите ФИО для удаления.\n";
        string fio = fio_token;
        replace(fio.begin(), fio.end(), '_', ' ');

        auto it = remove_if(students.begin(), students.end(),
            [&](const Student& s) { return s.fio == fio; });
        if (it != students.end()) {
            students.erase(it, students.end());
            return "Студент удалён.\n";
        }
        return "Студент не найден.\n";
    }

    default:
        return "Неизвестная команда.\n";
    }
}

DWORD WINAPI handleClient(LPVOID socket_ptr) {
    SOCKET client_socket = *((SOCKET*)socket_ptr);
    char buffer[4096] = {};

    while (true) {
        int received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) break;
        buffer[received] = '\0';
        string input(buffer);
        string response = processCommand(input);
        send(client_socket, response.c_str(), (int)response.length(), 0);
    }

    cout << "Клиент отключился." << endl;
    closesocket(client_socket);
    return 0;
}

int main() {
    setlocale(LC_ALL, "Russian");
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1280);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);

    cout << "Сервер запущен. Ожидание подключений..." << endl;

    while (true) {
        sockaddr_in client_addr{};
        int client_size = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_size);
        if (client_socket == INVALID_SOCKET) continue;
        DWORD thread_id;
        // Передаём копию сокета в поток
        SOCKET* pClient = new SOCKET(client_socket);
        CreateThread(NULL, 0, handleClient, pClient, 0, &thread_id);
    }

    WSACleanup();
    return 0;
}
