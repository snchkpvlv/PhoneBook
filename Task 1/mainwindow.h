#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QLineEdit>
#include <QMenuBar>
#include <QStandardItemModel>
#include <QList>
#include <QComboBox>
#include "contact.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddContact();
    void onEditContact();
    void onDeleteContact();
    void onSave();
    void onLoad();
    void onSearchTextChanged(const QString &text);
    void onSearchFieldChanged(int index); 

private:
    QTableView *tableView;
    QLineEdit *searchBox;
    QComboBox *searchFieldComboBox;
    QStandardItemModel *model;
    QList<Contact> contacts;

    void createActions();
    void createMenus();
    void setupUI();
    void updateTable();
    int getCurrentRow() const;
    void saveToFile(const QString &filename);
    void loadFromFile(const QString &filename);
    bool editContact(Contact &contact, bool isNew = true);
    
    bool editPhones(QList<PhoneInfo> &phones);
    QString getPhoneType(QWidget *parent, const QString &currentType = "");
};

#endif 