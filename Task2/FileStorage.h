#ifndef FILESTORAGE_H
#define FILESTORAGE_H

#include "Contact.h"
#include <string>
#include <vector>

class FileStorage {
public:
    static bool saveToFile(const std::vector<Contact>& contacts, const std::string& filename);
    static bool loadFromFile(std::vector<Contact>& contacts, const std::string& filename);
    static bool fileExists(const std::string& filename);
};

#endif