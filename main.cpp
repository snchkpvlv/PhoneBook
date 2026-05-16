#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    a.setApplicationName("Телефонный справочник");
    MainWindow w;
    w.setWindowTitle("Телефонный справочник");
    w.show();
    
    return a.exec();
}

//qmake PhoneBook.pro
//make
//./PhoneBook