#include "languagecontrol.h"
#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QIODevice>

LanguageControl::LanguageControl()
{
}

bool LanguageControl::initialize()
{
    bool languageFilesExist = false;
    languageFileDirectory.setPath(QCoreApplication::applicationDirPath());
    QStringList fileType("*.json");

    if (languageFileDirectory.cd("language"))
    {
        languageFileList = languageFileDirectory.entryInfoList(fileType, QDir::Files);
        if (!(languageFileList.isEmpty()))
            languageFilesExist = true;
    }
    return languageFilesExist;
}

QStringList LanguageControl::getLanguageList()
{
    QStringList languageListWithoutExtensions;
    for (int i = 0; i < languageFileList.size(); i++)
        languageListWithoutExtensions << languageFileList.at(i).baseName();

    return languageListWithoutExtensions;
}

bool LanguageControl::loadLanguage(QString language)
{
    QString filePath =  languageFileDirectory.absoluteFilePath(language.append(".json"));
    bool loadingSuccessful = loadJsonObject(filePath);

    if(loadingSuccessful)
    {
        QStringList translationList;
        translationList << loadedObject.value("OldNames").toString()
                        << loadedObject.value("NewNames").toString()
                        << loadedObject.value("DirectorySelection").toString()
                        << loadedObject.value("Selection").toString()
                        << loadedObject.value("Path").toString()
                        << loadedObject.value("SeriesSelection").toString()
                        << loadedObject.value("Series").toString()
                        << loadedObject.value("Season").toString()
                        << loadedObject.value("NameScheme").toString()
                        << loadedObject.value("Rename").toString()
                        << loadedObject.value("About").toString()
                        << loadedObject.value("Settings").toString()
                        << loadedObject.value("Language").toString()
                        << loadedObject.value("Display").toString()
                        << loadedObject.value("Help").toString()
                        << loadedObject.value("Close").toString()
                        << loadedObject.value("Done").toString()
                        << loadedObject.value("General").toString()
                        << loadedObject.value("Interface").toString()
                        << loadedObject.value("ShowSeriesInformation").toString()
                        << loadedObject.value("SaveOnExit").toString()
                        << loadedObject.value("SeriesDatabase").toString()
                        << loadedObject.value("Recommended").toString()
                        << loadedObject.value("NewFormat").toString()
                        << loadedObject.value("Add").toString()
                        << loadedObject.value("Remove").toString()
                        << loadedObject.value("Video").toString()
                        << loadedObject.value("NewNameScheme").toString()
                        << loadedObject.value("Reset").toString()
                        << loadedObject.value("ResetAll").toString()
                        << loadedObject.value("NotFound").toString()
                        << loadedObject.value("SeriesLanguageNotFound").toString()
                        << loadedObject.value("NameSchemeNotFound").toString()
                        << loadedObject.value("RenameFailed").toString()
                        << loadedObject.value("RenameSuccess").toString();

        languageData.setTranslationSet(translationList);
        languageData.setLanguage(language);
    }
    return loadingSuccessful;
}

QString LanguageControl::getTranslation(int translateThis)
{
    if (translateThis < languageData.getTranslationList().size())
        return languageData.getTranslation(translateThis);
    else
        return QString("");
}

QStringList LanguageControl::getTranslationList()
{
    return languageData.getTranslationList();
}

