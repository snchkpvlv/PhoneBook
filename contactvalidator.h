#ifndef CONTACTVALIDATOR_H
#define CONTACTVALIDATOR_H

#include <QString>
#include <QDate>
#include <QRegularExpression>

class ContactValidator {
public:
    static bool isValidName(const QString &name);
    static QString normalizeName(const QString &name);
    
    static bool isValidEmail(const QString &email);
    
    static QString normalizePhone(const QString &phone);
    static bool isValidPhoneFormat(const QString &phone);
    static QString validatePhoneWithDetails(const QString &phone); 
    
    static bool isValidBirthDate(const QDate &date);
};

#endif