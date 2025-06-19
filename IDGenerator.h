#pragma once
#include <fstream>
#include <string>

class IDGenerator
{
private:
    static unsigned int lastId;

public:
    // ����� ���� �������� ID
    static int getNextId() {
        return ++lastId;
    }

    // ����� �������� (��� ��������� �� ������ �� ����)
    static void setLastId(int id) {
        if (id > lastId)
            lastId = id;
    }

    // ��������� �� ������� ����
    static void loadLastIdFromFile(const std::string& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open()) return;

        in.read(reinterpret_cast<char*>(&lastId), sizeof(lastId));
        if (!in) {
            throw std::ios_base::failure("Error with file in IDGenerator!");
            in.close();
        }
        in.close();
    }

    // ����� � ������� ����
    static void saveLastIdToFile(const std::string& path) {
        std::ofstream out(path, std::ios::binary);
        if (!out.is_open()) return;

        out.write(reinterpret_cast<const char*>(&lastId), sizeof(lastId));
        if (!out) {
            throw std::ios_base::failure("Error with file in IDGenerator!");
            out.close();
        }
        out.close();
    }

    // ������ �� �������� (��� �� �� �����������)
    static int getLastId() {
        return lastId;
    }
};

