#pragma once
#include <string>
#include <exception>
#include <stdexcept>
#include "Date.h"
#include <iostream>
#include "LibraryUnit.h"
// �������� ���� ����� ����, ����� ������ ������� �� ���������, ������ �� ���� ����������
// ��� ���� ������ ����������� � ���� �� ���� �����������
// � ������ � ��-����� �������� �� ���� ���� �� ���� � ������������, ���� ���������� �� ������ 
// ����� ����� ���

enum TypeOfReader {
	UNKOWN = 1,
	READER = 2,
	ADMINISTRATOR = 3
};

class LibraryPerson
{
public:
	// ������ 6-����
	LibraryPerson(const std::string& name, const std::string& password, int day1, int month1, int year1, int day2, int month2, int year2);
	LibraryPerson(const LibraryPerson& other);
	LibraryPerson& operator=(const LibraryPerson& other);
	LibraryPerson(LibraryPerson&& other) noexcept;
	LibraryPerson& operator=(LibraryPerson&& other) noexcept;
	virtual ~LibraryPerson() noexcept;
	
	inline const std::string& getUsername()const { return username; }; 
	// �� � ����� ���� �� ������ ������ �� ��������, ���� �� �� ������������ �����
	
	inline Date getRegisterDate()const { return registerDate; }; // �� �����, �� � �����
	inline Date getLastLoginDate()const { return lastLoginDate; }; // �� �����, �� � �����

	//setters
	bool setPassword(const std::string& newPas);
	bool setNewUsername(const std::string& newUsername);

	virtual LibraryPerson* clone() const = 0 ; // ����� ��������� �������

	// ��� �������, ����� ������ ��������� ����� � ��������
	// ��� �� ������ ������ �� �� �� � �������� ������
	// � ������ ������� �� ����������� �� ���������� �� �����, ��� ��

	static bool validatePassword(const std::string& pass) noexcept;
	static bool validateUsername(const std::string& user) noexcept;

	virtual void display(std::ostream& out = std::cout)const = 0; 
	
	// ������� �������
	virtual void serialize(std::ostream& out)const = 0; 
	virtual void deserialize(std::istream& is) = 0;
	virtual TypeOfReader getType() const = 0;
	
	virtual void borrow(LibraryUnit* unit) = 0;
	virtual bool returnUnit(int id) = 0;

protected:
	LibraryPerson();
protected:
	// �� ��������� 
	LibraryPerson(std::istream& is) {
		deserializeBase(is);
	}

	void deserializeBase(std::istream& is);
	void serializeBase(std::ostream& out)const;

	// protected e!
	inline const std::string& getPassword()const { return password; };

	friend class LibrarySystem; // ������ �� �� ���� ����� �� ������������� � ���������

protected:
	std::string username; // ������������� ���
	std::string password; // ������, �� ������ �� ��� ������� ���������� �� ��������� 
	Date registerDate; // ���� �� �����������
	Date lastLoginDate; // �������� �������
};

std::ostream& operator<<(std::ostream& out, const LibraryPerson& person);