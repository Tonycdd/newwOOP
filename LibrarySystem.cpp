#include "LibrarySystem.h"
#include <fstream> // работим с двоични файлове
#include <algorithm> // само за std::sort

const int MAX = 100;

LibrarySystem::LibrarySystem(const std::string& usersFile, const std::string& unitsFile )
    :fileUsers(usersFile),fileUnits(unitsFile)
{
    // някакви валидации за името?
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
    bool emptyCase = true;
    try
    {
        deserialize(file1,file2);
        if (emptyCase) {
            std::cout << "Creating default administrator!\n";
                // регистрираме по подразбиране един admin user
                Date today = Date(); // днешна дата
                Administrator* admin = new Administrator("admin", "i<3C++", today, today, "admin@email.bg");
                currentPerson = admin; // важно!
                std::cout << "Logged as: 'admin!'\n";
                try
                {
                    users.push_back(admin);  

                }
                catch (...)
                {
                    delete admin;
                    throw;
                }  
        }
    }

    catch (...)
    {
        // тук ще ги затворим, ако нещо се счупи
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
	// правим си наши копия, затова трябва да ги трием тук

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
                // тогава е ок
                currentPerson = users[i];
                std::cout << "Welcome, " << username << " !\n";
                return true;
            }
        }
    }
    else
    {
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
            std::streampos posBeforeType = file.tellg(); // запазваме позицията
            file.read(reinterpret_cast<char*>(&type), sizeof(type));
            if (!file.good()) {
                throw std::ios_base::failure("Error with file!");
            }

            // курсора обратно, за да може обектът сам да си прочете Type вътре
            file.seekg(posBeforeType);
            if (!file) {
                throw std::ios_base::failure("Failed to seek back before reading unit!");
            }
            
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
    std::fstream file1(fileUsers, std::ios::binary| std::ios::in| std::ios::out);
    if (!file1.is_open()) {
        throw std::ios::basic_ios::failure("Error with opening fileUsers!");
    }

    std::fstream file2(fileUnits, std::ios::binary | std::ios::in | std::ios::out);
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

// тази фукнция търси в мета данните, не зареждаве, ако не ни трябват, бавно защото търсим във файла
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
            // тип - намираме първо правилния тип от колекцията
            if (type == "books" && infoUnits[i].type != Type::BOOK)
                continue;
            if (type == "series" && infoUnits[i].type != Type::SERIES)
                continue;
            if (type == "newsletters" && infoUnits[i].type != Type::PERIODICALS)
                continue;

            // намерили сме го и го зареждаме
            file.seekg(infoUnits[i].pos, std::ios::beg);
           
            // Read the user type
            Type type;
            std::streampos posBeforeType = file.tellg(); // запазваме позицията
            file.read(reinterpret_cast<char*>(&type), sizeof(type));
            if (!file.good()) {
                throw std::ios_base::failure("Error with file!");
            }

            // курсора обратно, за да може обектът сам да си прочете Type вътре
            file.seekg(posBeforeType);
            if (!file) {
                throw std::ios_base::failure("Failed to seek back before reading unit!");
            }

            LibraryUnit* current = LibraryFactory::createUnitFromStream(file, infoUnits[i].type);
            if (!current) {
                std::cerr << "Couldnt found that unit or something happened!\n";
                file.close();
                return;
            }

            // критерий за търсенето 
            if (option == "title")
            {
                if (current->getTitle() == value)
                    filtered.push_back(current);
                else {
                    delete current;
                }
            }

            else if (option == "author")
            {
                // Book и Series  - те имат автор
                // тук ще го направим с каст-за да избегнем проверки
                if (Book* book = dynamic_cast<Book*>(current))
                {
                    if (book->getAuthor() == value) // string operator==
                        filtered.push_back(current);
                    else {
                        delete current;
                    }
                }

                // Periodicals и Series
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
                // ключови думи 
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

        // сега дали имаме сортиране?
        if (!sortKey.empty()) {
            // значи трябва да сортираме 
            // взаимствано от интернет!
            std::sort(filtered.begin(), filtered.end(), [&](LibraryUnit* a, LibraryUnit* b) {
                if (sortKey == "title")
                return sortOrder != "desc" ? a->getTitle() <= b->getTitle() : a->getTitle() > b->getTitle();
            if (sortKey == "year")
                return sortOrder != "desc" ? a->getDate().getYear() <= b->getDate().getYear() : a->getDate().getYear() > b->getDate().getYear();
            return true;
                });
        }

        // Ограничение top
        size_t limit = (topCount > 0 && !sortKey.empty()) ? std::min<size_t>(topCount, filtered.size()) : filtered.size();

        // Извеждане
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

// аналогично с юзърите, не ги добавяме всичките
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
        Date today; // създава дата - която е днешната със std::time

        if (option == "name") {
            for (size_t i = 0; i < infoUsers.size(); i++) {
                if (infoUsers[i].username == value) {
                    file.seekg(infoUsers[i].pos, std::ios::beg);

                    // Прочитаме типа първо
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

                // Прочитаме типа
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

                // Проверяваме дали е Reader и има взетата книга
                Reader* reader = dynamic_cast<Reader*>(person);
                if (reader && reader->hasBorrowedUnit(targetId)) {
                    reader->display();
                    found = true;
                }

                delete person;  // Винаги изтриваме временния обект
            }

            if (!found) {
                std::cerr << "No users took unit with id: " << value << "\n";
            }
        }

        else if (option == "state") {
            std::vector<LibraryPerson*> tempReaders;

            // Първо зареждаме всички читатели в временен вектор
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
                    delete person;  // Не е Reader, изтриваме го
                }
            }

            // Сега филтрираме според състоянието
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

            // Изтриваме всички временни обекти
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
        file2.seekp(0, std::ios::beg);
        // Write units count
        size_t size = units.size() + infoUnits.size();
        file2.write(reinterpret_cast<const char*>(&size), sizeof(size));
        if (!file2.good()) {
            throw std::ios_base::failure("Error writing units count");
        }
        file2.seekp(0, std::ios::end);
        // Write units
        for (size_t i = 0; i < units.size(); i++) {
            units[i]->serialize(file2);
            if (!file2.good()) {
                throw std::ios_base::failure("Error serializing unit data");
            }
        }
        
        // Write users count
        file1.seekp(0, std::ios::beg);

        size_t sizeOfUsers = users.size() + infoUsers.size();

        for (size_t i = 0; i < users.size(); i++)
        {
            for (size_t j = 0; j < infoUsers.size(); j++)
            {
                if (users[i]->getUsername() == infoUsers[j].username &&
                    users[i]->getPassword() == infoUsers[j].password) {
                    sizeOfUsers--;
                }
            }
        }
        
        file1.write(reinterpret_cast<const char*>(&sizeOfUsers), sizeof(sizeOfUsers));
        if (!file1.good()) {
            throw std::ios_base::failure("Error writing users count");
        }
        
        // Write users

        file1.seekp(0, std::ios::end);

        for (size_t i = 0; i < users.size(); i++) {
            // Record position before writing this user
            size_t pos = file1.tellp();
            
            // Serialize the rest of the user data
            
            bool isAlreadeyIn = false;
            
            for (size_t j = 0; j < infoUsers.size(); j++)
            {
                if (users[i]->getUsername() == infoUsers[j].username &&
                    users[i]->getPassword() == infoUsers[j].password) 
                {
                    isAlreadeyIn = true;
                }
            }

            if (!isAlreadeyIn) {
                users[i]->serialize(file1);
                if (!file1.good()) {
                    throw std::ios_base::failure("Error serializing user data");
                }
            }

  
        }
        
        return true;
    }
    catch (std::exception& e) {
        std::cerr << "Error during serialization: " << e.what() << std::endl;
        throw;
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
        unsigned maxId = 0;
        file2.read(reinterpret_cast<char*>(&unitsCount), sizeof(unitsCount));
        if (!file2.good()) {
            throw std::ios_base::failure("Error reading units count from file");
        }
        
        // Read units metadata
        for (size_t i = 0; i < unitsCount; ++i) {
            size_t pos = file2.tellg();
            Type type;
            std::streampos posBeforeType = file2.tellg(); // запазваме позицията
            file2.read(reinterpret_cast<char*>(&type), sizeof(type));
            if (!file2.good()) {
                throw std::ios_base::failure("Error with file!");
            }

            // курсора обратно, за да може обектът сам да си прочете Type вътре
            file2.seekg(posBeforeType);
            if (!file2) {
                throw std::ios_base::failure("Failed to seek back before reading unit!");
            }

            // Skip the data by creating and then deleting a temporary object
            LibraryUnit* temp = LibraryFactory::createUnitFromStream(file2, type);
            if (temp->getId() > maxId) {

                maxId = temp->getId();
            }

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
        IDGenerator::setLastId(maxId);

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
            std::streampos posBeforeType = file1.tellg(); // запазваме позицията
            file1.read(reinterpret_cast<char*>(&type), sizeof(type));
            if (!file1.good()) {
                throw std::ios_base::failure("Error with file!");
            }

            // курсора обратно, за да може обектът сам да си прочете Type вътре
            file1.seekg(posBeforeType);
            if (!file1) {
                throw std::ios_base::failure("Failed to seek back before reading unit!");
            }
                        
            // Create the person object from the stream
            LibraryPerson* temp = LibraryFactory::createPersonFromStream(file1, type);
            if (!temp) {
                throw std::runtime_error("Failed to create person object from stream");
            }
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
        throw; 
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
    bool found = false;
    for (size_t i = 0; i < units.size(); i++)
    {
        if (units[i]->getId() == id) {
            found = true;
            std::swap(units[i], units.back());
            delete units.back();
            units.pop_back();
            // size се намалява и той 
            std::cout << "Succesfully remove unit with id: " << id << "\n";
        }
    }
    if (!found) {
        std::cout << "No unit with that id: " << id << " has been loaded into the system!\n";
    }
    // махаме ги от записите на читателите
    for (size_t i = 0; i < users.size(); i++)
    {
        Reader* isReader = dynamic_cast<Reader*>(users[i]);
        if (isReader == nullptr) {
            continue;
        }
        // иначе
        std::vector<LibraryUnit*> unit = isReader->getUnits();
        for (size_t j = 0; j < unit.size(); j++)
        {
            if (unit[i]->getId() == id) {
                unit[i] = nullptr;
            }
        }
    }
}

// тук ще ги зареждаме в системата вече
bool LibrarySystem::changeUnit(unsigned int id)
{
    if (!currentPerson || currentPerson->getType() != TypeOfReader::ADMINISTRATOR) {
        std::cerr << "Only admins can do that operation!" << "\n";
        return false;
    }
    
    std::ifstream file(fileUnits, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "File coulndnt open!\n";
        return false;
    }

    LibraryUnit* unit = nullptr;

    for (size_t i = 0; i < infoUnits.size(); i++)
    {
        if (infoUnits[i].uniqueNumber == id) {
            file.seekg(infoUnits[i].pos);
            unit = LibraryFactory::createUnitFromStream(file, infoUnits[i].type);
            if (!unit) {
                std::cerr << "Error in changeUnit!\n";
                file.close();
                return false;
            }

            try
            {
                units.push_back(unit);
                file.close();
            }
            catch (...)
            {
                delete unit;
                file.close();
                throw;
            }
        }
    }
    
    for (size_t i = 0; i < units.size(); i++)
    {
        if (units[i]->getId() == id) {
            
            try
            {
                bool res = units[i]->change();
                if (!res) {
                    return false;
                }
                rewriteUnitById(unit->getId());
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

bool LibrarySystem::rewriteUnitById(int id) {
    std::string path = "units.dat";
    std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for update.\n";
        return false;
    }

    for (const UniqueIDAndFilePositions& meta : infoUnits) {
        if (meta.uniqueNumber == id) {
            // Намери обекта в units
            for (LibraryUnit* unit : units) {
                if (unit->getId() == id) {
                    file.seekp(meta.pos);  // отиваме точно където е
                    unit->serialize(file);    // overwrite
                    file.close();
                    std::cout << "Successfully rewrote unit with ID: " << id << "\n";
                    return true;
                }
            }
            std::cerr << "Unit with ID found in meta but not in memory!\n";
            file.close();
            return false;
        }
    }

    std::cerr << "No metadata found for ID: " << id << "\n";
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
    for (size_t i = 0; i < infoUsers.size(); i++)
    {
        if (newItem->getUsername() == infoUsers[i].username) {
            std::cerr << "We have user with that username!";
            delete newItem;
            return;
        }
    }

    try
    {
        users.push_back(newItem);

    }
    catch (...)
    {
        delete newItem;
        throw;
    }
    
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
    std::string input;
    for (size_t i = 0; i < users.size(); i++)
    {
        std::string username = users[i]->getUsername();
        if (users[i]->getUsername() == name) {
            //  трябва да го махнем
            if (users[i]->getType() == TypeOfReader::ADMINISTRATOR && count == 1) {
                std::cerr << "Cannot remove cause  this is the last Admin in our system!\n";
                return;
            }

            // иначе можем да го махнем
            if (currentPerson->getUsername() == users[i]->getUsername() &&
                currentPerson->getPassword() == users[i]->getPassword())
            {
                if (!this->logout()) {
                    std::cerr << "Something happened!\n";
                    return;
                };

            }

            // иначе не е текущо логнатият

            std::cout << "Do you want to remove his books? 'yes' or 'no'\n";
            std::getline(std::cin, input);
            if (input == "yes") {
                std::vector<int> takenIds = users[i]->getTakenIds();
                for (size_t j = 0; j <  units.size(); j++)
                {
                    for (size_t k = 0; k  < takenIds.size(); k ++)
                    {
                        // ако са били заредени, ще ги махне, но те може да не са
                        if (units[j]->getId() == takenIds[k]) {
                            std::swap(units[j], units.back());
                            delete units.back();
                            units.pop_back();
                            std::cout << "Succesfully remove book with id: " << takenIds[k] << "\n";
                            break;
                        }
                    }
                }
            }

            else if (input != "no") {
                std::cerr << "invalid answer!\n";
                return;
            }

            std::swap(users[i], users.back());
            delete users.back();
            users.pop_back();
            std::cout << "Successfully removed user: " << name << "\n";

            return;
        }
    }
    std::cerr << "Not found!\n";
    return;
}

bool LibrarySystem::changeUser(const std::string& currentUser, const std::string& newPassword, const std::string& targetUser) {

    LibraryPerson* executor = findUserByUsername(currentUser);
    if (!executor) return false;

    // Без аргумент: текущият потребител сменя своята парола
    if (targetUser.empty()) {
        return executor->setPassword(newPassword);
    }

    // С аргумент: админ сменя паролата на друг потребител
    if (executor->getType() != TypeOfReader::ADMINISTRATOR) return false;  // Само админ може

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
        // иначе е намерен
        user->display();
        return true;
    }
    Date today = Date();
    std::vector<LibraryPerson*> filtered;

    if (option == "id") {
        unsigned int id = std::stoul(optionString);
        for (size_t i = 0; i < users.size(); i++)
        {
            
            // можем и да добавим някаква фукнция, но за простота ще го направим сега така
            Reader* hasToBeReader = dynamic_cast<Reader*>(users[i]);
            if (hasToBeReader == nullptr) {
                continue; // само читателите заемат книги
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
                // книгите които не е върнал в срок
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
                 // дали е взел повече от 5 книги
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
                // да не е вземал книги от повече от 3 месеца
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

void LibrarySystem::print()const
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
    std::cout << infoUnits.size();
    std::ifstream file(fileUnits,std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error with file while booksAll!\n";
        return;
    }

    //търсим в meta данните, ако е там -> принтираме, но не я зареждаме още

    for (size_t i = 0; i < infoUnits.size(); i++)
    {
        LibraryUnit* unit = nullptr;
        if (infoUnits[i].type == Type::BOOK) {
            try
            {
                file.seekg(infoUnits[i].pos, std::ios::beg);
                
                Type type;
                std::streampos posBeforeType = file.tellg(); // запазваме позицията
                file.read(reinterpret_cast<char*>(&type), sizeof(type));
                if (!file.good()) {
                    throw std::ios_base::failure("Error with file!");
                }

                // курсора обратно, за да може обектът сам да си прочете Type вътре
                file.seekg(posBeforeType);
                if (!file) {
                    throw std::ios_base::failure("Failed to seek back before reading unit!");
                }

                LibraryUnit* unit = LibraryFactory::createUnitFromStream(file, type);
                if (!unit) {
                    std::cerr << "Creating unit error!\n";
                    file.close();
                }
                
                unit->printBase();
                delete unit;
            }
            catch (...)
            {
                delete unit;
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

    //търсим в meta данните, ако е там -> зареждаме книгата
    for (size_t i = 0; i < infoUnits.size(); i++)
    {
        LibraryUnit* unit = nullptr;
        if (infoUnits[i].type == Type::PERIODICALS) {
            try
            {
                file.seekg(infoUnits[i].pos, std::ios::beg);
                Type type;
                std::streampos posBeforeType = file.tellg(); // запазваме позицията
                file.read(reinterpret_cast<char*>(&type), sizeof(type));
                if (!file.good()) {
                    throw std::ios_base::failure("Error with file!");
                }

                // курсора обратно, за да може обектът сам да си прочете Type вътре
                file.seekg(posBeforeType);
                if (!file) {
                    throw std::ios_base::failure("Failed to seek back before reading unit!");
                }



                unit = LibraryFactory::createUnitFromStream(file, type);
                if (!unit) {
                    std::cerr << "Creating unit error!\n";
                    file.close();
                }
             
                unit->printBase();
                delete unit;
            }
            catch (...)
            {
                delete unit;
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

    //търсим в meta данните, ако е там -> зареждаме книгата
    for (size_t i = 0; i < infoUnits.size(); i++)
    {
        LibraryUnit* unit = nullptr;
        if (infoUnits[i].type == Type::SERIES) {
            try
            {
                file.seekg(infoUnits[i].pos, std::ios::beg);

                Type type;
                std::streampos posBeforeType = file.tellg(); // запазваме позицията
                file.read(reinterpret_cast<char*>(&type), sizeof(type));
                if (!file.good()) {
                    throw std::ios_base::failure("Error with file!");
                }

                // курсора обратно, за да може обектът сам да си прочете Type вътре
                file.seekg(posBeforeType);
                if (!file) {
                    throw std::ios_base::failure("Failed to seek back before reading unit!");
                }

                 unit = LibraryFactory::createUnitFromStream(file, type);
                if (!unit) {
                    std::cerr << "Creating unit error!\n";
                    file.close();
                }

                unit->printBase();
                delete unit;

            }
            catch (...)
            {
                delete unit;
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

    
    //търсим в meta данните, ако е там -> зареждаме книгата
    for (size_t i = 0; i < infoUnits.size(); i++)
    {
        LibraryUnit* unit = nullptr;
            try
            {
                file.seekg(infoUnits[i].pos, std::ios::beg);


                Type type;
                std::streampos posBeforeType = file.tellg(); // запазваме позицията
                file.read(reinterpret_cast<char*>(&type), sizeof(type));
                if (!file.good()) {
                    throw std::ios_base::failure("Error with file!");
                }

                // курсора обратно, за да може обектът сам да си прочете Type вътре
                file.seekg(posBeforeType);
                if (!file) {
                    throw std::ios_base::failure("Failed to seek back before reading unit!");
                }


                unit = LibraryFactory::createUnitFromStream(file, type);
                if (!unit) {
                    std::cerr << "Creating unit error!\n";
                    file.close();
                }

                unit->printBase();
                delete unit;
            }
            catch (...)
            {
                delete unit;
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
    
    std::ifstream file(fileUnits, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error with seraching information in listInfo()!\n";
        return;
    }

    for (size_t i = 0; i < infoUnits.size(); i++)
    {
        file.seekg(infoUnits[i].pos);
        LibraryUnit* current = LibraryFactory::createUnitFromStream(file, infoUnits[i].type);
        if (current == nullptr)
        {
            std::cerr << "Error with creating unit in listInfo()!\n";
            return;
        }

        if (current->getType() == SERIES) {
            Series* itsASerie = dynamic_cast<Series*>(current);
            if (itsASerie!=nullptr && (itsASerie->getISBN() == ISNN_OR_ISBN || itsASerie->getISSN() == ISNN_OR_ISBN)) {
                itsASerie->display();
                delete current;
                return;
            }
        }
        
        if (current->getISSNorISBN() == ISNN_OR_ISBN) {
            try
            {
                current->display();
                delete current;
                return;
            }
            catch (...)
            {
                delete current;
                throw;
            }
        }
    }

    std::cerr << "Item was not found!" << "\n";
    return;
}

bool LibrarySystem::canAddNewUser(const std::string& name, const std::string& password) const
{
    // дали можем да добавин нов потребител
    // ако е в системата - не
    // ако има несъответствие с паролата - пак не

    // ако е добавен вече
    for (size_t i = 0; i < users.size(); i++)
    {
        // operator== of string
        if (users[i]->getUsername() == name) {
            return false;
        }
    }

    // ако ли пък не е добавен
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
