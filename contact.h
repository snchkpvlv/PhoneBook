#ifndef CONTACT_H
#define CONTACT_H

#include <QString>
#include <QList>
#include <QDate>

struct PhoneInfo {
    QString type; 
    QString number; 
    
    QString displayString() const {
        return type + ": " + number;
    }
};

struct Contact {
    QString firstName;
    QString lastName;
    QString patronymic;
    QString address;
    QDate birthDate;
    QString email;
    QList<PhoneInfo> phones; 

    QStringList toDisplayList() const;
    
    QString phonesAsString() const;
};

#endif