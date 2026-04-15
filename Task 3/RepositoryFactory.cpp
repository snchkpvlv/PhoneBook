#include "RepositoryFactory.h"
#include "FileRepository.h"
#include "PostgresRepository.h"
#include <iostream>

std::unique_ptr<IRepository> RepositoryFactory::create(StorageType type) {
    switch (type) {
        case StorageType::FILE:
            return std::make_unique<FileRepository>("contacts.txt");
        case StorageType::POSTGRESQL:
            return std::make_unique<PostgresRepository>(
                "host=localhost port=5432 dbname=phonebook user=sonchellos"
            );
        default:
            return std::make_unique<FileRepository>("contacts.txt");
    }
}

StorageType RepositoryFactory::showSelectionMenu() {
    std::cout << "\n=== ВЫБОР ТИПА ХРАНИЛИЩА ===\n";
    std::cout << "1. Файловое хранилище (contacts.txt)\n";
    std::cout << "2. PostgreSQL база данных\n";
    std::cout << "Выберите тип хранилища (1-2): ";
    
    int choice;
    std::cin >> choice;
    std::cin.ignore();
    
    if (choice == 2) {
        return StorageType::POSTGRESQL;
    }
    return StorageType::FILE;
}

std::string RepositoryFactory::storageTypeToString(StorageType type) {
    switch (type) {
        case StorageType::FILE: return "файл";
        case StorageType::POSTGRESQL: return "PostgreSQL";
        default: return "неизвестно";
    }
}