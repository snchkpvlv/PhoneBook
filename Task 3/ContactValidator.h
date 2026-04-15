#ifndef CONTACTVALIDATOR_H
#define CONTACTVALIDATOR_H

#include <string>
#include <ctime>

class ContactValidator {
public:
    static bool isValidName(const std::string &name);
    static std::string normalizeName(const std::string &name);
    
    static bool isValidEmail(const std::string &email);
    static std::string normalizeEmail(const std::string &email);
    
    static bool isValidPhoneFormat(const std::string &phone);
    static std::string validatePhoneWithDetails(const std::string &phone);
    static std::string normalizePhone(const std::string &phone);
    
    static bool isValidBirthDate(const std::string &dateStr);
    static bool isValidBirthDate(const std::tm &date);
    static std::string normalizeBirthDate(const std::string &dateStr);
    
    static bool isLeapYear(int year);
    static int getDaysInMonth(int month, int year);
};

#endif