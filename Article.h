#pragma once
#include <string>
#include <vector>
#include <iostream>

// ��������� �� ������
// plain - struct �� ��������� ����, �� �� �� �� ��������� �������
struct Article
{
	std::string author;
	std::string title;
	std::vector<std::string> keyWords;

	Article();
	Article(const std::string& author,
		const std::string& title,
		const std::vector<std::string>& v = {});
	// ���������� �� �������� �� �����������, ��� ���� �������� ������������� �� ������ ������

	// ������� �� �������� �� ���� ������� ����
	void addKeyWords(const std::string& key);

	// �� ���������� �� Articles 
	bool operator==(const Article& other)const;
	void print(std::ostream& out = std::cout)const;
	void serialize(std::ostream& os)const;
	void deserialize(std::istream& is);
};
