#include "FileStorage.h"
#include "ContactValidator.h"
#include <fstream>
#include <iostream>
#include <sstream>

bool FileStorage::saveToFile(const std::vector<Contact>& contacts, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    for (const Contact& c : contacts) {
        file << c.lastName << "|"
             << c.firstName << "|"
             << c.patronymic << "|"
             << c.address << "|"
             << c.birthDate << "|"
             << c.email << "|";
        
        for (size_t i = 0; i < c.phones.size(); ++i) {
            if (i > 0) file << ";";
            file << c.phones[i].type << ":" << c.phones[i].number;
        }
        file << "\n";
    }
    
    file.close();
    return true;
}

bool FileStorage::loadFromFile(std::vector<Contact>& contacts, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    contacts.clear();
    std::string line;
    int lineNumber = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;

        if (line.empty()) continue;
        
        std::stringstream ss(line);
        std::string lastName, firstName, patronymic, address, birthDate, email, phonesStr;
        
        std::getline(ss, lastName, '|');
        std::getline(ss, firstName, '|');
        std::getline(ss, patronymic, '|');
        std::getline(ss, address, '|');
        std::getline(ss, birthDate, '|');
        std::getline(ss, email, '|');
        std::getline(ss, phonesStr);

        if (!ContactValidator::isValidName(firstName) || 
            !ContactValidator::isValidName(lastName)) {
            std::cerr << "Ошибка в строке " << lineNumber << ": некорректное имя или фамилия\n";
            continue;
        }
        
        if (email.empty() || !ContactValidator::isValidEmail(email)) {
            std::cerr << "Ошибка в строке " << lineNumber << ": некорректный email\n";
            continue;
        }
        
        if (!birthDate.empty() && !ContactValidator::isValidBirthDate(birthDate)) {
            std::cerr << "Ошибка в строке " << lineNumber << ": некорректная дата рождения\n";
            continue;
        }
        
        Contact c;
        c.lastName = lastName;
        c.firstName = firstName;
        c.patronymic = patronymic;
        c.address = address;
        c.birthDate = birthDate;
        c.email = email;

        std::stringstream phoneSs(phonesStr);
        std::string phoneItem;
        while (std::getline(phoneSs, phoneItem, ';')) {
            size_t colonPos = phoneItem.find(':');
            if (colonPos != std::string::npos) {
                PhoneInfo phone;
                phone.type = phoneItem.substr(0, colonPos);
                phone.number = phoneItem.substr(colonPos + 1);
                
                if (!ContactValidator::isValidPhoneFormat(phone.number)) {
                    std::cerr << "Ошибка в строке " << lineNumber << ": некорректный телефон\n";
                    continue;
                }
                c.phones.push_back(phone);
            }
        }
        
        if (c.phones.empty()) {
            std::cerr << "Ошибка в строке " << lineNumber << ": нет ни одного телефона\n";
            continue;
        }
        
        contacts.push_back(c);
    }
    
    file.close();
    return true;
}

bool FileStorage::fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}