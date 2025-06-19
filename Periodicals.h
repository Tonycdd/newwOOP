#pragma once
#include "LibraryUnit.h"
#include "Article.h"
#include <vector>
#include <iostream>

class Periodicals : public virtual LibraryUnit // ������ �� �������� ����������� ������� ��� ������������� �����������
{
public:
	// ������� ������ 6-
	// 
	// �� ���������
	Periodicals(std::istream& is) : LibraryUnit(is)
	{
		deserializePeriodicalsUnit(is);
	}
	Periodicals(const std::string& title,
		const std::string& publishers,
		const std::string& description,
		const Genre& genre,
		const Rate&  rate,
		int month, int year,
		unsigned int issueNumber,
		const std::string& issn = "",
		const std::vector<Article>& articles = {});
	Periodicals(const Periodicals& other);
	Periodicals& operator=(const Periodicals& other);
	Periodicals(Periodicals&& other)noexcept;
	Periodicals& operator=(Periodicals&& other)noexcept;

	virtual ~Periodicals() noexcept override;
	inline virtual LibraryUnit* clone() const override { return new Periodicals(*this); };
	virtual void display(std::ostream& out = std::cout)const override;
	inline virtual size_t getCount()const override { return counter; };

	static size_t getPeriodicalsCount() { return counter; };

	// ������
	inline unsigned int getIssueNumber()const { return issueNumber; };
	inline const std::string& getISSN()const { return ISSN; };
	inline const std::vector<Article>& getArticles()const { return articles; };

	// ������
	bool setNewIssueNumber(unsigned int n);
	bool setNewISSN(const std::string& ISSN);
	bool setNewArticles(const std::vector<Article>& newArticles);


	// �������� �� �������� �� ���� ������
	void addNewArticle(const Article& article);
	virtual void serialize(std::ostream& os)const override;
	virtual void deserialize(std::istream& is)override;
	inline virtual Type getType()const override { return Type::PERIODICALS; };

	// �� list info � ���������
	inline virtual std::vector<std::string> getIdentifiers()const override {
		return { getISSN() };
	}

	virtual std::vector<std::string> getKeyWordsForFind()const override 
	{
		std::vector<std::string> res;
		for (size_t i = 0; i < articles.size(); i++)
		{
			for (size_t j = 0; j < articles[i].keyWords.size(); j++)
			{
				res.push_back(articles[i].keyWords[j]);
			}
		}
		return res; // returning const reference and exteniding its life
	};

	static Periodicals* createInteractively();

	virtual bool change()override;
	
protected:
	Periodicals();
	bool changePeriodicalsPart();
protected:
	void deserializePeriodicalsUnit(std::istream& is);
	void serializePeriodicalsUnit(std::ostream& os)const;
	void printPeriodicals(std::ostream& os = std::cout)const;
private:
	unsigned int issueNumber; // ���� - ����� �� �������
	std::string ISSN; // �� � ������������
	std::vector<Article> articles;
	static size_t counter;
};

std::ostream& operator<<(std::ostream& os, const Periodicals& periodical);
