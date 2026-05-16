#include "contactvalidator.h"
#include <QRegularExpression>
#include <QDate>

bool ContactValidator::isValidName(const QString &name) {
    QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) return false;
    
    // 1. Начало с заглавной буквы
    if (!trimmed[0].isUpper()) {
        return false;
    }
    
    // 2. Проверка формата: заглавная буква, затем строчные
    QRegularExpression re(R"(^\p{Lu}[\p{Ll}\s\-]*$)");
    if (!re.match(trimmed).hasMatch()) return false;
    
    // 3. Запрет начинать или заканчивать на дефис
    if (trimmed.startsWith('-') || trimmed.endsWith('-')) return false;
    
    // 4. Запрет два подряд дефиса или пробела
    if (trimmed.contains("--") || trimmed.contains("  ")) return false;
    
    // 5. Проверяем, что после дефиса идет заглавная буква (для двойных фамилий)
    for (int i = 0; i < trimmed.length(); ++i) {
        if (trimmed[i] == '-' && i + 1 < trimmed.length()) {
            if (!trimmed[i + 1].isUpper()) {
                return false;
            }
        }
    }
    
    return true;
}

QString ContactValidator::normalizePhone(const QString &phone) {
    QString digits = phone;
    digits.remove(QRegularExpression("[^\\d+]"));
    if (digits.startsWith("+")) {
        digits.remove(0, 1);
    }
    return digits;
}

QString ContactValidator::normalizeName(const QString &name) {
    QString result = name.trimmed();
    
    while (result.contains("  ")) {
        result = result.replace("  ", " ");
    }
    
    return result;
}

bool ContactValidator::isValidEmail(const QString &email) {
    QString trimmed = email.trimmed();
    if (trimmed.isEmpty()) return false;
    
    int atPos = trimmed.indexOf('@');
    if (atPos == -1) return false;
    QString user = trimmed.left(atPos).trimmed();
    QString domain = trimmed.mid(atPos + 1).trimmed();
    if (user.isEmpty() || domain.isEmpty()) return false;
    
    QString cleanEmail = user + "@" + domain;
    
    QRegularExpression re(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return re.match(cleanEmail).hasMatch();
}

bool ContactValidator::isValidPhoneFormat(const QString &phone) {
    return validatePhoneWithDetails(phone).isEmpty();
}

QString ContactValidator::validatePhoneWithDetails(const QString &phone) {
    QString input = phone.trimmed();
    if (input.isEmpty()) return "Номер телефона не может быть пустым";
    
    int open = input.count('(');
    int close = input.count(')');
    if (open != close) {
        return "Непарные скобки в номере телефона";
    }
    
    if (input.contains("--")) {
        return "Обнаружены двойные дефисы в номере";
    }
    if (input.contains("  ")) {
        return "Обнаружены двойные пробелы в номере";
    }
    
    QRegularExpression re(R"(^(\+?[1-9]\d{0,2}[\s\-]*)?(\(?\d{2,5}\)?[\s\-]*)?\d{2,}[\s\-]*\d{2,}[\s\-]*\d{0,}$)");
    
    if (!re.match(input).hasMatch()) {
        return "Некорректный формат номера телефона";
    }
    
    QString digits = normalizePhone(input);
    if (digits.isEmpty()) {
        return "Номер не содержит цифр";
    }
    
    bool hasPlus = input.contains('+');
    int minLength = hasPlus ? 8 : 7;
    int maxLength = 15;
    
    if (digits.length() < minLength) {
        if (hasPlus) {
            return "Слишком короткий номер. Минимум " + QString::number(minLength) + " цифр (с учётом кода страны)";
        } else {
            return "Слишком короткий номер. Минимум " + QString::number(minLength) + " цифр";
        }
    }
    
    if (digits.length() > maxLength) {
        return "Слишком длинный номер. Максимум " + QString::number(maxLength) + " цифр";
    }
    
    if (digits.startsWith("7") || digits.startsWith("8")) {
        if (digits.length() != 11) {
            return "Российский номер должен содержать 11 цифр (сейчас " + QString::number(digits.length()) + ")";
        }
    }
    
    QRegularExpression invalidChars(R"([^0-9\s\-\+\(\)])");
    if (input.contains(invalidChars)) {
        return "Номер содержит недопустимые символы. Разрешены только цифры, пробелы, дефисы, скобки и знак +";
    }
    
    if (input.contains('+') && input.indexOf('+') != 0) {
        return "Знак '+' может быть только в начале номера";
    }
    
    return ""; 
}

bool ContactValidator::isValidBirthDate(const QDate &date) {
    if (!date.isValid()) return false;
    
    if (date >= QDate::currentDate()) return false;
    return true;
}
