#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QString>
#include "directoryparser.h"
#include "seriesparser.h"
#include "filerenamer.h"
#include "seriesdata.h"
#include "nameschemehandler.h"
#include "message.h"
#include "languagecontrol.h"
#include "serieslanguage.h"
#include "settings.h"

class Controller : public QObject
{
    Q_OBJECT
private:
    SeriesParser seriesParser;
    DirectoryParser directoryParser;
    SeriesData seriesData;
    FileRenamer fileRenamer;
    NameSchemeHandler nameSchemeHandler;
    LanguageControl languageControl;
    SeriesLanguage seriesLanguage;
    Settings settings;

    void initializeNameSchemes();
    void initializeSeriesLanguages();
    void initializeGUILanguages();
    void initializeSettings();
    void updateNewFileNames();
    void setStatusMessage(QString status);
    bool loadSeries(int season, QString language);

public:
    explicit Controller(QObject *parent = 0);
    void initialize();
    bool setSeries(QString series, int season); // Set series if existing and set table
    bool changeSeason(int season);
    bool changeLanguage(QString language);
    bool changeGuiLanguage(QString language);
    void changeSeriesParser(int seriesParser);
    bool setDirectory(QDir directory);
    bool renameFiles();
    void updateView();
    void updateRenameButton();

public slots:
    void notify(Message &msg);

signals:
    void sendMessage(Message &msg);
};

#endif // CONTROLLER_H
