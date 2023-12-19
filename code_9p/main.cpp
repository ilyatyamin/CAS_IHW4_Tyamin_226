#include "task.cpp"
#include <iostream>
#include <string>
#include <fstream>

/// Проверяет, является ли строка числом (можно ли ее прикастить к числу)
bool IsNumeric(const char *str) {
    std::string config(str);
    return !config.empty() && std::all_of(config.begin(), config.end(), ::isdigit);
}

std::pair<std::string, int> ReadConfigFile(const char* fileName) {
    std::ifstream file;
    file.open(fileName);

    std::string name;
    int countOfVisitors = 0;
    file >> countOfVisitors;
    file >> name;

    file.close();
    return {name, countOfVisitors};
}


int main(int argc, char *argv[]) {
    // по умолчанию main принимает 1 параметр: название запускаемого файла.
    // если передаем какой-то параметр, то их уже два
    if (argc <= 1) {
        int number;
        std::cout << "Введите количество посетителей: ";
        std::cin >> number;
        SolveTheTask(number, "log.txt");
    } else if (argc == 2 && IsNumeric(argv[1])) {
        // консоль все пишет в char*, а нам надо в int. пользуемся std::stoi
        SolveTheTask(std::stoi(argv[1]), "log.txt");
    } else if (argc == 2 && !IsNumeric(argv[1])) {
        // консоль все пишет в char*, а нам надо в int. пользуемся std::stoi
        std::pair<std::string, int> config = ReadConfigFile(argv[1]);
        SolveTheTask(config.second, config.first.c_str());
    } else if (argc == 3) {
        SolveTheTask(std::stoi(argv[1]), argv[2]);
    }
    return 0;
}
