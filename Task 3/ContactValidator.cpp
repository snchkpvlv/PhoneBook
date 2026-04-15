#include "ContactValidator.h"
#include <regex>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <string>

bool isUpperCaseLatin(char c) {
    return (c >= 'A' && c <= 'Z');
}

bool isUpperCaseRussian(const std::string& str, size_t pos) {
    unsigned char c1 = static_cast<unsigned char>(str[pos]);
    if (c1 >= 0xD0 && c1 <= 0xD1) {
        unsigned char c2 = static_cast<unsigned char>(str[pos + 1]);
        if (c1 == 0xD0 && c2 >= 0x90 && c2 <= 0xAF) return true;
        if (c1 == 0xD0 && c2 == 0x81) return true;
    }
    return false;
}

bool isValidNameChars(const std::string& str) {
    for (size_t i = 0; i < str.length(); ++i) {
        unsigned char c = str[i];

        if (c & 0x80) {
            int bytes = 0;
            if ((c & 0xE0) == 0xC0) bytes = 2;
            else if ((c & 0xF0) == 0xE0) bytes = 2;
            else if ((c & 0xF8) == 0xF0) bytes = 2;
            else bytes = 1;
            
            if (bytes > 1) {
                i += bytes - 1;
                continue;
            }
        } else {
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) continue;
            if (c >= '0' && c <= '9') continue;
            if (c == ' ' || c == '-') continue;
            return false;
        }
    }
    return true;
}

bool ContactValidator::isValidName(const std::string &name) {
    std::string trimmed = normalizeName(name);
    if (trimmed.empty()) return false;
    
    if (trimmed.empty()) return false;
    
    char firstChar = trimmed[0];

    bool isUpper = (firstChar >= 'A' && firstChar <= 'Z');

    if (!isUpper && trimmed.length() >= 2) {
        isUpper = isUpperCaseRussian(trimmed, 0);
    }
    
    if (!isUpper) return false;

    if (!isValidNameChars(trimmed)) return false;

    if (trimmed.find("--") != std::string::npos) return false;
    if (trimmed.find("  ") != std::string::npos) return false;

    if (trimmed.front() == '-' || trimmed.back() == '-') return false;
    if (trimmed.front() == ' ' || trimmed.back() == ' ') return false;
    
    return true;
}

std::string ContactValidator::normalizeName(const std::string &name) {
    size_t start = name.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    
    size_t end = name.find_last_not_of(" \t\n\r");
    std::string result = name.substr(start, end - start + 1);
    
    bool inSpace = false;
    std::string normalized;
    for (char c : result) {
        if (c == ' ') {
            if (!inSpace) {
                normalized += c;
                inSpace = true;
            }
        } else {
            normalized += c;
            inSpace = false;
        }
    }
    
    return normalized;
}

bool ContactValidator::isValidEmail(const std::string &email) {
    std::string trimmed = normalizeEmail(email);
    if (trimmed.empty()) return false;
    
    std::regex re(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return std::regex_match(trimmed, re);
}

std::string ContactValidator::normalizeEmail(const std::string &email) {
    size_t start = email.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    
    size_t end = email.find_last_not_of(" \t\n\r");
    std::string result = email.substr(start, end - start + 1);
    
    size_t atPos = result.find('@');
    if (atPos != std::string::npos) {
        std::string user = result.substr(0, atPos);
        std::string domain = result.substr(atPos + 1);
        
        user = normalizeName(user);
        domain = normalizeName(domain);
        
        result = user + "@" + domain;
    }
    
    return result;
}

bool ContactValidator::isValidPhoneFormat(const std::string &phone) {
    return validatePhoneWithDetails(phone).empty();
}

std::string ContactValidator::validatePhoneWithDetails(const std::string &phone) {
    std::string input = normalizePhone(phone);
    if (input.empty()) return "Номер телефона не может быть пустым";
    
    int open = 0, close = 0;
    for (char c : input) {
        if (c == '(') open++;
        if (c == ')') close++;
    }
    if (open != close) return "Непарные скобки в номере телефона";
    
    if (input.find("--") != std::string::npos) return "Обнаружены двойные дефисы в номере";
    if (input.find("  ") != std::string::npos) return "Обнаружены двойные пробелы в номере";
    
    std::regex re(R"(^(\+7|8|7)\(?\d{3}\)?\d{3}-?\d{2}-?\d{2}$)");
    
    if (!std::regex_match(input, re)) {
        return "Некорректный формат номера телефона.\n"
               "Допустимые форматы:\n"
               "+7XXXXXXXXXX (11 цифр)\n"
               "8XXXXXXXXXX (11 цифр)\n"
               "+7(XXX)XXXXXXX\n"
               "8(XXX)XXXXXXX\n"
               "+7(XXX)XXX-XX-XX\n"
               "8(XXX)XXX-XX-XX\n";
    }
    
    int digitCount = 0;
    for (char c : input) {
        if (std::isdigit(c)) digitCount++;
    }
    
    if (digitCount != 11) {
        return "Номер должен содержать 11 цифр (сейчас " + std::to_string(digitCount) + ")";
    }
    
    return "";
}

std::string ContactValidator::normalizePhone(const std::string &phone) {
    size_t start = phone.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    
    size_t end = phone.find_last_not_of(" \t\n\r");
    std::string result = phone.substr(start, end - start + 1);
    
    bool inSpace = false;
    std::string normalized;
    for (char c : result) {
        if (c == ' ') {
            if (!inSpace) {
                normalized += c;
                inSpace = true;
            }
        } else {
            normalized += c;
            inSpace = false;
        }
    }
    
    return normalized;
}

bool ContactValidator::isValidBirthDate(const std::string &dateStr) {
    std::string normalized = normalizeBirthDate(dateStr);
    if (normalized.empty()) return false;
    
    int day, month, year;
    char dot1, dot2;
    std::stringstream ss(normalized);
    ss >> day >> dot1 >> month >> dot2 >> year;
    
    if (ss.fail() || dot1 != '.' || dot2 != '.') return false;
    if (year < 1900 || year > 2026) return false;
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > getDaysInMonth(month, year)) return false;
    
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    
    if (year > now->tm_year + 1900) return false;
    if (year == now->tm_year + 1900 && month > now->tm_mon + 1) return false;
    if (year == now->tm_year + 1900 && month == now->tm_mon + 1 && day >= now->tm_mday) return false;
    
    return true;
}

bool ContactValidator::isValidBirthDate(const std::tm &date) {
    if (date.tm_year < 0 || date.tm_mon < 0 || date.tm_mday < 0) return false;
    if (date.tm_mon + 1 < 1 || date.tm_mon + 1 > 12) return false;
    if (date.tm_mday < 1 || date.tm_mday > getDaysInMonth(date.tm_mon + 1, date.tm_year + 1900)) return false;
    
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    
    if (date.tm_year + 1900 > now->tm_year + 1900) return false;
    if (date.tm_year + 1900 == now->tm_year + 1900 && date.tm_mon > now->tm_mon) return false;
    if (date.tm_year + 1900 == now->tm_year + 1900 && date.tm_mon == now->tm_mon && date.tm_mday >= now->tm_mday) return false;
    
    return true;
}

std::string ContactValidator::normalizeBirthDate(const std::string &dateStr) {
    std::string result = normalizeName(dateStr);
    
    if (result.length() != 10) return "";
    if (result[2] != '.' || result[5] != '.') return "";
    
    return result;
}

bool ContactValidator::isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int ContactValidator::getDaysInMonth(int month, int year) {
    static const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    if (month < 1 || month > 12) return 0;
    if (month == 2 && isLeapYear(year)) return 29;
    return daysInMonth[month - 1];
}