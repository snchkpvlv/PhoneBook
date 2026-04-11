#ifndef CONTACT_H
#define CONTACT_H

#include <string>
#include <vector>
#include <ctime>

struct PhoneInfo {
    std::string type;      
    std::string number;    
    
    std::string displayString() const;
};

struct Contact {
    std::string firstName;
    std::string lastName;
    std::string patronymic;
    std::string address;
    std::string birthDate;    
    std::string email;
    std::vector<PhoneInfo> phones;
    
    std::vector<std::string> toDisplayList() const;
    std::string phonesAsString() const;
};

#endif