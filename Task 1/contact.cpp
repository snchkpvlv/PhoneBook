#include "contact.h"

QStringList Contact::toDisplayList() const {
    QString phonesStr = phonesAsString();
    
    return {
        lastName,
        firstName,
        patronymic,
        address,
        birthDate.toString("dd.MM.yyyy"),
        email,
        phonesStr
    };
}

QString Contact::phonesAsString() const {
    QStringList phoneStrings;
    for (const PhoneInfo& p : phones) {
        phoneStrings << p.displayString();
    }
    return phoneStrings.join("; ");
}