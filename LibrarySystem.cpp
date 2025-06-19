#include "LibrarySystem.h"
#include <fstream> // ������� � ������� �������
#include <algorithm> // ���� �� std::sort

const int MAX = 100;

LibrarySystem::LibrarySystem(const std::string& usersFile, const std::string& unitsFile )
    :fileUsers(usersFile),fileUnits(unitsFile)
{
    // ������� ��������� �� �����?
    if (fileUsers.empty()) {
        throw std::invalid_argument("Invalid name!");
    }
    else if (fileUnits.empty()) {
        throw std::invalid_argument("Invalid name!");
    }
    else if (fileUsers == fileUnits) {
        throw std::invalid_argument("Those names are equal!");
    }
    
    std::ifstream file1(usersFile, std::ios::binary);
    
    if (!file1.is_open()) {
        throw std::ios_base::failure("Error with opening fileUsers!");
    }

    std::ifstream file2(unitsFile, std::ios::binary);
    if (!file2.is_open()) {
            file1.close();
            file2.close();
            throw std::ios_base::failure("Error with opening fileUnits!");
        }   
    bool emptyCase = false;
    try
    {
        deserialize(file1,file2);
        if (infoUnits.empty() ){
            std::cerr << "There is no info about units loaded !\n";
            emptyCase = true;
        }
        else if(units.empty())
        {
            std::cerr << "No units loaded into the system!\n";
            emptyCase = true;
        }
        else if (infoUsers.empty()) {
            std::cerr << "There is no info loaded about users!\n";
            emptyCase = true;
        }
        else if (users.empty()) {
            std::cerr << "No users loaded into the system!\n";
            emptyCase = true;
        }

        
        if (emptyCase) {
            // ������������ �� ������������ ���� admin user
            Date today = Date(); // ������ ����
            Administrator* initialUser = new Administrator("admin", "i<3C++", today, today, "admin@email.bg");
            currentPerson = initialUser; // �����!
            users.push_back(initialUser);
        }
    }
    catch (...)
    {
        // ��� �� �� ��������, ��� ���� �� �����
        file1.close();
        file2.close();
        throw std::runtime_error("Somethign went wrong with deserializing in LibrarySystem!");
    }
    file1.close();
    file2.close();
}

LibrarySystem::LibrarySystem(LibrarySystem&& other) noexcept
    :units(std::move(other.units)),users(std::move(other.users))
    ,infoUnits(std::move(other.infoUnits)), infoUsers(std::move(other.infoUsers)),
    fileUsers(std::move(other.fileUsers)), fileUnits(std::move(other.fileUnits))
{
    currentPerson = other.currentPerson;
    other.currentPerson = nullptr;
}

LibrarySystem& LibrarySystem::operator=(LibrarySystem&& other) noexcept
{
    if (this != &other) {
        std::swap(units, other.units);
        std::swap(users, other.users);
        std::swap(fileUsers, other.fileUsers);
        std::swap(fileUnits, other.fileUnits);
        std::swap(infoUnits, other.infoUnits);
        std::swap(infoUsers, other.infoUsers);
        std::swap(currentPerson, other.currentPerson);
    }
    return *this;
}

LibrarySystem::~LibrarySystem()
{
	// ������ �� ���� �����, ������ ������ �� �� ����� ���

	for (size_t i = 0; i < units.size(); i++)
	{
		delete units[i];
	}
	for (size_t i = 0; i < users.size(); i++)
	{
		delete users[i];
	}
    users.clear();
    units.clear();
}

bool LibrarySystem::login(const std::string& username, const std::string& password)
{
    // Check if a user is already logged in
    if (currentPerson != nullptr) {
        std::cerr << "You are already logged in as " << currentPerson->getUsername() << ". Please logout first.\n";
        return false;
    }

    // First check if the user exists in metadata
    bool userFound = false;
    size_t filePos = 0;
    
    for (size_t i = 0; i < infoUsers.size(); i++) {
        if (infoUsers[i].username == username) {
            // Found the username, now check password
            if (infoUsers[i].password != password) {
                std::cerr << "Invalid username or password!\n";
                return false;
            }
            
            userFound = true;
            filePos = infoUsers[i].pos;
            break;
        }
    }

    // If user wasn't found in metadata
    if (!userFound) {
        //see if its already loaded?
        for (size_t i = 0; i < users.size(); i++)
        {
            if (users[i]->getUsername() == username && users[i]->getPassword() == password) {
                // ������ � ��
                currentPerson = users[i];
                std::cout << "Welcome, " << username << " !\n";
                return true;
            }
        }
    }

    try {
        // Open the file and load the user
        std::ifstream file(fileUsers, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Cannot open users file!\n";
            return false;
        }
        
        // Seek to the user's position in the file
        file.seekg(filePos, std::ios::beg);
        if (!file.good()) {
            file.close();
            std::cerr << "Error seeking to user position in file!\n";
            return false;
        }

        // Read the user type
        TypeOfReader type;
        file.read(reinterpret_cast<char*>(&type), sizeof(type));
        if (!file.good()) {
            file.close();
            std::cerr << "Error reading user type from file!\n";
            return false;
        }

        //// Skip username and password as we already have them
        //size_t size;
        //
        //// Skip username length and username
        //file.read(reinterpret_cast<char*>(&size), sizeof(size));
        //if (!file.good()) {
        //    file.close();
        //    std::cerr << "Error reading username size from file!\n";
        //    return false;
        //}
        //file.seekg(size, std::ios::cur); // Skip username content
        //
        //// Skip password length and password
        //file.read(reinterpret_cast<char*>(&size), sizeof(size));
        //if (!file.good()) {
        //    file.close();
        //    std::cerr << "Error reading password size from file!\n";
        //    return false;
        //}
        //file.seekg(size, std::ios::cur); // Skip password content

        //// Reset file position to beginning of the user record
        //file.seekg(filePos, std::ios::beg);
        
        // Create the user object (this will read all data including username/password)
        LibraryPerson* newUser = LibraryFactory::createPersonFromStream(file, type);
        if (!newUser) {
            file.close();
            std::cerr << "Error creating user object from file!\n";
            return false;
        }
        
        // Update the last login date
        newUser->lastLoginDate = Date(); // Current date
        
        // Close the file and add user to the system
        file.close();
        users.push_back(newUser);
        currentPerson = newUser;
        
        // Display welcome message
        std::cout << "Welcome, " << username << "!\n";
        
        return true;
    }
    catch (std::exception& e) {
        std::cerr << "Error during login: " << e.what() << std::endl;
        return false;
    }
}

bool LibrarySystem::logout()
{
    if (!currentPerson) {
        std::cerr << "There is no user currently logged in!\n";
        return false;
    }

    std::string username = currentPerson->getUsername();
    currentPerson = nullptr;
    std::cout << "User " << username << " has been successfully logged out.\n";
    return true;
}

bool LibrarySystem::exit()
{
    std::ofstream file1(fileUsers, std::ios::binary | std::ios::trunc);
    if (!file1.is_open()) {
        throw std::ios::basic_ios::failure("Error with opening fileUsers!");
    }
    std::ofstream file2(fileUnits, std::ios::binary | std::ios::trunc);
    if (!file2.is_open()) {
        file1.close();
        throw std::ios::basic_ios::failure("Error with opening fileUnits!");
    }
    try
    {
        serialize(file1, file2);
        IDGenerator::saveLastIdToFile("last_id.dat");
    }
    catch (...)
    {
        file1.close();
        file2.close();
        throw;
    }
}

// ���� ������� ����� � ���� �������, �� ���������, ��� �� �� �������, ����� ������ ������ ��� �����
void LibrarySystem::findUnits(const std::string& type, 
    const std::string& option, const std::string& value,
    const std::string& sortKey, const std::string& sortOrder,
    int topCount) const
{
    if (currentPerson == nullptr) {
        std::cerr << "We need atelast one active person!" << "\n";
        return;
    }
    std::vector<LibraryUnit*> filtered;
    std::ifstream file(fileUnits, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error with file in findUnits!\n";
        return;
    }

    try
    {
        for (size_t i = 0; i < infoUnits.size(); i++)
        {
            // ��� - �������� ����� ��������� ��� �� ����������
            if (type == "books" && infoUnits[i].type != Type::BOOK)
                continue;
            if (type == "series" && infoUnits[i].type != Type::SERIES)
                continue;
            if (type == "newsletters" && infoUnits[i].type != Type::PERIODICALS)
                continue;

            // �������� ��� �� � �� ���������
            file.seekg(infoUnits[i].pos, std::ios::beg);
            Type hasToBeRead;
            file.read(reinterpret_cast<char*>(&hasToBeRead), sizeof(hasToBeRead));
            if (!file) {
                throw std::ios_base::failure("Error with reading type!\n");
            }
            LibraryUnit* current = LibraryFactory::createUnitFromStream(file, infoUnits[i].type);
            if (!current) {
                std::cerr << "Couldnt found that unit or something happened!\n";
                file.close();
                return;
            }

            // �������� �� ��������� 
            if (option == "title")
            {
                if (current->getTitle().find(value) != std::string::npos)
                    filtered.push_back(current);
                else {
                    delete current;
                }
            }

            else if (option == "author")
            {
                // Book � Series  - �� ���� �����
                // ��� �� �� �������� � ����-�� �� �������� ��������
                if (Book* book = dynamic_cast<Book*>(current))
                {
                    if (book->getAuthor() == value) // string operator==
                        filtered.push_back(current);
                    else {
                        delete current;
                    }
                }

                // Periodicals � Series
                else if (Periodicals* periodical = dynamic_cast<Periodicals*>(current))
                {
                    bool found = false;
                    std::vector<Article> artc = periodical->getArticles();
                    for (size_t j = 0; j < artc.size(); j++)
                    {
                        if (artc[j].author == value) {
                            found = true;
                            filtered.push_back(current);
                        }
                    }
                    if (!found) {
                        delete current;
                    }
                }
            }

            else if (option == "tag")
            {
                // ������� ���� 
                std::vector<std::string> keyWord = current->getKeyWordsForFind();
                bool found = false;
                for (size_t i = 0; i < keyWord.size(); i++)
                {

                    if (keyWord[i] == value) {
                        filtered.push_back(units[i]);
                    }
                }
                if (!found) {
                    delete current;
                }
            }
        }

        // ���� ���� ����� ���������?
        if (!sortKey.empty()) {
            // ����� ������ �� ��������� 
            // ����������� �� ��������!
            std::sort(filtered.begin(), filtered.end(), [&](LibraryUnit* a, LibraryUnit* b) {
                if (sortKey == "title")
                return sortOrder != "desc" ? a->getTitle() <= b->getTitle() : a->getTitle() > b->getTitle();
            if (sortKey == "year")
                return sortOrder != "desc" ? a->getDate().getYear() <= b->getDate().getYear() : a->getDate().getYear() > b->getDate().getYear();
            return true;
                });
        }

        // ����������� top
        size_t limit = (topCount > 0 && !sortKey.empty()) ? std::min<size_t>(topCount, filtered.size()) : filtered.size();

        // ���������
        for (size_t i = 0; i < limit; ++i)
        {
            filtered[i]->display();
        }
        for (size_t i = 0; i < filtered.size(); i++)
        {
            delete filtered[i];
        }
        filtered.clear();
    }
    catch (const std::exception&)
    {
        file.close();
        throw;
    }
    file.close();
}

// ���������� � �������, �� �� �������� ��������
void LibrarySystem::findReaders(const std::string& option, const std::string& value)
{
    if (currentPerson == nullptr) {
        std::cerr << "We need at least one active person!" << "\n";
        return;
    }

    std::ifstream file(fileUsers, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Couldn't open file!" << "\n";
        return;
    }

    try {
        Date today; // ������� ���� - ����� � �������� ��� std::time

        if (option == "name") {
            for (size_t i = 0; i < infoUsers.size(); i++) {
                if (infoUsers[i].username == value) {
                    file.seekg(infoUsers[i].pos, std::ios::beg);

                    // ��������� ���� �����
                    TypeOfReader type;
                    file.read(reinterpret_cast<char*>(&type), sizeof(type));
                    if (!file.good()) {
                        std::cerr << "Error reading user type!" << "\n";
                        file.close();
                        return;
                    }

                    LibraryPerson* reader = LibraryFactory::createPersonFromStream(file, type);
                    if (!reader) {
                        std::cerr << "Error creating reader!" << "\n";
                        file.close();
                        return;
                    }

                    reader->display();
                    delete reader;
                    file.close();
                    return;
                }
            }
            std::cerr << "No user with name: " << value << "\n";
        }

        else if (option == "ID") {
            int targetId = std::stoi(value);
            bool found = false;

            for (size_t i = 0; i < infoUsers.size(); i++) {
                file.seekg(infoUsers[i].pos, std::ios::beg);

                // ��������� ����
                TypeOfReader type;
                file.read(reinterpret_cast<char*>(&type), sizeof(type));
                if (!file.good()) {
                    std::cerr << "Error reading user type!" << "\n";
                    continue;
                }

                LibraryPerson* person = LibraryFactory::createPersonFromStream(file, type);
                if (!person) {
                    std::cerr << "Error creating person!" << "\n";
                    continue;
                }

                // ����������� ���� � Reader � ��� ������� �����
                Reader* reader = dynamic_cast<Reader*>(person);
                if (reader && reader->hasBorrowedUnit(targetId)) {
                    reader->display();
                    found = true;
                }

                delete person;  // ������ ��������� ��������� �����
            }

            if (!found) {
                std::cerr << "No users took unit with id: " << value << "\n";
            }
        }

        else if (option == "state") {
            std::vector<LibraryPerson*> tempReaders;

            // ����� ��������� ������ �������� � �������� ������
            for (size_t i = 0; i < infoUsers.size(); i++) {
                file.seekg(infoUsers[i].pos, std::ios::beg);

                TypeOfReader type;
                file.read(reinterpret_cast<char*>(&type), sizeof(type));
                if (!file.good()) continue;

                LibraryPerson* person = LibraryFactory::createPersonFromStream(file, type);
                if (!person) continue;

                Reader* reader = dynamic_cast<Reader*>(person);
                if (reader) {
                    tempReaders.push_back(person);
                }
                else {
                    delete person;  // �� � Reader, ��������� ��
                }
            }

            // ���� ���������� ������ �����������
            bool found = false;
            for (size_t i = 0; i < tempReaders.size(); i++) {
                Reader* reader = dynamic_cast<Reader*>(tempReaders[i]);
                bool shouldDisplay = false;

                if (value == "overdue") {
                    shouldDisplay = reader->hasOverdueBooks(today);
                }
                else if (value == "active") {
                    shouldDisplay = reader->borrowedLastMonth(today) > 5;
                }
                else if (value == "inactive") {
                    shouldDisplay = reader->monthsSinceLastBorrow(today) > 3;
                }

                if (shouldDisplay) {
                    reader->display();
                    found = true;
                }
            }

            // ��������� ������ �������� ������
            for (size_t i = 0; i < tempReaders.size(); i++) {
                delete tempReaders[i];
            }
            tempReaders.clear();

            if (!found) {
                std::cerr << "No users found with state: " << value << "\n";
            }

            if (value != "overdue" && value != "active" && value != "inactive") {
                std::cerr << "Invalid option for state!" << "\n";
            }
        }

        else {
            std::cerr << "Invalid option" << "\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in findReaders: " << e.what() << "\n";
        file.close();
        throw;
    }

    file.close();
}

bool LibrarySystem::serialize(std::ostream& file1, std::ostream& file2) const
{
    if (!file1 || !file2) {
        throw std::invalid_argument("Invalid stream arguments for serialization!");
    }
    
    try {
        // Write units count
        size_t size = units.size();
        file2.write(reinterpret_cast<const char*>(&size), sizeof(size));
        if (!file2.good()) {
            throw std::ios_base::failure("Error writing units count");
        }
        
        // Write units
        for (size_t i = 0; i < size; i++) {
            // Record position before writing this unit
            size_t pos = file2.tellp();
            
            /*
            // Write unit ID and availability status for the metadata
            unsigned int id = units[i]->getUniqueNumber();
            file2.write(reinterpret_cast<const char*>(&id), sizeof(id));
            if (!file2.good()) {
                throw std::ios_base::failure("Error writing unit ID");
            }
            
            bool isFree = units[i]->isAvailable();
            file2.write(reinterpret_cast<const char*>(&isFree), sizeof(isFree));
            if (!file2.good()) {
                throw std::ios_base::failure("Error writing unit availability status");
            }
            */

            // Serialize the entire unit data using the public interface
            units[i]->serialize(file2);
            if (!file2.good()) {
                throw std::ios_base::failure("Error serializing unit data");
            }
            
            
        }
        
        // Write users count
        size_t sizeOfUsers = users.size();
        file1.write(reinterpret_cast<const char*>(&sizeOfUsers), sizeof(sizeOfUsers));
        if (!file1.good()) {
            throw std::ios_base::failure("Error writing users count");
        }
        
        // Write users
        for (size_t i = 0; i < sizeOfUsers; i++) {
            // Record position before writing this user
            size_t pos = file1.tellp();
            
            // Serialize the rest of the user data
            users[i]->serialize(file1);
            if (!file1.good()) {
                throw std::ios_base::failure("Error serializing user data");
            }
            
            // Note: We no longer modify infoUsers here since this is a const method
        }
        
        return true;
    }
    catch (std::exception& e) {
        std::cerr << "Error during serialization: " << e.what() << std::endl;
        throw; // Re-throw the exception after logging it
    }
}

bool LibrarySystem::deserialize(std::istream& file1, std::istream& file2)
{
    if (!file1 || !file2) {
        throw std::invalid_argument("Invalid stream arguments for deserialization!");
    }
    
    // Check for empty files
    if ((file2.tellg() == 0 && file2.peek() == std::ifstream::traits_type::eof()) ||
        (file1.tellg() == 0 && file1.peek() == std::ifstream::traits_type::eof())) {
        return false;
    }
    
    try {
        // Clear existing metadata before deserializing
        infoUnits.clear();
        infoUsers.clear();
        
        // Read units count
        size_t unitsCount;
        file2.read(reinterpret_cast<char*>(&unitsCount), sizeof(unitsCount));
        if (!file2.good()) {
            throw std::ios_base::failure("Error reading units count from file");
        }
        
        // Read units metadata
        for (size_t i = 0; i < unitsCount; ++i) {
            size_t pos = file2.tellg();  // Record current position
            
            // Read the type
            Type type;
            file2.read(reinterpret_cast<char*>(&type), sizeof(type));
            if (!file2.good()) {
                throw std::ios_base::failure("Error reading unit type");
            }
           
            // Skip the data by creating and then deleting a temporary object
            LibraryUnit* temp = LibraryFactory::createUnitFromStream(file2, type);
            if (!temp) {
                throw std::runtime_error("Failed to create unit object from stream");
            }
            // Add metadata to our collection
            try
            {
                infoUnits.push_back(UniqueIDAndFilePositions{ temp->getId(), pos, temp->isAvailable(),temp->getType() });
                delete temp;

            }
            catch (...)
            {
                delete temp;
                throw;
            }
            
        }

        // Read users count - this was missing in the original code!
        size_t userCount;
        file1.read(reinterpret_cast<char*>(&userCount), sizeof(userCount));
        if (!file1.good()) {
            throw std::ios_base::failure("Error reading user count from file");
        }
        
        // Now read exactly 'userCount' users
        for (size_t i = 0; i < userCount; ++i) {
            size_t pos = file1.tellg();
            
            TypeOfReader type;
            file1.read(reinterpret_cast<char*>(&type), sizeof(type));
            if (!file1.good()) {
                throw std::ios_base::failure("Error reading user type");
            }
                        
            // Create the person object from the stream
            LibraryPerson* temp = LibraryFactory::createPersonFromStream(file1, type);
            if (!temp) {
                throw std::runtime_error("Failed to create person object from stream");
            }

            // Add the user's info to our metadata collection
            try
            {
                infoUsers.push_back(MetaInfoAboutUsers{ temp->getUsername(), temp->getPassword(), pos,temp->getType() });
                delete temp;
            }
            catch (...)
            {
                
                delete temp;
                throw;
            }
            
        }
        
        return true;
    }
    catch (std::exception& e) {
        std::cerr << "Error during deserialization: " << e.what() << std::endl;
        throw; // Re-throw the exception after logging it
    }
}

void LibrarySystem::addUnit(const Type& type)
{
    if (!currentPerson || currentPerson->getType() != TypeOfReader::ADMINISTRATOR) {
        std::cerr << "Only admins can do that operation!" << "\n";
        return;
    }

    LibraryUnit* newItem = LibraryFactory::createUnitInteractively(type);
    if (newItem == nullptr) {
        std::cerr << "Adding new unit been cancelled!" << "\n";
        return;
    }

    units.push_back(newItem);
    
    std::cout << "Successfully pushed new Unit in our system!" << "\n";
    return;
}

void LibrarySystem::removeUnit(unsigned int id)
{
    if (!currentPerson || currentPerson->getType() != TypeOfReader::ADMINISTRATOR) {
        std::cerr << "Only admins can do that operation!" << "\n";
        return;
    }

    for (size_t i = 0; i < units.size(); i++)
    {
        if (units[i]->getId() == id) {
            std::swap(units[i], units.back());
            delete units.back();
            units.pop_back();
            // size �� �������� � ��� � ������ ������ �� nullptr
        }
    }

    // ������ �� �� �������� �� ����������
    for (size_t i = 0; i < users.size(); i++)
    {
        Reader* isReader = dynamic_cast<Reader*>(users[i]);
        if (isReader == nullptr) {
            continue;
        }
        // �����
        std::vector<LibraryUnit*> unit = isReader->getUnits();
        for (size_t j = 0; j < unit.size(); j++)
        {
            if (unit[i]->getId() == id) {
                unit[i] = nullptr;
            }
        }
    }
}

bool LibrarySystem::changeUnit(unsigned int id)
{
    if (!currentPerson || currentPerson->getType() != TypeOfReader::ADMINISTRATOR) {
        std::cerr << "Only admins can do that operation!" << "\n";
        return false;
    }

    for (size_t i = 0; i < units.size(); i++)
    {
        if (units[i]->getId() == id) {
            
            try
            {
                if (!units[i]->change()) {
                    return false;
                }
                return true;
            }
            catch (...)
            {
                std::cerr << "Cannot change current unit!";
                return false;
            }
        }
    }
    std::cerr << "Not found!\n";
    return false;
}

void LibrarySystem::addUser(bool isAdmin)
{
    if (!currentPerson || currentPerson->getType() != TypeOfReader::ADMINISTRATOR) {
        std::cerr << "Only admins can do that operation!" << "\n";
        return;
    }
    
    TypeOfReader type = READER;
    if(isAdmin){
        type = ADMINISTRATOR;
    }

    LibraryPerson* newItem = LibraryFactory::createPersonInteractively(type);
    if (newItem == nullptr) {
        std::cerr << "Adding new user been cancelled!" << "\n";
        return;
    }
    
    // Add the user to the users vector
    users.push_back(newItem);
    

    /*
    // To add the user to the file, we need to rewrite the entire file
    // because we need to update the users count at the beginning of the file
    std::ofstream tempFile(fileUsers + ".temp", std::ios::binary);
    if (!tempFile.is_open()) {
        std::cerr << "Cannot open temporary file to save user data!" << "\n";
        return;
    }
    
    // Write the updated user count
    size_t sizeOfUsers = users.size();
    tempFile.write(reinterpret_cast<const char*>(&sizeOfUsers), sizeof(sizeOfUsers));
    if (!tempFile.good()) {
        tempFile.close();
        std::cerr << "Error writing user count to file!" << "\n";
        return;
    }
    
    // Write all users including the new one
    for (size_t i = 0; i < users.size(); i++) {
        // Record position for metadata
        size_t pos = tempFile.tellp();

        // Serialize the user
        users[i]->serialize(tempFile);
        if (!tempFile.good()) {
            tempFile.close();
            std::cerr << "Error serializing user data to file!" << "\n";
            return;
        }
        
        // Update metadata for this user
        // For existing users, update position
        bool found = false;
        for (size_t j = 0; j < infoUsers.size(); j++) {
            if (infoUsers[j].username == users[i]->getUsername()) {
                infoUsers[j].pos = pos;
                found = true;
                break;
            }
        }
        
        // For the new user, add metadata
        if (!found && users[i] == newItem) {
            infoUsers.push_back(MetaInfoAboutUsers{newItem->getUsername(), newItem->getPassword(), pos});
        }
    }
    
    // Close the temporary file before attempting to rename it
    tempFile.close();
    
    // In Windows, std::rename will fail if the destination file exists
    // First try to remove the existing file if it exists
    if (std::remove(fileUsers.c_str()) != 0) {
        // If remove failed but the file doesn't exist, that's fine
        // Otherwise, only show an error if it's not a "file not found" error
        if (errno != ENOENT) {
            std::cerr << "Warning: Could not remove existing file. " 
                      << "Error code: " << errno << "\n";
        }
    }
    
    // Now try to rename the temporary file
    if (std::rename((fileUsers + ".temp").c_str(), fileUsers.c_str()) != 0) {
        std::cerr << "Failed to update the users file! Error code: " << errno << "\n";
        // In case of failure, try a different approach - copy the content manually
        std::ifstream src(fileUsers + ".temp", std::ios::binary);
        std::ofstream dst(fileUsers, std::ios::binary | std::ios::trunc);
        
        if (src && dst) {
            dst << src.rdbuf();
            if (dst.good()) {
                std::cout << "Successfully added new user " << newItem->getUsername() 
                          << " to the system using file copy!\n";
                // Remove the temporary file
                src.close();
                dst.close();
                std::remove((fileUsers + ".temp").c_str());
                return;
            }
        }
        
        std::cerr << "Critical error: could not update user data.\n";
        return;
    }
    */
    std::cout << "Successfully added new user " << newItem->getUsername() << " to the system!\n";
}

void LibrarySystem::removeUser(const std::string& name)
{
    if (!currentPerson || currentPerson->getType() != TypeOfReader::ADMINISTRATOR) {
        std::cerr << "Only admins can do that operation!" << "\n";
        return;
    }

    size_t count = 0;
    for (size_t i = 0; i < users.size(); i++)
    {
        if (users[i]->getType() == ADMINISTRATOR) {
            count++;
        }
    }

    for (size_t i = 0; i < users.size(); i++)
    {
        if (users[i]->getUsername() == name) {
            //  ������ �� �� ������
            if (users[i]->getType() == TypeOfReader::ADMINISTRATOR && count == 1) {
                std::cerr << "Cannot remove cause  this is the last Admin in our system!\n";
                return;
            }
            // ����� ����� �� �� ������
            if (currentPerson->getUsername() == users[i]->getUsername() &&
                currentPerson->getPassword() == users[i]->getPassword())
            {
                if (!this->logout()) {
                    std::cerr << "Something happened!\n";
                    return;
                };

                // ����� � ���� �������
                std::cout << "Successfully removed user: " << name << "\n";
                return;
            }
            // ����� �� � ������ ���������
            std::swap(users[i], users.back());
            delete users.back();
            users.pop_back();
            return;
        }
    }
    std::cerr << "Not found!\n";
    return;
}

bool LibrarySystem::changeUser(const std::string& currentUser, const std::string& newPassword, const std::string& targetUser) {

    LibraryPerson* executor = findUserByUsername(currentUser);
    if (!executor) return false;

    // ��� ��������: �������� ���������� ����� ������ ������
    if (targetUser.empty()) {
        return executor->setPassword(newPassword);
    }

    // � ��������: ����� ����� �������� �� ���� ����������
    if (executor->getType() != TypeOfReader::ADMINISTRATOR) return false;  // ���� ����� ����

    LibraryPerson* target = findUserByUsername(targetUser);
    if (!target) return false;

    return target->setPassword(newPassword);
}

bool LibrarySystem::userFind(const std::string& option, const std::string& optionString) const
{
    
    if (!currentPerson || currentPerson->getType() != ADMINISTRATOR) {
        std::cerr << "No active user or active but not administrator!\n";
        return false;
    }

    if (option == "name") {
        LibraryPerson* user = findUserByUsername(optionString);
        if (user == nullptr) {
            std::cerr << "Not found!";
            return false;
        }
        // ����� � �������
        user->display();
        return true;
    }
    Date today = Date();
    std::vector<LibraryPerson*> filtered;

    if (option == "id") {
        unsigned int id = std::stoul(optionString);
        for (size_t i = 0; i < users.size(); i++)
        {
            
            // ����� � �� ������� ������� �������, �� �� �������� �� �� �������� ���� ����
            Reader* hasToBeReader = dynamic_cast<Reader*>(users[i]);
            if (hasToBeReader == nullptr) {
                continue; // ���� ���������� ������ �����
            }

            std::vector<LibraryUnit*> takenFromReader = hasToBeReader->getUnits();
            for (size_t j = 0; j < takenFromReader.size(); j++)
            {
                if (takenFromReader[j]->getId() == id) {
                    filtered.push_back(users[i]);
                }
            }
        }

        for (size_t i = 0; i < filtered.size(); i++)
        {
            filtered[i]->display();
        }
    }
    
    else if (option == "state" && (optionString == "overdue" || optionString == "reader" || optionString == "inactive")) 
    {
        if (optionString == "overdue") {
            for (size_t i = 0; i < users.size(); i++)
            {
                Reader* hasToBeReader = dynamic_cast<Reader*>(users[i]);
                if (hasToBeReader == nullptr) {
                    continue;
                }
                // ������� ����� �� � ������ � ����
                if (hasToBeReader->hasOverdueBooks(today)) {
                    filtered.push_back(users[i]);
                }
            }
        }
        else if (optionString == "reader") {
            for (size_t i = 0; i < users.size(); i++)
            {
                Reader* hasToBeReader = dynamic_cast<Reader*>(users[i]);
                if (hasToBeReader == nullptr) {
                    continue;
                }
                 // ���� � ���� ������ �� 5 �����
                unsigned int count = hasToBeReader->borrowedLastMonth(today);
                if (count > 5) {
                    filtered.push_back(users[i]);
                }
            }
        }
        else if (optionString == "inactive")
        {
            for (size_t i = 0; i < users.size(); i++)
            {
                Reader* hasToBeReader = dynamic_cast<Reader*>(users[i]);
                if (hasToBeReader == nullptr) {
                    continue;
                }
                // �� �� � ������ ����� �� ������ �� 3 ������
                if (hasToBeReader->monthsSinceLastBorrow(today) > 3) {
                    filtered.push_back(users[i]);
                }
            }
        }

        for (size_t i = 0; i < filtered.size(); i++)
        {
            filtered[i]->display();
            
        }
        return true;
    }

    else {
        throw std::invalid_argument("Invalid option for usersFind!\n");
    }

}

bool LibrarySystem::take(const std::string& user, unsigned int id)
{
    if (currentPerson == nullptr || currentPerson->getUsername() != user) {
        std::cerr << "We need a current user logged in our system!\n";
        return false;
    }

    LibraryUnit* unit = findUnitById(id);

    if (currentPerson->getType() == ADMINISTRATOR) {
        std::cerr << "Administrators cannot take units!\n";
        return false;
    }

    if (!unit)
        return false;

    if (!unit->isAvailable())
        return false;

    if (unit->incrementTakenCopies()) {
        currentPerson->borrow(unit);
        return true;
    }

    return false;
}

bool LibrarySystem::returnUnit(unsigned int id)
{
    if (currentPerson == nullptr || currentPerson->getType() == ADMINISTRATOR) {
        std::cerr << "We need a user logged in our system to perform this return operation! \nAdmins cannot perform it!\n";
        return false;
    }
    
    if (!currentPerson->returnUnit(id)) {
        return false;
    };

    return true;
}

void LibrarySystem::print()
{
    for (size_t i = 0; i < units.size(); i++)
    {
        units[i]->display();
    }for (size_t i = 0; i < users.size(); i++)
    {
        users[i]->display();

    }
}

void LibrarySystem::booksAll(std::ostream& out) 
{
    if (currentPerson == nullptr) {
        std::cerr << "We need atelast one active person!" << "\n";
        return;
    }

    std::ifstream file(fileUnits,std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error with file while booksAll!\n";
        return;
    }

    //������ � meta �������, ��� � ��� -> ����������, �� �� � ��������� ���

    for (size_t i = 0; i < infoUnits.size(); i++)
    {
        if (infoUnits[i].type == Type::BOOK) {
            try
            {
                file.seekg(infoUnits[i].pos, std::ios::beg);
                
                Type hasToBeRead;
                file.read(reinterpret_cast<char*>(&hasToBeRead), sizeof(hasToBeRead));
                if (!file) {
                    file.close();
                    throw std::runtime_error("Someting happened with file!");
                }

                LibraryUnit* unit = LibraryFactory::createUnitFromStream(file, hasToBeRead);
                if (!unit) {
                    std::cerr << "Creating unit error!\n";
                    file.close();
                }
                
                unit->printBase();
            }
            catch (...)
            {
                file.close();
                throw;
            } 
        }
        
    }
}

void LibrarySystem::periodicalsAll(std::ostream& out) 
{
    if (currentPerson == nullptr) {
        std::cerr << "We need atelast one active person!" << "\n";
        return;
    }
    
    std::ifstream file(fileUnits, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error with file while booksAll!\n";
        return;
    }

    //������ � meta �������, ��� � ��� -> ��������� �������
    for (size_t i = 0; i < infoUnits.size(); i++)
    {
        if (infoUnits[i].type == Type::PERIODICALS) {
            try
            {
                file.seekg(infoUnits[i].pos, std::ios::beg);

                Type hasToBeRead;
                file.read(reinterpret_cast<char*>(&hasToBeRead), sizeof(hasToBeRead));
                if (!file) {
                    file.close();
                    throw std::runtime_error("Someting happened with file!");
                }

                LibraryUnit* unit = LibraryFactory::createUnitFromStream(file, hasToBeRead);
                if (!unit) {
                    std::cerr << "Creating unit error!\n";
                    file.close();
                }
             
                unit->printBase();
            }
            catch (...)
            {
                file.close();
                throw;
            }
        }
    }
}

void LibrarySystem::seriesAll(std::ostream& out) 
{
    if (currentPerson == nullptr) {
        std::cerr << "We need atelast one active person!" << "\n";
        return;
    }

    std::ifstream file(fileUnits, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error with file while booksAll!\n";
        return;
    }

    //������ � meta �������, ��� � ��� -> ��������� �������
    for (size_t i = 0; i < infoUnits.size(); i++)
    {
        if (infoUnits[i].type == Type::SERIES) {
            try
            {
                file.seekg(infoUnits[i].pos, std::ios::beg);

                Type hasToBeRead;
                file.read(reinterpret_cast<char*>(&hasToBeRead), sizeof(hasToBeRead));
                if (!file) {
                    file.close();
                    throw std::runtime_error("Someting happened with file!");
                }

                LibraryUnit* unit = LibraryFactory::createUnitFromStream(file, hasToBeRead);
                if (!unit) {
                    std::cerr << "Creating unit error!\n";
                    file.close();
                }

                unit->printBase();

            }
            catch (...)
            {
                file.close();
                throw;
            }
        }
    }
}

void LibrarySystem::all(std::ostream& out) 
{
    if (currentPerson == nullptr) {
        std::cerr << "We need atelast one active person!" << "\n";
        return;
    }


    std::ifstream file(fileUnits, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error with file while booksAll!\n";
        return;
    }

    
    //������ � meta �������, ��� � ��� -> ��������� �������
    for (size_t i = 0; i < infoUnits.size(); i++)
    {
            try
            {
                file.seekg(infoUnits[i].pos, std::ios::beg);

                Type hasToBeRead;
                file.read(reinterpret_cast<char*>(&hasToBeRead), sizeof(hasToBeRead));
                if (!file) {
                    file.close();
                    throw std::runtime_error("Someting happened with file!");
                }

                LibraryUnit* unit = LibraryFactory::createUnitFromStream(file, hasToBeRead);
                if (!unit) {
                    std::cerr << "Creating unit error!\n";
                    file.close();
                }

                unit->printBase();
            }
            catch (...)
            {
                file.close();
                throw;
            }
    }
}

void LibrarySystem::listInfo(const std::string& ISNN_OR_ISBN) 
{
    if (currentPerson == nullptr) {
        std::cerr << "We need atelast one active person!" << "\n";
        return;
    }
    
    // ������������ ������ �� � ��������, ����� ���� �� �� ������
    // ����� �� �������, �� � ����, �� ���� �� �� �������

    // ������������ ���������
    for (size_t i = 0; i < units.size(); i++)
    {
        std::vector<std::string> ident = units[i]->getIdentifiers(); // �� ������ ����� ������, �� �� ��� -����� � ��� �������� 
        for (size_t j = 0; j  < ident.size(); j ++)
        {
            if (ident[i] == ISNN_OR_ISBN) {
                units[i]->display(); // ��-�������� ����������
                return;
            }
        }
    }
    std::cerr << "Item was not found!" << "\n";
    return;
}

bool LibrarySystem::canAddNewUser(const std::string& name, const std::string& password) const
{
    // ���� ����� �� ������� ��� ����������
    // ��� � � ��������� - ��
    // ��� ��� �������������� � �������� - ��� ��

    // ��� � ������� ����
    for (size_t i = 0; i < users.size(); i++)
    {
        // operator== of string
        if (users[i]->getUsername() == name) {
            return false;
        }
    }

    // ��� �� ��� �� � �������
    for (size_t i = 0; i < infoUsers.size(); i++)
    {
        if(infoUsers[i].username == name && infoUsers[i].password == password) 
        {
            return true;
        }
    }
    return false;
}

LibraryPerson* LibrarySystem::findUserByUsername(const std::string& n) const
{
    for (size_t i = 0; i < users.size(); i++)
    {
        if (users[i]->getUsername() == n) {
            return users[i];
        }
    }
    return nullptr;
}

LibraryUnit* LibrarySystem::findUnitById(unsigned int id) const
{
    for (size_t i = 0; i < units.size(); i++)
    {
        if (units[i]->getId() == id) { return units[i]; };
    }
    return nullptr;
}
