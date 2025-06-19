#include "LibraryPerson.h"

LibraryPerson::LibraryPerson(const std::string& name,
    const std::string& password,
    int day1, int month1, int year1,
    int day2, int month2, int year2)
    :username(name),password(password), registerDate(day1, month1, year1), lastLoginDate(day2, month2, year2)
{
    // ��� ����� ������� ������� � registerDate, lastLoginDate, ������ �������
    // ���� ��� ����������� ������ � ������ ���������������
    // ���������� ������ 
    if (!validateUsername(this->username)) {
        throw std::invalid_argument("Username is not in a valid form!");
    }
    else if (!validatePassword(this->password)) {
        throw std::invalid_argument("Password is not in a valid form!");
    }
}

LibraryPerson::LibraryPerson(const LibraryPerson& other)
    :username(other.username), password(other.password),registerDate(other.registerDate),lastLoginDate(other.lastLoginDate)
{
    // other  � ������� �����, � ��� ������� ��������� this
    // ������ ����� ����� �� ������ ���, ������ �� ��������, ���� ������ copy ctor �� �������
}

LibraryPerson& LibraryPerson::operator=(const LibraryPerson& other)
{
    
    if (this != &other) {
        // ������ ������� ��������, ��� ���� ���� � ���������� ���� �� ����� �� �������� ctor
        // ���� �� ��� � ��-��������� �������, ��� �� ������ � �������� �����������, ����� �� � ���-������� �������
        try
        {
        std::string copyUsername = other.username; // ������� ctor �� string
        std::string copyPassword = other.password;
        Date copyRegister = other.registerDate;
        Date copyLastLogin = other.lastLoginDate;

        // ��� ��� ������ � ������, �� ��� ��
        // ����� ���� �� �� ����� ������������� �� ������ �����, ���� �� �� ����������� �� ���� ����
        username = std::move(copyUsername); // ���������� move operator=  �� ������
        password = std::move(copyPassword);
        registerDate = copyRegister;
        lastLoginDate = copyLastLogin;
        }
        catch (...)
        {
            // ��� ����� �� ����������� �� �����, �� ��� ��������� ������ �����
            throw;
        }

    }
    return *this;
}

LibraryPerson::LibraryPerson(LibraryPerson&& other) noexcept
    :username(std::move(other.username)), password(std::move(other.password))
{
    std::swap(this->registerDate, other.registerDate);
    std::swap(this->lastLoginDate, other.lastLoginDate);
}


LibraryPerson& LibraryPerson::operator=(LibraryPerson&& other) noexcept
{
    if (this != &other) {
        // �� ����������� ����, �������� ���� ���� �� ������ ������ �� other
        std::swap(username, other.username);
        std::swap(password, other.password);
        std::swap(registerDate, other.registerDate);
        std::swap(lastLoginDate, other.lastLoginDate);
    }
    return *this;
}

LibraryPerson::~LibraryPerson() noexcept
{
    // ������ ������ ���
}

bool LibraryPerson::setPassword(const std::string& newPas)
{
    if (newPas.empty()) {
        std::cout << "NewTitle cannot be empty!\n";
        return false;
    }

    if (newPas == password) {
        // ���� �������
        return true;
    }

    try
    {
        password = newPas; // ���� �� ������ �� � ��

    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool LibraryPerson::setNewUsername(const std::string& newUsername)
{
    if (newUsername.empty()) {
        std::cout << "NewTitle cannot be empty!\n";
        return false;
    }

    if (newUsername == username) {
        // ���� �������
        return true;
    }

    try
    {
        username = newUsername; // ���� �� ������ �� � ��

    }
    catch (...)
    {
        return false;
    }
    return true;
}

/*
��������� �� ������ :
����������(�������� �� ����� ������, ����.Google, GitHub, Microsoft) :

    ���. 6 �������

    ���� 1 ������ �����

    ���� 1 ����� �����

    ���� 1 �����

    ���� 1 ��������� ������(����. !@#$ % ^&*()_ += -)
*/

bool LibraryPerson::validatePassword(const std::string& pass) noexcept
{
    if (pass.length() < 6)
        return false;

    bool hasLower = false;
    bool hasUpper = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    for (char c : pass)
    {
        if (std::islower(c)) hasLower = true;
        else if (std::isupper(c)) hasUpper = true;
        else if (std::isdigit(c)) hasDigit = true;
        else hasSpecial = true; 
    }

    return hasLower && hasUpper && hasDigit && hasSpecial;
}

/*
��������� �� Username :
����������(������) :

    ����� 4 � 20 �������

    ���� �����, �����, ����� �����(_) � �����(.)

    �� ������� ��� �������� � ����� ��� ����� �����

    ��� �������������� ����� ��� ����� �����(����. .., __, ._ � �.�.)
*/

bool LibraryPerson::validateUsername(const std::string& name) noexcept
{
    // �������� �� �������
    if (name.length() < 4 || name.length() > 20)
        return false;

    // �� ������ �� ������� ��� �������� � �����
    if (name.front() == '.' || name.back() == '.')
        return false;

    // ��������� �������: �����, ����� � �����
    for (size_t i = 0; i < name.length(); ++i) {
        char c = name[i];
        if (!(std::isalnum(c) || c == '.'))
            return false;

        // �������� �� ��� �������������� �����
        if (i > 0 && name[i] == '.' && name[i - 1] == '.')
            return false;
    }

    return true;
}

void LibraryPerson::display(std::ostream& out) const
{
    out << "Username: " << username << "\n";
    out << "Password: ";
    // ���������� ����� �������� �� ������� �����
    for (size_t i = 0; i < password.size(); i++)
    {
        out << "*";
    }
    out << "\n";
    out << "Register date: ";
    registerDate.print();
    out << "Last login: ";
    lastLoginDate.print();
}

LibraryPerson::LibraryPerson() : username("No_username"), password("No_password"),
    registerDate(), lastLoginDate()
{
}

void LibraryPerson::serializeBase(std::ostream& out)const
{
    // �������� os �� � ������� �������� � ������� �����
    // �� ������ ��� �������� �� ���� �������� � ���� �� ����� � �� ����������� ��������� �� ����� �������� �� ��������
    // ����������� �� �������� ����

    // username
    size_t len;
    len = username.size();
    out.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (!out.good()) {
        throw std::ios_base::failure("Error writing size of username!");
    }
    out.write(username.c_str(), len);
    if (!out.good()) {
        throw std::ios_base::failure("Error writing username!");
    }

    // password
    len = password.size();
    out.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (!out.good()) {
        throw std::ios_base::failure("Error writing size of password!");
    }
    out.write(password.c_str(), len);
    if (!out.good()) {
        throw std::ios_base::failure("Error writing password!");
    }

    // registerDate
    registerDate.serialize(out);
    if (!out.good()) {
        throw std::ios_base::failure("Error with writing registerDate!");
    }

    // lastLoginDate
    lastLoginDate.serialize(out);
    if (!out.good()) {
        throw std::ios_base::failure("Error with writing lastLoginDate!");
    }
}

void LibraryPerson::deserializeBase(std::istream& is)
{
    if (!is.good()) {
        throw std::invalid_argument("Invalid stream before deserializingBase of user in Library!");
    }

    // username
    std::string tempUsername, tempPassword;
    Date tempRegisterDate, tempLastLoginDate;

    size_t size;
    is.read(reinterpret_cast<char*>(&size), sizeof(size));
    if (!is.good()) {
        throw std::ios_base::failure("Error with reading size of username!");
    }
    tempUsername.resize(size);
    if (size > 0) {
        is.read(&tempUsername[0], size);
    }
    if (!is.good()) {
        throw std::ios_base::failure("Error with reading username!");
    }

    // password
    is.read(reinterpret_cast<char*>(&size), sizeof(size));
    if (!is.good()) {
        throw std::ios_base::failure("Error with reading size of password!");
    }
    tempPassword.resize(size);
    if (size > 0) {
        is.read(&tempPassword[0], size);
    }
    if (!is.good()) {
        throw std::ios_base::failure("Error with reading password!");
    }

    // registerDate
    tempRegisterDate.deserialize(is);
    if (!is.good()) {
        throw std::ios_base::failure("Error with reading registerDate");
    }

    // lastLoginDate
    tempLastLoginDate.deserialize(is);
    if (!is.good()) {
        throw std::ios_base::failure("Error with reading lastLoginDate!");
    }

    // ������� ���������
    if (tempUsername.empty()) {
        throw std::invalid_argument("Username cannot be emtpy!");
    }
    if (tempPassword.empty()) {
        throw std::invalid_argument("Username cannot be emtpy!");
    }
    if (!tempRegisterDate.validateDay(tempRegisterDate.getDay(), tempRegisterDate.getMonth())) {
        throw std::invalid_argument("Invalid registerDate!");
    }
    if (!tempLastLoginDate.validateDay(tempLastLoginDate.getDay(), tempLastLoginDate.getMonth())) {
        throw std::invalid_argument("Invalid lastLoginDate!");
    }

    this->username = std::move(tempUsername);
    this->password = std::move(tempPassword);
    this->registerDate = tempRegisterDate;
    this->lastLoginDate = tempLastLoginDate;


}

std::ostream& operator<<(std::ostream& out, const LibraryPerson& person)
{
    person.display(out);
    return out;
}
