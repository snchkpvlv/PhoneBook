#ifndef IREPOSITORY_H
#define IREPOSITORY_H

#include "Contact.h"
#include <string>
#include <vector>

class IRepository {
public:
    virtual ~IRepository() = default;
    
    virtual bool saveAll(const std::vector<Contact>& contacts) = 0;
    virtual bool loadAll(std::vector<Contact>& contacts) = 0;
    virtual bool addContact(const Contact& contact) = 0;
    virtual bool updateContact(int index, const Contact& contact) = 0;
    virtual bool deleteContact(int index) = 0;
    virtual bool contactExists(int index) const = 0;
    virtual int getCount() = 0; 
    virtual std::string getStorageType() const = 0;
};

#endif