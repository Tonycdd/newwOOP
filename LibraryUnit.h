#pragma once
#include <string>
#include "Date.h"
#include <vector>
// ������� ����, ���������� ������, ���� ������ �� ���������� �� ������ � 
// ��������� ������, ��� ��� � ����������� �� �������� - 1900 �� �������� ��-����� �� ���� ������
// ������ �� ���� ���� � �� ��������� ��� ��������� �� ������ ������� � ������ ����������
// ����� �� ������ ����������� 
// ������������ � ���������� ���� 

// ��������� �� ������� ��������� ��������� �� �������� � �����

enum Type {
	UNKNOWN = 1,
	BOOK = 2,
	PERIODICALS = 3,
	SERIES = 4,
};

enum Rate : uint8_t
{
	VeryPoor = 1,
	Poor = 2,
	Average = 3,
	Good = 4,
	VeryGood = 5,
	Excellent = 6
};

enum Genre : uint16_t
{
	// ������ �������
	Fiction = 0,
	NonFiction,
	SciFi,
	Fantasy,
	Mystery,
	Thriller,
	Romance,
	Historical,

	// ����������
	Academic,
	Textbook,

	// ���������� �������
	News,
	Science,
	Sports,
	Technology,

	// ������������
	Other
};

class LibraryUnit
{
public:
	
	LibraryUnit(const std::string& title, const std::string& publishers, const std::string& description,
		int day, int month,int year,const Rate& rate, const Genre& genre);
	LibraryUnit(const LibraryUnit& other);

	// ����� ��������� ��������=, ���������� ������������ �� �� �������������
	LibraryUnit& operator=(const LibraryUnit& other);
	
	virtual LibraryUnit* clone() const = 0; // ����� ��������� ����� �� ��������, 
	// �������� ���������� �� �������������

	// ���� ���������
	LibraryUnit(LibraryUnit&& other) noexcept;
	LibraryUnit& operator=(LibraryUnit&& other) noexcept;

	virtual ~LibraryUnit() noexcept = default; //  ��������� ���������� �� �������� ��������� 
	// ��� ���������� �� ����������, ������ ������� � ��������� ��� ������� ����

	// �� � �������� ����� � ������������ �� ����� ������ � static
	static int generateNextNumber() {
		return ++LibraryUnit::uniqueNumber;
	}

	// ������
	bool setNewTitle(const std::string& newTitle);
	bool setNewPublishers(const std::string& newPub);
	bool setNewDescription(const std::string& desc);
	bool setNewDate(const Date& newDate);
	bool setNewRate(const Rate& newRate);
	bool setNewGenre(const Genre& newGenre);
	void setTotalCopies(unsigned newTotal);

	// ������
	inline const std::string& getTitle()const { return title; }; 
	inline const std::string& getPublishers()const { return publishers; }; 
	inline const std::string& getDescription()const { return description; }; 
	inline Date getDate()const { return date; };// ����� - �� �������� ����������, �� � ����� �����
	inline Rate getRate()const { return rate; };// �����, �� �������� ������������
	inline static int getUniqueNumber() { return LibraryUnit::uniqueNumber; }; // ���� � ��� ���� �����, ��� ���� � ������
	inline Genre getGenre()const { return genre; };// ������� ����� 
	inline int getId()const { return id; };
	inline bool isAvailable()const { return takenCopies < totalCopies; };
	inline unsigned int getAvailableCopies()const { return totalCopies - takenCopies; };
	inline unsigned int getTotalCopies()const { return totalCopies; };
	
	// ��������� display
	virtual void display(std::ostream& os = std::cout) const = 0; // ���� �� ��� ������������� ;)
	// ���� �� �� �������� �� ���� operator<< - �� �������� ���� �������

	bool equals(const LibraryUnit& other)const;
	// ������� �� ����������� ������ ������� ����� �� �������� ���������
	bool incrementTakenCopies();
	bool decrementTakenCopies();

	// ���� �� �������� �� ������ ������ � ���� �� ������� ����� � ����� �� ����� �� ���
	virtual size_t getCount() const = 0;
	virtual void serialize(std::ostream& os)const = 0;
	virtual void deserialize(std::istream& is) = 0;
	virtual Type getType()const = 0;

	// �� ������� �� isnn � isbn
	virtual std::vector<std::string> getIdentifiers()const = 0;

	void printBase(std::ostream& out=std::cout) const;

	virtual std::vector<std::string> getKeyWordsForFind()const = 0;

	// �� change - �������� ������� - � ���� �������
	virtual bool change();

protected:
	
	// ���� ��� ����� ���� ����������� �� ��������, �� ��� �� ���������� ���� � ������������ ������, ���� �� ����
	// �� ����������� � ������� ����������� �� ��������

	LibraryUnit();
protected:

	// �� ���������
	LibraryUnit(std::istream& is) {
		deserializeBaseUnit(is);
	}

	void serializeBaseUnit(std::ostream& os)const;
	void deserializeBaseUnit(std::istream& is);

protected:
	// � ������ � �� � � ���������
	std::string title;// ��������
	std::string publishers;// �����������
	std::string description;// ������ ��������
	Date date;// ���� �� ��������
	Rate rate;// ������� - �������� �� 5, ����� ���� �� � ������� � ���� 8 ����� ��������� ���������
	Genre genre; // ����� �� 2 ������ ��������� ��� �� �������� �����������
	static int uniqueNumber;// �� ���������� �� �������� ������
	unsigned int totalCopies;
	unsigned int takenCopies;
	int id; // ���������� ����� � ������������
};

// �������� ���� ������ �������������, �� �� ������ �� ���� friend, ��� ���� ���� ����� �������� �������
std::ostream& operator<<(std::ostream& os, const LibraryUnit& lib);
bool operator==(const LibraryUnit& lhs, const LibraryUnit& rhs);