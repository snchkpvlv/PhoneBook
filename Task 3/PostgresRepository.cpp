#include "PostgresRepository.h"
#include "ContactValidator.h"
#include <libpq-fe.h>
#include <iostream>
#include <sstream>

PostgresRepository::PostgresRepository(const std::string& connStr) 
    : conn(nullptr), connectionString(connStr), connected(false) {
    
    conn = PQconnectdb(connectionString.c_str());
    
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Ошибка подключения к PostgreSQL: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        conn = nullptr;
        connected = false;
        return;
    }
    
    connected = true;
    std::cout << "Подключение к PostgreSQL установлено" << std::endl;
    
    if (!createTablesIfNotExist()) {
        std::cerr << "Ошибка создания таблиц" << std::endl;
        connected = false;
    }
}

PostgresRepository::~PostgresRepository() {
    if (conn) {
        PQfinish(conn);
        conn = nullptr;
    }
}

bool PostgresRepository::executeQuery(const std::string& query) {
    if (!connected || !conn) return false;
    
    PGresult* res = PQexec(conn, query.c_str());
    ExecStatusType status = PQresultStatus(res);
    
    bool success = (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK);
    
    if (!success) {
        std::cerr << "Ошибка выполнения запроса: " << PQerrorMessage(conn) << std::endl;
        std::cerr << "Запрос: " << query << std::endl;
    }
    
    PQclear(res);
    return success;
}

PGresult* PostgresRepository::executeQueryResult(const std::string& query) {
    if (!connected || !conn) return nullptr;
    
    PGresult* res = PQexec(conn, query.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка выполнения запроса: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return nullptr;
    }
    
    return res;
}

std::string PostgresRepository::escapeString(const std::string& str) const {
    if (!conn) return str;
    
    char* escaped = PQescapeLiteral(conn, str.c_str(), str.length());
    if (!escaped) return str;
    
    std::string result(escaped);
    PQfreemem(escaped);
    return result;
}

std::string PostgresRepository::formatDateForPostgres(const std::string& date) const {
    if (date.empty() || date.length() != 10) return "";
    
    std::string day = date.substr(0, 2);
    std::string month = date.substr(3, 2);
    std::string year = date.substr(6, 4);
    
    return year + "-" + month + "-" + day;
}

std::string PostgresRepository::formatDateFromPostgres(const std::string& date) const {
    if (date.empty() || date.length() != 10) return date;

    std::string year = date.substr(0, 4);
    std::string month = date.substr(5, 2);
    std::string day = date.substr(8, 2);
    
    return day + "." + month + "." + year;
}

bool PostgresRepository::createTablesIfNotExist() {
    std::string createContactsTable = 
        "CREATE TABLE IF NOT EXISTS contacts ("
        "    id SERIAL PRIMARY KEY,"
        "    last_name VARCHAR(100) NOT NULL,"
        "    first_name VARCHAR(100) NOT NULL,"
        "    patronymic VARCHAR(100),"
        "    address TEXT,"
        "    birth_date DATE,"
        "    email VARCHAR(255) NOT NULL"
        ");";
    
    if (!executeQuery(createContactsTable)) {
        return false;
    }
    
    std::string createPhonesTable = 
        "CREATE TABLE IF NOT EXISTS phones ("
        "    id SERIAL PRIMARY KEY,"
        "    contact_id INTEGER NOT NULL REFERENCES contacts(id) ON DELETE CASCADE,"
        "    type VARCHAR(50) NOT NULL,"
        "    number VARCHAR(50) NOT NULL"
        ");";
    
    if (!executeQuery(createPhonesTable)) {
        return false;
    }
    
    std::string createIndex = 
        "CREATE INDEX IF NOT EXISTS idx_phones_contact_id ON phones(contact_id);";
    
    return executeQuery(createIndex);
}

Contact PostgresRepository::rowToContact(PGresult* res, int row) const {
    Contact c;
    c.lastName = PQgetvalue(res, row, PQfnumber(res, "last_name"));
    c.firstName = PQgetvalue(res, row, PQfnumber(res, "first_name"));
    
    int patronymicIdx = PQfnumber(res, "patronymic");
    if (patronymicIdx != -1) {
        c.patronymic = PQgetvalue(res, row, patronymicIdx);
    }
    
    int addressIdx = PQfnumber(res, "address");
    if (addressIdx != -1) {
        c.address = PQgetvalue(res, row, addressIdx);
    }
    
    int birthDateIdx = PQfnumber(res, "birth_date");
    if (birthDateIdx != -1) {
        std::string birthDateStr = PQgetvalue(res, row, birthDateIdx);
        if (!birthDateStr.empty() && birthDateStr != "NULL") {
            c.birthDate = formatDateFromPostgres(birthDateStr);
        }
    }
    
    c.email = PQgetvalue(res, row, PQfnumber(res, "email"));
    
    return c;
}

bool PostgresRepository::saveAll(const std::vector<Contact>& contacts) {
    if (!executeQuery("DELETE FROM phones;") || !executeQuery("DELETE FROM contacts;")) {
        return false;
    }
    
    for (const Contact& c : contacts) {
        if (!addContact(c)) {
            return false;
        }
    }
    
    cache = contacts;
    return true;
}

bool PostgresRepository::loadAll(std::vector<Contact>& contacts) {
    if (!connected) return false;
    
    PGresult* res = executeQueryResult("SELECT * FROM contacts ORDER BY id;");
    if (!res) return false;
    
    int rows = PQntuples(res);
    contacts.clear();
    
    for (int i = 0; i < rows; ++i) {
        Contact c = rowToContact(res, i);

        int contactId = std::stoi(PQgetvalue(res, i, PQfnumber(res, "id")));
        std::string phonesQuery = "SELECT type, number FROM phones WHERE contact_id = " + std::to_string(contactId) + ";";
        
        PGresult* phonesRes = executeQueryResult(phonesQuery);
        if (phonesRes) {
            int phoneRows = PQntuples(phonesRes);
            for (int j = 0; j < phoneRows; ++j) {
                PhoneInfo phone;
                phone.type = PQgetvalue(phonesRes, j, PQfnumber(phonesRes, "type"));
                phone.number = PQgetvalue(phonesRes, j, PQfnumber(phonesRes, "number"));
                c.phones.push_back(phone);
            }
            PQclear(phonesRes);
        }
        
        contacts.push_back(c);
    }
    
    PQclear(res);
    cache = contacts;
    return true;
}

bool PostgresRepository::addContact(const Contact& contact) {
    if (!connected) return false;
    
    std::string escapedLastName = escapeString(contact.lastName);
    std::string escapedFirstName = escapeString(contact.firstName);
    std::string escapedPatronymic = escapeString(contact.patronymic);
    std::string escapedAddress = escapeString(contact.address);
    std::string escapedEmail = escapeString(contact.email);

    std::string birthDateStr = "NULL";
    if (!contact.birthDate.empty()) {
        std::string pgDate = formatDateForPostgres(contact.birthDate);
        if (!pgDate.empty()) {
            birthDateStr = "'" + pgDate + "'";
        }
    }
    
    std::stringstream query;
    query << "INSERT INTO contacts (last_name, first_name, patronymic, address, birth_date, email) "
          << "VALUES (" << escapedLastName << ", " << escapedFirstName << ", " 
          << escapedPatronymic << ", " << escapedAddress << ", " << birthDateStr << ", " 
          << escapedEmail << ") RETURNING id;";
    
    PGresult* res = executeQueryResult(query.str());
    if (!res) return false;
    
    int contactId = std::stoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    
    for (const PhoneInfo& phone : contact.phones) {
        std::string escapedType = escapeString(phone.type);
        std::string escapedNumber = escapeString(phone.number);
        
        std::stringstream phoneQuery;
        phoneQuery << "INSERT INTO phones (contact_id, type, number) VALUES ("
                   << contactId << ", " << escapedType << ", " << escapedNumber << ");";
        
        if (!executeQuery(phoneQuery.str())) {
            return false;
        }
    }
    
    cache.push_back(contact);
    return true;
}

bool PostgresRepository::updateContact(int index, const Contact& contact) {
    if (!connected || index < 0 || index >= static_cast<int>(cache.size())) {
        return false;
    }

    std::stringstream getIdQuery;
    getIdQuery << "SELECT id FROM contacts ORDER BY id LIMIT 1 OFFSET " << index << ";";
    
    PGresult* res = executeQueryResult(getIdQuery.str());
    if (!res) return false;
    
    int contactId = std::stoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    
    std::string escapedLastName = escapeString(contact.lastName);
    std::string escapedFirstName = escapeString(contact.firstName);
    std::string escapedPatronymic = escapeString(contact.patronymic);
    std::string escapedAddress = escapeString(contact.address);
    std::string escapedEmail = escapeString(contact.email);

    std::string birthDateStr = "NULL";
    if (!contact.birthDate.empty()) {
        std::string pgDate = formatDateForPostgres(contact.birthDate);
        if (!pgDate.empty()) {
            birthDateStr = "'" + pgDate + "'";
        }
    }
    
    std::stringstream updateQuery;
    updateQuery << "UPDATE contacts SET "
                << "last_name = " << escapedLastName << ", "
                << "first_name = " << escapedFirstName << ", "
                << "patronymic = " << escapedPatronymic << ", "
                << "address = " << escapedAddress << ", "
                << "birth_date = " << birthDateStr << ", "
                << "email = " << escapedEmail << " "
                << "WHERE id = " << contactId << ";";
    
    if (!executeQuery(updateQuery.str())) {
        return false;
    }
    
    std::string deletePhones = "DELETE FROM phones WHERE contact_id = " + std::to_string(contactId) + ";";
    if (!executeQuery(deletePhones)) {
        return false;
    }
    
    for (const PhoneInfo& phone : contact.phones) {
        std::string escapedType = escapeString(phone.type);
        std::string escapedNumber = escapeString(phone.number);
        
        std::stringstream phoneQuery;
        phoneQuery << "INSERT INTO phones (contact_id, type, number) VALUES ("
                   << contactId << ", " << escapedType << ", " << escapedNumber << ");";
        
        if (!executeQuery(phoneQuery.str())) {
            return false;
        }
    }
    
    if (index < static_cast<int>(cache.size())) {
        cache[index] = contact;
    }
    
    return true;
}

bool PostgresRepository::deleteContact(int index) {
    if (!connected || index < 0 || index >= static_cast<int>(cache.size())) {
        return false;
    }
    
    std::stringstream getIdQuery;
    getIdQuery << "SELECT id FROM contacts ORDER BY id LIMIT 1 OFFSET " << index << ";";
    
    PGresult* res = executeQueryResult(getIdQuery.str());
    if (!res) return false;
    
    int contactId = std::stoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    
    std::string deleteQuery = "DELETE FROM contacts WHERE id = " + std::to_string(contactId) + ";";
    
    if (!executeQuery(deleteQuery)) {
        return false;
    }
    
    if (index < static_cast<int>(cache.size())) {
        cache.erase(cache.begin() + index);
    }
    
    return true;
}

bool PostgresRepository::contactExists(int index) const {
    return index >= 0 && index < static_cast<int>(cache.size());
}

int PostgresRepository::getCount() {
    if (!connected) return 0;
    
    PGresult* res = PQexec(conn, "SELECT COUNT(*) FROM contacts;");
    if (!res || PQresultStatus(res) != PGRES_TUPLES_OK) {
        if (res) PQclear(res);
        return 0;
    }
    
    int count = std::stoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    
    return count;
}

std::string PostgresRepository::getStorageType() const {
    return "PostgreSQL хранилище";
}

std::string PostgresRepository::getLastError() const {
    if (conn) {
        return PQerrorMessage(conn);
    }
    return "Нет подключения";
}