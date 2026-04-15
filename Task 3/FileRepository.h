#ifndef FILEREPOSITORY_H
#define FILEREPOSITORY_H

#include "IRepository.h"
#include <string>

class FileRepository : public IRepository {
private:
    std::string filename;
    std::vector<Contact> cache;
    bool loaded;

public:
    explicit FileRepository(const std::string& filepath = "contacts.txt");
    ~FileRepository() override = default;
    
    bool saveAll(const std::vector<Contact>& contacts) override;
    bool loadAll(std::vector<Contact>& contacts) override;
    bool addContact(const Contact& contact) override;
    bool updateContact(int index, const Contact& contact) override;
    bool deleteContact(int index) override;
    bool contactExists(int index) const override;
    int getCount() override;
    std::string getStorageType() const override;
    
    bool fileExists() const;
    static bool saveToFile(const std::vector<Contact>& contacts, const std::string& filename);
    static bool loadFromFile(std::vector<Contact>& contacts, const std::string& filename);
};

#endif