#include "Contact.h"
#include "ContactValidator.h"
#include "IRepository.h"
#include "FileRepository.h"
#include "PostgresRepository.h"
#include "RepositoryFactory.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <memory>

std::unique_ptr<IRepository> repo;
std::vector<Contact> contacts;
std::string currentStorageType;

void showMenu();
void showAllContacts();
void addContact();
void editContact();
void deleteContact();
void searchContacts();
void sortContacts();
void saveToFile();
void loadFromFile();
void changeStorage();
void printContact(const Contact& c, int index);
void printTableHeader();
std::string inputString(const std::string& prompt, bool required = false);
std::string inputPhone();
std::string inputEmail();
std::string inputBirthDate();
std::string inputName(const std::string& fieldName, bool required);
void managePhones(std::vector<PhoneInfo>& phones);
std::string getPhoneType();
void refreshContacts();

void refreshContacts() {
    if (repo) {
        repo->loadAll(contacts);
    }
}

int main() {
    std::cout << "ТЕЛЕФОННЫЙ СПРАВОЧНИК\n";
 
    StorageType storageType = RepositoryFactory::showSelectionMenu();
    repo = RepositoryFactory::create(storageType);
    currentStorageType = RepositoryFactory::storageTypeToString(storageType);
    
    std::cout << "\nИспользуется: " << repo->getStorageType() << std::endl;
    
    PostgresRepository* pgRepo = dynamic_cast<PostgresRepository*>(repo.get());
    if (pgRepo && !pgRepo->isConnected()) {
        std::cout << "Не удалось подключиться к PostgreSQL. Используется файловое хранилище.\n";
        repo = std::make_unique<FileRepository>("contacts.txt");
        currentStorageType = "файл (резервный)";
    }
    
    refreshContacts();
    
    if (contacts.empty()) {
        std::cout << "Хранилище пусто. Будет создан новый справочник.\n";
    } else {
        std::cout << "Загружено контактов: " << contacts.size() << "\n";
    }
    
    int choice;
    do {
        showMenu();
        std::cout << "Ваш выбор: ";
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        switch (choice) {
            case 1: showAllContacts(); break;
            case 2: addContact(); break;
            case 3: editContact(); break;
            case 4: deleteContact(); break;
            case 5: searchContacts(); break;
            case 6: sortContacts(); break;
            case 7: saveToFile(); break;
            case 8: loadFromFile(); break;
            case 9: changeStorage(); break;
            case 0: 
                std::cout << "\nВыход из программы\n";
                char save;
                std::cout << "Сохранить изменения перед выходом? (y/n): ";
                std::cin >> save;
                std::cin.ignore();
                if (save == 'y' || save == 'Y') {
                    if (repo) {
                        repo->saveAll(contacts);
                        std::cout << "Данные сохранены\n";
                    }
                }
                break;
        }
    } while (choice != 0);
    
    return 0;
}

void showMenu() {
    std::cout << "\n МЕНЮ\n";
    std::cout << "Текущее хранилище: " << currentStorageType << "\n";
    std::cout << "1. Показать все контакты\n";
    std::cout << "2. Добавить контакт\n";
    std::cout << "3. Редактировать контакт\n";
    std::cout << "4. Удалить контакт\n";
    std::cout << "5. Поиск контактов\n";
    std::cout << "6. Сортировка контактов\n";
    std::cout << "7. Сохранить в текущее хранилище\n";
    std::cout << "8. Загрузить из текущего хранилища\n";
    std::cout << "9. Сменить тип хранилища\n";
    std::cout << "0. Выход\n";
}

void changeStorage() {
    char confirm;
    std::cout << "При смене хранилища текущие несохранённые изменения могут быть потеряны.\n";
    std::cout << "Продолжить? (y/n): ";
    std::cin >> confirm;
    std::cin.ignore();
    
    if (confirm != 'y' && confirm != 'Y') {
        return;
    }
    
    StorageType newType = RepositoryFactory::showSelectionMenu();
    auto newRepo = RepositoryFactory::create(newType);
    
    PostgresRepository* pgRepo = dynamic_cast<PostgresRepository*>(newRepo.get());
    if (pgRepo && !pgRepo->isConnected()) {
        std::cout << "Не удалось подключиться к PostgreSQL.\n";
        return;
    }
    
    std::vector<Contact> newContacts;
    if (newRepo->loadAll(newContacts)) {
        repo = std::move(newRepo);
        contacts = newContacts;
        currentStorageType = RepositoryFactory::storageTypeToString(newType);
        std::cout << "Переключено на " << repo->getStorageType() << std::endl;
        std::cout << "Загружено контактов: " << contacts.size() << std::endl;
    } else {
        std::cout << "Не удалось загрузить данные из выбранного хранилища.\n";
    }
}

void printTableHeader() {
    std::cout << "\n" << std::string(160, '-') << "\n";
    std::cout << std::left 
              << std::setw(5) << "№"
              << std::setw(20) << "Фамилия"
              << std::setw(15) << "Имя"
              << std::setw(20) << "Отчество"
              << std::setw(30) << "Адрес"
              << std::setw(12) << "Дата рожд."
              << std::setw(30) << "Email"
              << std::setw(50) << "Телефоны\n";
    std::cout << std::string(160, '-') << "\n";
}

void printContact(const Contact& c, int index) {
    std::string lastName = c.lastName;
    if (lastName.length() > 18) lastName = lastName.substr(0, 15) + "...";
    
    std::string firstName = c.firstName;
    if (firstName.length() > 13) firstName = firstName.substr(0, 10) + "...";
    
    std::string patronymic = c.patronymic;
    if (patronymic.length() > 18) patronymic = patronymic.substr(0, 15) + "...";
    
    std::string address = c.address;
    if (address.length() > 23) address = address.substr(0, 20) + "...";
    
    std::string email = c.email;
    if (email.length() > 23) email = email.substr(0, 20) + "...";
    
    std::string phones = c.phonesAsString();
    
    std::cout << std::left 
              << std::setw(5) << index + 1
              << std::setw(20) << lastName
              << std::setw(15) << firstName
              << std::setw(20) << patronymic
              << std::setw(25) << address
              << std::setw(12) << c.birthDate
              << std::setw(25) << email
              << std::setw(40) << phones
              << "\n";
}

void showAllContacts() {
    if (contacts.empty()) {
        std::cout << "\nСправочник пуст.\n";
        return;
    }
    
    printTableHeader();
    for (size_t i = 0; i < contacts.size(); ++i) {
        printContact(contacts[i], i);
    }
    std::cout << "\nВсего контактов: " << contacts.size() << "\n";
}

std::string inputString(const std::string& prompt, bool required) {
    std::string input;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, input);
        
        if (!required && input.empty()) {
            return input;
        }
        
        if (required && input.empty()) {
            std::cout << "Это поле обязательно для заполнения.\n";
            continue;
        }
        
        break;
    }
    
    return input;
}

std::string inputName(const std::string& fieldName, bool required) {
    std::string name;
    while (true) {
        name = inputString(fieldName + ": ", required);
        
        if (!required && name.empty()) {
            return name;
        }
        
        if (required && name.empty()) {
            std::cout << "Поле \"" << fieldName << "\" обязательно для заполнения.\n";
            continue;
        }
        
        if (!ContactValidator::isValidName(name)) {
            std::cout << "Ошибка. " << fieldName << " должно:\n"
                      << "- Начинаться с заглавной буквы\n"
                      << "- Содержать только буквы, дефис и пробел\n"
                      << "- Не начинаться и не заканчиваться на дефис\n"
                      << "- Не содержать двойные дефисы или пробелы\n";
            continue;
        }
        
        break;
    }
    
    return ContactValidator::normalizeName(name);
}

std::string inputEmail(bool required) {
    std::string email;
    while (true) {
        email = inputString("Email: ", required);
        
        if (!required && email.empty()) {
            return email;
        }
        
        if (required && email.empty()) {
            std::cout << "Email обязателен для заполнения\n";
            continue;
        }
        
        if (!email.empty() && !ContactValidator::isValidEmail(email)) {
            std::cout << "Ошибка. Некорректный email. Формат: user@domain.com\n";
            continue;
        }
        
        break;
    }
    
    return ContactValidator::normalizeEmail(email);
}

std::string inputBirthDate() {
    std::string date;
    do {
        date = inputString("Дата рождения (ДД.ММ.ГГГГ): ", false);
        if (date.empty()) return date;
        
        if (!ContactValidator::isValidBirthDate(date)) {
            std::cout << "Ошибка. Некорректная дата рождения.\n"
                      << "- Дата должна быть в прошлом\n"
                      << "- Формат: ДД.ММ.ГГГГ\n"
                      << "- Должен быть корректный день для месяца/года\n";
            date.clear();
        }
    } while (date.empty());
    
    return date;
}

std::string getPhoneType() {
    std::cout << "Тип телефона:\n";
    std::cout << "1. рабочий\n";
    std::cout << "2. домашний\n";
    std::cout << "3. служебный\n";
    std::cout << "4. мобильный\n";
    
    int choice;
    do {
        std::cout << "Выберите тип (1-4): ";
        std::cin >> choice;
        std::cin.ignore();
    } while (choice < 1 || choice > 4);
    
    switch (choice) {
        case 1: return "рабочий";
        case 2: return "домашний";
        case 3: return "служебный";
        case 4: return "мобильный";
        default: return "рабочий";
    }
}

std::string inputPhone() {
    std::string phone;
    do {
        std::cout << "Номер телефона: ";
        std::getline(std::cin, phone);
        
        if (phone.empty()) return phone;
        
        std::string error = ContactValidator::validatePhoneWithDetails(phone);
        if (!error.empty()) {
            std::cout << error << "\n";
            phone.clear();
        }
    } while (phone.empty());
    
    return ContactValidator::normalizePhone(phone);
}

void managePhones(std::vector<PhoneInfo>& phones) {
    int choice;
    do {
        std::cout << "\nУправление телефонами \n";
        if (phones.empty()) {
            std::cout << "Список телефонов пуст.\n";
        } else {
            std::cout << "Текущие телефоны:\n";
            for (size_t i = 0; i < phones.size(); ++i) {
                std::cout << "  " << i + 1 << ". " << phones[i].displayString() << "\n";
            }
        }
        std::cout << "\n1. Добавить телефон\n";
        std::cout << "2. Редактировать телефон\n";
        std::cout << "3. Удалить телефон\n";
        std::cout << "0. Готово\n";
        std::cout << "Выбор: ";
        std::cin >> choice;
        std::cin.ignore();
        
        switch (choice) {
            case 1: {
                std::string type = getPhoneType();
                std::string number = inputPhone();
                if (!number.empty()) {
                    phones.push_back({type, number});
                    std::cout << "Телефон добавлен\n";
                }
                break;
            }
            case 2: {
                if (phones.empty()) {
                    std::cout << "Нет телефонов для редактирования.\n";
                    break;
                }
                int idx;
                std::cout << "Номер телефона для редактирования (1-" << phones.size() << "): ";
                std::cin >> idx;
                std::cin.ignore();
                if (idx >= 1 && idx <= static_cast<int>(phones.size())) {
                    std::string newType = getPhoneType();
                    std::string newNumber = inputPhone();
                    if (!newNumber.empty()) {
                        phones[idx - 1].type = newType;
                        phones[idx - 1].number = newNumber;
                        std::cout << "Телефон обновлен\n";
                    }
                }
                break;
            }
            case 3: {
                if (phones.empty()) {
                    std::cout << "Нет телефонов для удаления.\n";
                    break;
                }
                int idx;
                std::cout << "Номер телефона для удаления (1-" << phones.size() << "): ";
                std::cin >> idx;
                std::cin.ignore();
                if (idx >= 1 && idx <= static_cast<int>(phones.size())) {
                    phones.erase(phones.begin() + idx - 1);
                    std::cout << "Телефон удален\n";
                }
                break;
            }
        }
    } while (choice != 0);
}

void addContact() {
    std::cout << "\nДОБАВЛЕНИЕ НОВОГО КОНТАКТА \n";
    
    Contact c;
    
    c.lastName = inputName("Фамилия", true);
    c.firstName = inputName("Имя", true);
    c.patronymic = inputName("Отчество", false);
    c.address = inputString("Адрес: ", false);
    c.birthDate = inputBirthDate();
    c.email = inputEmail(true);
    
    managePhones(c.phones);
    
    if (c.phones.empty()) {
        std::cout << "Ошибка. Должен быть указан хотя бы один телефон.\n";
        return;
    }
    
    contacts.push_back(c);
    
    if (repo) {
        repo->addContact(c);
    }
    
    std::cout << "\nКонтакт успешно добавлен\n";
}

void editContact() {
    if (contacts.empty()) {
        std::cout << "\nСправочник пуст. Нечего редактировать.\n";
        return;
    }
    
    showAllContacts();
    
    int idx;
    std::cout << "\nВведите номер контакта для редактирования (1-" << contacts.size() << "): ";
    std::cin >> idx;
    std::cin.ignore();
    
    if (idx < 1 || idx > static_cast<int>(contacts.size())) {
        std::cout << "Неверный номер контакта\n";
        return;
    }
    
    Contact& c = contacts[idx - 1];
    
    std::cout << "\nРЕДАКТИРОВАНИЕ КОНТАКТА \n";
    std::cout << "(оставьте поле пустым, чтобы не изменять)\n\n";
    
    std::string newValue;
    
    newValue = inputName("Фамилия", false);
    if (!newValue.empty()) c.lastName = newValue;
    
    newValue = inputName("Имя", false);
    if (!newValue.empty()) c.firstName = newValue;
    
    newValue = inputName("Отчество", false);
    if (!newValue.empty()) c.patronymic = newValue;
    
    newValue = inputString("Адрес: ", false);
    if (!newValue.empty()) c.address = newValue;
    
    newValue = inputBirthDate();
    if (!newValue.empty()) c.birthDate = newValue;
    
    newValue = inputEmail(false);
    if (!newValue.empty()) c.email = newValue;
    
    std::cout << "\nРедактирование телефонов:\n";
    managePhones(c.phones);
    
    if (repo) {
        repo->updateContact(idx - 1, c);
    }
    
    std::cout << "\nКонтакт успешно обновлен\n";
}

void deleteContact() {
    if (contacts.empty()) {
        std::cout << "\nСправочник пуст. Нечего удалять.\n";
        return;
    }
    
    showAllContacts();
    
    int idx;
    std::cout << "\nВведите номер контакта для удаления (1-" << contacts.size() << "): ";
    std::cin >> idx;
    std::cin.ignore();
    
    if (idx < 1 || idx > static_cast<int>(contacts.size())) {
        std::cout << "Неверный номер контакта\n";
        return;
    }
    
    char confirm;
    std::cout << "Вы уверены, что хотите удалить контакт \"" 
              << contacts[idx - 1].firstName << " " << contacts[idx - 1].lastName << "\"? (y/n): ";
    std::cin >> confirm;
    std::cin.ignore();
    
    if (confirm == 'y' || confirm == 'Y') {
        if (repo) {
            repo->deleteContact(idx - 1);
        }
        contacts.erase(contacts.begin() + idx - 1);
        std::cout << "Контакт удален\n";
    } else {
        std::cout << "Удаление отменено.\n";
    }
}

void searchContacts() {
    if (contacts.empty()) {
        std::cout << "\nСправочник пуст.\n";
        return;
    }
    
    std::cout << "\nПОИСК КОНТАКТОВ \n";
    std::cout << "1. Поиск по всем полям\n";
    std::cout << "2. Поиск по фамилии\n";
    std::cout << "3. Поиск по имени\n";
    std::cout << "4. Поиск по email\n";
    std::cout << "5. Поиск по телефону\n";
    std::cout << "Выберите тип поиска: ";
    
    int searchType;
    std::cin >> searchType;
    std::cin.ignore();
    
    std::string query;
    std::cout << "Введите текст для поиска: ";
    std::getline(std::cin, query);
    
    if (query.empty()) {
        std::cout << "Поисковый запрос не может быть пустым\n";
        return;
    }
    
    std::string lowerQuery = query;
    for (char& c : lowerQuery) c = std::tolower(c);
    
    std::vector<Contact> results;
    
    for (const Contact& c : contacts) {
        bool found = false;
        
        switch (searchType) {
            case 1: {
                std::string fullText = c.lastName + " " + c.firstName + " " + c.patronymic + " " + c.address + " " + c.email;
                std::string lowerFull = fullText;
                for (char& ch : lowerFull) ch = std::tolower(ch);
                if (lowerFull.find(lowerQuery) != std::string::npos) found = true;
                break;
            }
            case 2: {
                std::string lowerLast = c.lastName;
                for (char& ch : lowerLast) ch = std::tolower(ch);
                if (lowerLast.find(lowerQuery) != std::string::npos) found = true;
                break;
            }
            case 3: {
                std::string lowerFirst = c.firstName;
                for (char& ch : lowerFirst) ch = std::tolower(ch);
                if (lowerFirst.find(lowerQuery) != std::string::npos) found = true;
                break;
            }
            case 4: {
                std::string lowerEmail = c.email;
                for (char& ch : lowerEmail) ch = std::tolower(ch);
                if (lowerEmail.find(lowerQuery) != std::string::npos) found = true;
                break;
            }
            case 5: {
                for (const PhoneInfo& p : c.phones) {
                    std::string lowerPhone = p.number;
                    for (char& ch : lowerPhone) ch = std::tolower(ch);
                    if (lowerPhone.find(lowerQuery) != std::string::npos) {
                        found = true;
                        break;
                    }
                }
                break;
            }
        }
        
        if (found) {
            results.push_back(c);
        }
    }
    
    if (results.empty()) {
        std::cout << "\nКонтакты не найдены.\n";
    } else {
        std::cout << "\nНайдено контактов: " << results.size() << "\n";
        printTableHeader();
        for (size_t i = 0; i < results.size(); ++i) {
            printContact(results[i], i);
        }
    }
}

void sortContacts() {
    if (contacts.empty()) {
        std::cout << "\nСправочник пуст.\n";
        return;
    }
    
    std::cout << "\n--- СОРТИРОВКА КОНТАКТОВ ---\n";
    std::cout << "1. По фамилии\n";
    std::cout << "2. По имени\n";
    std::cout << "3. По дате рождения\n";
    std::cout << "4. По email\n";
    std::cout << "Выберите поле для сортировки: ";
    
    int sortType;
    std::cin >> sortType;
    std::cin.ignore();
    
    switch (sortType) {
        case 1:
            std::sort(contacts.begin(), contacts.end(), [](const Contact& a, const Contact& b) {
                return a.lastName < b.lastName;
            });
            std::cout << "Контакты отсортированы по фамилии.\n";
            break;
        case 2:
            std::sort(contacts.begin(), contacts.end(), [](const Contact& a, const Contact& b) {
                return a.firstName < b.firstName;
            });
            std::cout << "Контакты отсортированы по имени.\n";
            break;
        case 3:
            std::sort(contacts.begin(), contacts.end(), [](const Contact& a, const Contact& b) {
                return a.birthDate < b.birthDate;
            });
            std::cout << "Контакты отсортированы по дате рождения.\n";
            break;
        case 4:
            std::sort(contacts.begin(), contacts.end(), [](const Contact& a, const Contact& b) {
                return a.email < b.email;
            });
            std::cout << "Контакты отсортированы по email.\n";
            break;
        default:
            std::cout << "Неверный выбор.\n";
            return;
    }
    
    if (repo) {
        repo->saveAll(contacts);
    }
    
    showAllContacts();
}

void saveToFile() {
    if (repo) {
        if (repo->saveAll(contacts)) {
            std::cout << "\nДанные успешно сохранены в " << repo->getStorageType() << "\n";
            std::cout << "Сохранено контактов: " << contacts.size() << "\n";
        } else {
            std::cout << "\nОшибка при сохранении данных.\n";
        }
    }
}

void loadFromFile() {
    if (repo) {
        if (repo->loadAll(contacts)) {
            std::cout << "\nДанные успешно загружены из " << repo->getStorageType() << "\n";
            std::cout << "Загружено контактов: " << contacts.size() << "\n";
        } else {
            std::cout << "\nОшибка при загрузке данных.\n";
        }
    }
}