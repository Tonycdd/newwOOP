#pragma once
#include "LibraryPerson.h"

class Administrator : public LibraryPerson
{
public:
	// ���������� ������ ������ 6-����
	// �� ���������
	Administrator(std::istream& is) : LibraryPerson(is) {
		deserializeAdministratorUnit(is);
	}
	Administrator(const std::string& user, const std::string& pass,
		int day1, int month1, int year1,
		int day2, int month2, int year2,
		const std::string& email);
	// ������ �� �� ��������� �� emptyCase ������
	Administrator(const std::string& user, const std::string& pass,
		const Date&, const Date&,
		const std::string& email);

	Administrator(const Administrator& other);
	Administrator& operator=(const Administrator& other);
	Administrator(Administrator&& other) noexcept;
	Administrator& operator=(Administrator&& other) noexcept;

	// ������
	const std::string& getEmail()const { return email; };

	// ������� ���������� �� email
	static bool validateEmail(const std::string& email);

	virtual ~Administrator() noexcept override;
	virtual LibraryPerson* clone()const override;
	virtual void display(std::ostream& out = std::cout)const override;

	inline virtual TypeOfReader getType()const override { return TypeOfReader::ADMINISTRATOR; };
	virtual void deserialize(std::istream& is) override;
	virtual void serialize(std::ostream& out)const override;

	virtual void borrow(LibraryUnit* unit);
	virtual bool returnUnit(int id)override;
	
	// �������� ����� �� ������������ ����� �� ���������
	static Administrator* createInteractively();
protected:
	Administrator();
protected:
	void deserializeAdministratorUnit(std::istream& is);
	void serializeAdministratorUnit(std::ostream& out)const;
private:
	std::string email;
};
std::ostream& operator<<(std::ostream& out, const Administrator& other);
