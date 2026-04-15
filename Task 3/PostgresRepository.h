#ifndef POSTGRESREPOSITORY_H
#define POSTGRESREPOSITORY_H

#include "IRepository.h"
#include <string>
#include <vector>
#include <libpq-fe.h>

class PostgresRepository : public IRepository {
private:
    PGconn* conn;
    std::string connectionString;
    bool connected;
    std::vector<Contact> cache;
    
    bool executeQuery(const std::string& query);
    PGresult* executeQueryResult(const std::string& query);
    bool createTablesIfNotExist();
    Contact rowToContact(PGresult* res, int row) const;
    std::string escapeString(const std::string& str) const;
    std::string formatDateForPostgres(const std::string& date) const;
    std::string formatDateFromPostgres(const std::string& date) const;

public:
    explicit PostgresRepository(const std::string& connStr = "host=localhost port=5432 dbname=phonebook user=sonchellos");
    ~PostgresRepository() override;
    
    bool saveAll(const std::vector<Contact>& contacts) override;
    bool loadAll(std::vector<Contact>& contacts) override;
    bool addContact(const Contact& contact) override;
    bool updateContact(int index, const Contact& contact) override;
    bool deleteContact(int index) override;
    bool contactExists(int index) const override;
    int getCount() override;
    std::string getStorageType() const override;
    
    bool isConnected() const { return connected; }
    std::string getLastError() const;
};

#endif