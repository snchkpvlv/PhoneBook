#include "mainwindow.h"
#include "contactvalidator.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QListWidget>
#include <QGroupBox>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , model(new QStandardItemModel(this))
{
    setupUI();
    createActions();
    createMenus();
    updateTable();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Выпадающий список для выбора поля поиска
    searchFieldComboBox = new QComboBox(this);
    searchFieldComboBox->addItem("Все поля");
    searchFieldComboBox->addItem("Фамилия");
    searchFieldComboBox->addItem("Имя");
    searchFieldComboBox->addItem("Отчество");
    searchFieldComboBox->addItem("Адрес");
    searchFieldComboBox->addItem("Дата рождения");
    searchFieldComboBox->addItem("Email");
    searchFieldComboBox->addItem("Телефоны");

    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("Введите текст для поиска...");

    tableView = new QTableView(this);
    model->setHorizontalHeaderLabels({
        "Фамилия", "Имя", "Отчество", "Адрес", "Дата рождения", "Email", "Телефоны"
    });
    tableView->setModel(model);
    tableView->setSortingEnabled(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableView->horizontalHeader()->setStretchLastSection(true);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->addWidget(searchFieldComboBox);
    searchLayout->addWidget(searchBox);
    
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(searchLayout);
    mainLayout->addWidget(tableView);
    centralWidget->setLayout(mainLayout);

    connect(searchBox, SIGNAL(textChanged(const QString &)), this, SLOT(onSearchTextChanged(const QString &)));
    connect(searchFieldComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onSearchFieldChanged(int))); // ДОБАВЛЕНО
}

void MainWindow::onSearchFieldChanged(int index)
{
    // При изменении поля поиска обновляем поиск с текущим текстом
    onSearchTextChanged(searchBox->text());
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    QString filter = text.trimmed().toLower();
    int searchField = searchFieldComboBox->currentIndex(); 
    
    for (int i = 0; i < model->rowCount(); ++i) {
        bool show = false;
        
        if (searchField == 0) {
            // Поиск по всем полям
            for (int j = 0; j < model->columnCount(); ++j) {
                QString cellText = model->item(i, j)->text();
                
                // Для столбца с телефонами извлекаем только цифры
                if (j == 6) {
                    QString phoneDigits;
                    for (const QChar &ch : cellText) {
                        if (ch.isDigit()) {
                            phoneDigits.append(ch);
                        }
                    }
                    if (phoneDigits.contains(filter, Qt::CaseInsensitive)) {
                        show = true;
                        break;
                    }
                } else {
                    if (cellText.toLower().contains(filter)) {
                        show = true;
                        break;
                    }
                }
            }
        } else {
            // Поиск по конкретному полю 
            int columnIndex = searchField - 1;
            if (columnIndex >= 0 && columnIndex < model->columnCount()) {
                QString cellText = model->item(i, columnIndex)->text();
                
                // Особый случай для телефонов
                if (columnIndex == 6) {
                    QString phoneDigits;
                    for (const QChar &ch : cellText) {
                        if (ch.isDigit()) {
                            phoneDigits.append(ch);
                        }
                    }
                    show = phoneDigits.contains(filter, Qt::CaseInsensitive);
                } else {
                    show = cellText.toLower().contains(filter);
                }
            }
        }
        
        tableView->setRowHidden(i, !show);
    }
}

void MainWindow::createActions()
{
    QAction *actAdd = new QAction("Добавить", this);
    QAction *actEdit = new QAction("Редактировать", this);
    QAction *actDelete = new QAction("Удалить", this);
    QAction *actSave = new QAction("Сохранить", this);
    QAction *actLoad = new QAction("Загрузить", this);

    connect(actAdd, SIGNAL(triggered()), this, SLOT(onAddContact()));
    connect(actEdit, SIGNAL(triggered()), this, SLOT(onEditContact()));
    connect(actDelete, SIGNAL(triggered()), this, SLOT(onDeleteContact()));
    connect(actSave, SIGNAL(triggered()), this, SLOT(onSave()));
    connect(actLoad, SIGNAL(triggered()), this, SLOT(onLoad()));

    QToolBar *toolbar = addToolBar("Контакты");
    toolbar->addAction(actAdd);
    toolbar->addAction(actEdit);
    toolbar->addAction(actDelete);
    toolbar->addSeparator();
    toolbar->addAction(actLoad);
    toolbar->addAction(actSave);
}

void MainWindow::createMenus()
{
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu *fileMenu = menuBar->addMenu("Файл");
    fileMenu->addAction("Загрузить", this, &MainWindow::onLoad);
    fileMenu->addAction("Сохранить", this, &MainWindow::onSave);

    QMenu *contactMenu = menuBar->addMenu("Контакт");
    contactMenu->addAction("Добавить", this, &MainWindow::onAddContact);
    contactMenu->addAction("Редактировать", this, &MainWindow::onEditContact);
    contactMenu->addAction("Удалить", this, &MainWindow::onDeleteContact);
}

void MainWindow::updateTable()
{
    model->removeRows(0, model->rowCount());//очистка таблицы
    for (const Contact &c : contacts) {
        QList<QStandardItem*> row;
        for (const QString &s : c.toDisplayList()) {
            QStandardItem *item = new QStandardItem(s);
            if (row.size() == 4) { // Индекс столбца с датой
                item->setData(c.birthDate, Qt::UserRole);
            }
            row << item;
        }
        model->appendRow(row);
    }
}

int MainWindow::getCurrentRow() const
{
    QModelIndexList selected = tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return -1;
    return selected.first().row();
}

QString MainWindow::getPhoneType(QWidget *parent, const QString &currentType)
{
    QStringList types = {"рабочий", "домашний", "служебный", "мобильный"};
    
    bool ok;
    QString type = QInputDialog::getItem(parent, "Тип телефона", 
                                       "Выберите тип телефона:", 
                                       types, 
                                       types.indexOf(currentType) >= 0 ? types.indexOf(currentType) : 0, 
                                       false, &ok);
    
    if (!ok || type.isEmpty()) {
        return "";
    }
    
    return type;
}

bool MainWindow::editPhones(QList<PhoneInfo> &phones)
{
    QDialog dialog(this);
    dialog.setWindowTitle("Управление телефонами");
    dialog.setModal(true);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    
    // Список телефонов, каждый добавляется как элемент списка
    QListWidget *listWidget = new QListWidget(&dialog);
    for (const PhoneInfo &phone : phones) {
        listWidget->addItem(phone.displayString());
    }
    mainLayout->addWidget(listWidget);
    
    // Кнопки управления телефонами
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    QPushButton *btnAdd = new QPushButton("Добавить", &dialog);
    QPushButton *btnEdit = new QPushButton("Редактировать", &dialog);
    QPushButton *btnRemove = new QPushButton("Удалить", &dialog);
    
    buttonLayout->addWidget(btnAdd);
    buttonLayout->addWidget(btnEdit);
    buttonLayout->addWidget(btnRemove);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
    
    // Кнопки OK/Cancel
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    mainLayout->addWidget(buttonBox);
    
    // добавить
    connect(btnAdd, &QPushButton::clicked, [&]() {
        bool ok;
        QString number = QInputDialog::getText(&dialog, "Новый телефон", 
                                             "Введите номер телефона:", 
                                             QLineEdit::Normal, "", &ok);
        if (!ok || number.isEmpty()) return;
        
        QString phoneError = ContactValidator::validatePhoneWithDetails(number);
        if (!phoneError.isEmpty()) {
            QMessageBox::warning(&dialog, "Ошибка", "Некорректный формат телефона!\n" + phoneError);
            return;
        }
        
        QString type = getPhoneType(&dialog);
        if (type.isEmpty()) return;
        
        PhoneInfo newPhone;
        newPhone.type = type;
        newPhone.number = ContactValidator::normalizePhone(number);
        
        phones.append(newPhone);
        listWidget->addItem(newPhone.displayString());
    });
    
    //редактировать
    connect(btnEdit, &QPushButton::clicked, [&]() {
        int row = listWidget->currentRow();
        if (row < 0 || row >= phones.size()) return;
        
        PhoneInfo &phone = phones[row];
    
        bool ok;
        QString newNumber = QInputDialog::getText(&dialog, "Редактирование телефона", 
                                                "Введите новый номер:", 
                                                QLineEdit::Normal, phone.number, &ok);
        if (ok && !newNumber.isEmpty()) {
            QString phoneError = ContactValidator::validatePhoneWithDetails(newNumber);
            if (!phoneError.isEmpty()) {
                QMessageBox::warning(&dialog, "Ошибка", "Некорректный формат телефона!\n" + phoneError);
                return;
            }
            phone.number = ContactValidator::normalizePhone(newNumber);
        }
        
        QString newType = getPhoneType(&dialog, phone.type);
        if (!newType.isEmpty()) {
            phone.type = newType;
        }
        
        listWidget->item(row)->setText(phone.displayString());
    });
    
    //удалить
    connect(btnRemove, &QPushButton::clicked, [&]() {
        int row = listWidget->currentRow();
        if (row >= 0 && row < phones.size()) {
            delete listWidget->takeItem(row);
            phones.removeAt(row);
        }
    });
    
    connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    
    return dialog.exec() == QDialog::Accepted;
}

bool MainWindow::editContact(Contact &contact, bool isNew)
{
    QDialog dialog(this);
    dialog.setWindowTitle(isNew ? "Добавление контакта" : "Редактирование контакта");
    dialog.setModal(true);
    
    QFormLayout *formLayout = new QFormLayout(&dialog);
    
    // Поля ввода
    QLineEdit *lastNameEdit = new QLineEdit(contact.lastName, &dialog);
    QLineEdit *firstNameEdit = new QLineEdit(contact.firstName, &dialog);
    QLineEdit *patronymicEdit = new QLineEdit(contact.patronymic, &dialog);
    QLineEdit *addressEdit = new QLineEdit(contact.address, &dialog);
    
    // Поле даты рождения
    QDateEdit *birthDateEdit = new QDateEdit(&dialog);
    birthDateEdit->setCalendarPopup(true);
    birthDateEdit->setDisplayFormat("dd.MM.yyyy");
    birthDateEdit->setDate(contact.birthDate.isValid() ? contact.birthDate : QDate::currentDate());
    birthDateEdit->setMaximumDate(QDate::currentDate().addDays(-1));
    
    QLineEdit *emailEdit = new QLineEdit(contact.email, &dialog);
    
    // Кнопка для управления телефонами
    QPushButton *phonesButton = new QPushButton("Управление телефонами", &dialog);
    phonesButton->setStyleSheet("QPushButton { padding: 5px; }");
    
    formLayout->addRow("Фамилия:", lastNameEdit);
    formLayout->addRow("Имя:", firstNameEdit);
    formLayout->addRow("Отчество:", patronymicEdit);
    formLayout->addRow("Адрес:", addressEdit);
    formLayout->addRow("Дата рождения:", birthDateEdit);
    formLayout->addRow("Email:", emailEdit);
    formLayout->addRow("Телефоны:", phonesButton);
    
    // Кнопки OK/Cancel
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    formLayout->addRow(buttonBox);
    
    // Хранение копии телефонов для диалога
    QList<PhoneInfo> tempPhones = contact.phones;
    
    connect(phonesButton, &QPushButton::clicked, [&]() {
        editPhones(tempPhones);
    });
    
    // Убираем стандартные обработчики кнопок
    buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
    
    bool dialogAccepted = false;
    
    // Подключаем свою логику для кнопки OK
    connect(buttonBox, &QDialogButtonBox::accepted, [&]() {
        // Валидация данных
        bool hasErrors = false;
        
        QString lastName = ContactValidator::normalizeName(lastNameEdit->text());
        if (!ContactValidator::isValidName(lastName)) {
            QMessageBox::warning(&dialog, "Ошибка", "Некорректная фамилия!\n"
                                                   "- Должна начинаться с заглавной буквы\n"
                                                   "- Может содержать только буквы, дефис и пробел\n"
                                                   "- После первой буквы должны быть строчные\n"
                                                   "- Не может начинаться или заканчиваться на дефис");
            lastNameEdit->setFocus();
            lastNameEdit->selectAll();
            hasErrors = true;
        } else {
            contact.lastName = lastName;
        }
        
        QString firstName = ContactValidator::normalizeName(firstNameEdit->text());
        if (!ContactValidator::isValidName(firstName)) {
            if (!hasErrors) {
                QMessageBox::warning(&dialog, "Ошибка", "Некорректное имя!\n"
                                                       "- Должно начинаться с заглавной буквы\n"
                                                       "- Может содержать только буквы, дефис и пробел\n"
                                                       "- После первой буквы должны быть строчные\n"
                                                       "- Не может начинаться или заканчиваться на дефис");
                firstNameEdit->setFocus();
                firstNameEdit->selectAll();
            }
            hasErrors = true;
        } else {
            contact.firstName = firstName;
        }
        
        QString patronymic = ContactValidator::normalizeName(patronymicEdit->text());
        if (!patronymic.isEmpty() && !ContactValidator::isValidName(patronymic)) {
            if (!hasErrors) {
                QMessageBox::warning(&dialog, "Ошибка", "Некорректное отчество!\n"
                                                       "- Должно начинаться с заглавной буквы\n"
                                                       "- Может содержать только буквы, дефис и пробел\n"
                                                       "- После первой буквы должны быть строчные\n"
                                                       "- Не может начинаться или заканчиваться на дефис");
                patronymicEdit->setFocus();
                patronymicEdit->selectAll();
            }
            hasErrors = true;
        } else {
            contact.patronymic = patronymic;
        }
        
        contact.address = addressEdit->text().trimmed();
        
        QDate birthDate = birthDateEdit->date();
        if (!ContactValidator::isValidBirthDate(birthDate)) {
            if (!hasErrors) {
                QMessageBox::warning(&dialog, "Ошибка", "Некорректная дата рождения!\n"
                                                       "- Дата должна быть в прошлом\n"
                                                       "- Корректный день для выбранного месяца");
                birthDateEdit->setFocus();
            }
            hasErrors = true;
        } else {
            contact.birthDate = birthDate;
        }
        
        QString email = emailEdit->text().trimmed();
        if (!email.isEmpty() && !ContactValidator::isValidEmail(email)) {
            if (!hasErrors) {
                QMessageBox::warning(&dialog, "Ошибка", "Некорректный email!\n"
                                                       "- Формат: user@domain.com\n"
                                                       "- Только латинские буквы, цифры, точки, дефисы и подчеркивания");
                emailEdit->setFocus();
                emailEdit->selectAll();
            }
            hasErrors = true;
        } else {
            contact.email = email;
        }
        
        // Проверка, что есть хотя бы один телефон
        if (tempPhones.isEmpty()) {
            if (!hasErrors) {
                QMessageBox::warning(&dialog, "Ошибка", "Должен быть указан хотя бы один телефон!");
                phonesButton->setFocus();
            }
            hasErrors = true;
        } else {
            contact.phones = tempPhones;
        }
        
        // Если ошибок нет - принимаем диалог
        if (!hasErrors) {
            dialogAccepted = true;
            dialog.accept();
        }
        // Если есть ошибки - диалог остается открытым, пользователь может исправить
    });
    
    connect(buttonBox, &QDialogButtonBox::rejected, [&]() {
        dialog.reject();
    });
    
    // Показываем диалог
    dialog.exec();
    
    return dialogAccepted;
}

void MainWindow::onAddContact()
{
    Contact c;
    if (editContact(c, true)) {
        contacts.append(c);
        updateTable();
    }
}

void MainWindow::onEditContact()
{
    int row = getCurrentRow();
    if (row == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите контакт для редактирования");
        return;
    }
    if (editContact(contacts[row], false)) {
        updateTable();
    }
}

void MainWindow::onDeleteContact()
{
    int row = getCurrentRow();
    if (row == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите контакт для удаления");
        return;
    }
    
    int answer = QMessageBox::question(this, "Удаление контакта", 
                                      "Вы действительно хотите удалить выбранный контакт?",
                                      QMessageBox::Yes | QMessageBox::No);
    
    if (answer == QMessageBox::Yes) {
        contacts.removeAt(row);
        updateTable();
    }
}

void MainWindow::onSave()
{
    QString filename = QFileDialog::getSaveFileName(this, "Сохранить справочник", "", "JSON Files (*.json)");
    if (!filename.isEmpty()) {
        saveToFile(filename);
    }
}

void MainWindow::onLoad()
{
    QString filename = QFileDialog::getOpenFileName(this, "Загрузить справочник", "", "JSON Files (*.json)");
    if (!filename.isEmpty()) {
        loadFromFile(filename);
        updateTable();
    }
}

void MainWindow::saveToFile(const QString &filename)
{
    QJsonArray array;
    for (const Contact &c : contacts) {
        QJsonObject obj;
        obj["lastName"] = c.lastName;
        obj["firstName"] = c.firstName;
        obj["patronymic"] = c.patronymic;
        obj["address"] = c.address;
        obj["birthDate"] = c.birthDate.toString(Qt::ISODate);
        obj["email"] = c.email;
        
        QJsonArray phonesArray;
        for (const PhoneInfo &p : c.phones) {
            QJsonObject phoneObj;
            phoneObj["type"] = p.type;
            phoneObj["number"] = p.number;
            phonesArray.append(phoneObj);
        }
        obj["phones"] = phonesArray;
        
        array.append(obj);
    }

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
        file.close();
        QMessageBox::information(this, "Сохранение", "Данные успешно сохранены!");
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл!");
    }
}

void MainWindow::loadFromFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл!");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        QMessageBox::warning(this, "Ошибка", "Некорректный формат файла!");
        return;
    }

    contacts.clear();
    for (const QJsonValue &v : doc.array()) {
        QJsonObject obj = v.toObject();
        Contact c;
        c.lastName = obj["lastName"].toString();
        c.firstName = obj["firstName"].toString();
        c.patronymic = obj["patronymic"].toString();
        c.address = obj["address"].toString();
        c.birthDate = QDate::fromString(obj["birthDate"].toString(), Qt::ISODate);
        c.email = obj["email"].toString();
        
        for (const QJsonValue &p : obj["phones"].toArray()) {
            QJsonObject phoneObj = p.toObject();
            PhoneInfo phone;
            phone.type = phoneObj["type"].toString();
            phone.number = phoneObj["number"].toString();
            c.phones.append(phone);
        }
        contacts.append(c);
    }
    
    QString message = "Загружено " + QString::number(contacts.size()) + " контактов";
    QMessageBox::information(this, "Загрузка", message);
}