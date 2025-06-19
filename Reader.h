#pragma once
#include "LibraryPerson.h"
#include "Book.h"
#include "Periodicals.h"
#include "Series.h"

// ���� ������������ ���������, ����� �� ���� �������� �� ������� ����  � �� ����� ���� � ����� � 
// ������� �������
struct LoanInfo {
	// ������ unit �����, ������ �� �� ������, �� �� ���� ������ ���������
	// �� �� ����� �� unit, � ���� ���� �������� ��� ����
	const LibraryUnit* unit; // �������� ��� ������� �� ������������
	Date borrowDate; // ���� �� �������
	Date returnDate; // ���� �� ���������
	bool returned; // ���� � ������� ����
	
	LoanInfo() = default;
	LoanInfo(const LibraryUnit* unit, const Date& borrow, const Date& due,bool isReturned = false) :
		unit(unit), borrowDate(borrow), returnDate(due), returned(false) {};

	inline void changeReturned() {
		returned = !returned; // ������ �����������
	}

	void serialize(std::ostream& out)const {
		// unit  ���� � ������������, ���� �� �� �� ����� ������!!!
		borrowDate.serialize(out);
		if (!out.good()) {
			throw std::ios_base::failure("Error with writing borrowDate in LoanInfo!");
		}
		returnDate.serialize(out);
		if (!out.good()) {
			throw std::ios_base::failure("Error with writing returnDate in LoanInfo!");
		}
		out.write(reinterpret_cast<const char*>(&returned), sizeof(returned));
		if (!out.good()) {
			throw std::ios_base::failure("Error with writing returned state in LoanInfo!");
		}
	}

	void deserialize(std::istream& is) {
		if (!is.good()) {
			throw std::invalid_argument("Error with stream before reading in LoanInfo!");
		}
		borrowDate.deserialize(is);
		if (!is.good()) {
			throw std::ios_base::failure("Error with reading borrowDate in LoanInfo!");
		}
		returnDate.deserialize(is);
		if (!is.good()) {
			throw std::ios_base::failure("Error with reading returnDate in LoanInfo!");
		}
		is.read(reinterpret_cast<char*>(&returned), sizeof(returned));
		if (!is.good()) {
			throw std::ios_base::failure("Error with reading returned state in LoanInfo!");
		}
	}

};

class Reader: public LibraryPerson
{
public:
	// ���������� ������ ������ 6-����
	// 
	//�� ���������
	Reader(std::istream& is):LibraryPerson(is)
	{
		deserializeReaderUnit(is);
	}

	Reader(const std::string& name, const std::string& pass, int day1, int month1, int year1,
		int day2, int month2, int year2, const std::vector<LibraryUnit*>& v = {});
	Reader(const Reader& other);
	Reader& operator=(const Reader& other);
	Reader(Reader&& other)noexcept;
	Reader& operator=(Reader&& other)noexcept;
	virtual ~Reader() noexcept override;

	// �������������� �� ����������� �������
	virtual LibraryPerson* clone()const override;
	virtual void display(std::ostream& out = std::cout)const override;

	// ������
	inline const std::vector<LibraryUnit*>& getUnits()const { return units; };
	inline std::vector<LibraryUnit*>& getUnits() { return units; }; // ������ �� �� ��������� � ���������, �� � ����� �������� �����
	inline const std::vector<LoanInfo>& getHistory()const { return history; };

	// �������� � �������� �� �������� �� ���� ������� � ������� ������� � �������
	void addNewUnit(LibraryUnit* unit);
	virtual void borrow(LibraryUnit* unitToBorrow)override;
	virtual bool returnUnit(int id)override;

	// ������������, ��������������
	virtual void serialize(std::ostream& out)const override;
	virtual void deserialize(std::istream& is)override;
	inline virtual TypeOfReader getType()const override { return TypeOfReader::READER; };

	// ��������� ������ �� ������� ���� �� �������������, ����� �� ��������
	bool hasOverdueBooks(const Date& today)const;
	unsigned int borrowedLastMonth(const Date& today)const;
	int monthsSinceLastBorrow(const Date& today)const;
	bool hasBorrowedUnit(unsigned int unitId)const ; // ����� � ��������� - ����, ����� �� ���� ����� ������ �� ���� ����������

	// �������� ����� �� ������������ ����� �� ���������
	static Reader* createInteractively();

protected:
	Reader();

protected:
	// ����� �� ����������
	// ������ �� �� �� �����
	void serializeReaderUnit(std::ostream& out)const;
	void deserializeReaderUnit(std::istream& is);

private:
	std::vector<LibraryUnit*> units; // ������������ ���������, ����� ����� ������� � ���� ���� �������
	std::vector<LoanInfo> history; // ����� �� units �� ���� ������� � history
};

std::ostream& operator<<(std::ostream& out, const Reader& other);
