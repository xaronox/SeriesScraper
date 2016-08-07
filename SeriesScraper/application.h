#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

class MainWindow;
class LanguageControl;
class Controller;

class Application
{
public:
    Application(int &argc, char *argv[]);
    ~Application();

    void init(); // Initialize the application
    int exec(); // Execute the application

private:
    QApplication app; // Dont derive from QApplication
    MainWindow* view; // Create view after QApplication
    LanguageControl* languageControl;
    Controller* controller;

public slots:

};

#endif // APPLICATION_H
