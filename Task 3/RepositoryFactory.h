#ifndef REPOSITORYFACTORY_H
#define REPOSITORYFACTORY_H

#include "IRepository.h"
#include <memory>

enum class StorageType {
    FILE,
    POSTGRESQL
};

class RepositoryFactory {
public:
    static std::unique_ptr<IRepository> create(StorageType type);
    static StorageType showSelectionMenu();
    static std::string storageTypeToString(StorageType type);
};

#endif