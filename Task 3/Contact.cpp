#include "Contact.h"
#include <sstream>

std::string PhoneInfo::displayString() const {
    return type + ": " + number;
}

std::vector<std::string> Contact::toDisplayList() const {
    return {
        lastName,
        firstName,
        patronymic,
        address,
        birthDate,
        email,
        phonesAsString()
    };
}

std::string Contact::phonesAsString() const {
    std::stringstream ss;
    for (size_t i = 0; i < phones.size(); ++i) {
        if (i > 0) ss << "; ";
        ss << phones[i].displayString();
    }
    return ss.str();
}