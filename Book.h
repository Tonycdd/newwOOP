#pragma once
#include "LibraryUnit.h"
#include <vector> // ��������� � � �� �� �������� �� ������� ����,
// ��� ��� ����� ���������� ������� �� ���������, ������� � ��.

class Book : public virtual LibraryUnit // ������ �� �������� ����������� ������� ��� ������������� �����������
{
public:
	// ������ 6-����
	
	explicit Book(std::istream& is)
		: LibraryUnit(is)
	{
		deserializeBookUnit(is);
	}

	Book(const std::string& title,
		const std::string& publishers,
		const std::string& description,
		int day, int month, int year,
		const Rate& rate,
		const Genre& genre,
		const std::string& author,
		const std::string& isbn = "",
		const std::vector<std::string>& v = {}); // �������������� �����������
	Book(const Book& other);
	Book& operator= (const Book& other);
	Book(Book&& other)noexcept;
	Book& operator=(Book&& other)noexcept;
	virtual ~Book()noexcept override;

	// ������
	// ������� �� ���������� �� �������� �������� � � �����, �� �� �� ��������� ������� �� ������� ����
	inline const std::string& getAuthor()const { return author; };
	inline const std::string& getISBN()const { return ISBN; };
	inline const std::vector<std::string>& getKeyWords()const { return keyWords; };
	
	// ������
	bool setNewAuthor(const std::string& newAth);
	bool setNewKeyWords(const std::vector<std::string>& newKeyWords);
	bool setNewISBN(const std::string& newISBN);


	// �������������� �����-����������� ������
	// ��������� ���� ctor 
	// ����� �� ������������ ���� ����� � 
	inline virtual LibraryUnit* clone() const override { return new Book(*this); };
	inline virtual void display(std::ostream& os = std::cout)const override;
	inline virtual size_t getCount()const override { return counter; };
	inline static size_t getBookCount(){ return counter; };
	
	virtual void serialize(std::ostream& os)const override;
	virtual void deserialize(std::istream& is) override;
	inline virtual Type getType()const override { return Type::BOOK; };
	
	// �� list info � ���������
	inline virtual std::vector<std::string> getIdentifiers()const override {
		return {getISBN()}; 
	}

	// ������� �� �������� �� ������� ����
	void addKeyWords(const std::string& key);

	virtual std::vector<std::string> getKeyWordsForFind()const override { return keyWords; };

	// �������� ����� �� ������������ ����� �� ���������
	static Book* createInteractively();

	virtual bool change()override;
protected:

	Book();
	bool changeBookPart();
protected:
	// ����������� ���� �������� ������ � ������a��������
	void serializeBookUnit(std::ostream& os)const;
	void deserializeBookUnit(std::istream& is);

	void printBook(std::ostream& out = std::cout)const;
private:
	std::string author;
	std::vector<std::string> keyWords;
	std::string ISBN; // �� � ������������
	// ���������� ���� ����� �� �������� �������

	static size_t counter; // ���� ����� ��������� �����
};

std::ostream& operator<<(std::ostream& out, const Book& book);
