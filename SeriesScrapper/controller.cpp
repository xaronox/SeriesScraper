#include "controller.h"
#include <QDebug>
#include <vector>

Controller::Controller(QObject *parent) : QObject(parent)
{
}

bool Controller::setSeries(QString series, int season)
{
    qDebug() << "Lädt";

    if (seriesParser.getSeriesSeason("http://www.omdbapi.com/?", series, season, "Title")) {
        qDebug() << "Laden erfolgreich";

        QStringList episodeList = seriesParser.getIDValue();
        int amountSeasons = seriesParser.getAmountSeasons();
        seriesData.setSeries(series);
        seriesData.setSelectedSeason(season);
        seriesData.setEpisodes(episodeList);
        seriesData.setAmountSeasons(amountSeasons);
        updateView();

        return true;
    }
    qDebug() << "Laden fehlgeschlagen";
    return false;
}

bool Controller::setDirectory(QDir directory)
{
    if (directoryParser.initializeDirectory(directory)) {
        seriesData.setWorkingDirectory(directory);

        QStringList suffixesList = directoryParser.getFilesSuffix();
        seriesData.setSuffixes(suffixesList);
        QStringList oldFileNames = directoryParser.getOldFileNameList();
        seriesData.setOldFileNames(oldFileNames);
        updateView();

        return true;
    }
    else
        return false;
}

bool Controller::renameFiles()
{
    QDir directory = seriesData.getWorkingDirectory();
    QStringList episodeNames = seriesData.getNewFileNamesForView();
    QStringList fileNames = seriesData.getOldFileNamesForView();
    fileRenamer.setDirectory(directory);
    fileRenamer.setEpisodeNames(episodeNames);
    fileRenamer.setFiles(fileNames);

    if (fileRenamer.rename())
    {
        QStringList renamedFiles = directoryParser.getOldFileNameList();
        seriesData.setOldFileNames(renamedFiles);
        updateView();
        return true;
        // Output!
    } else
    {
        false;
        // Output!
    }
}


void Controller::updateView()
{
    QStringList episodeNameList = seriesData.getEpisodes();
    QStringList newFileNameList = seriesData.getNewFileNamesForView();
    QStringList oldFileNameList = seriesData.getOldFileNamesForView();
    int amountSeasons = seriesData.getAmountSeasons();

    Message msgViewUpdate;
    msgViewUpdate.type = Message::controller_updateView_view;
    msgViewUpdate.data[0].i = amountSeasons;
    msgViewUpdate.data[1].qsListPointer = &episodeNameList;
    msgViewUpdate.data[2].qsListPointer = &newFileNameList;
    msgViewUpdate.data[3].qsListPointer = &oldFileNameList;
    emit(sendMessage(msgViewUpdate));
}

void Controller::notify(Message &msg)
{
    switch (msg.type) {
    case Message::view_seriesText_changed_controller:
    {
        QString seriesText = *msg.data[0].qsPointer;
        int season = msg.data[1].i;
        bool seriesSet = setSeries(seriesText, season);
        bool isEmpty = seriesText.isEmpty();

        // Emit wether series was scraped succesfully
        Message msgSeriesSet;
        msgSeriesSet.type = Message::controller_seriesSet_view;
        msgSeriesSet.data[0].b = seriesSet;
        msgSeriesSet.data[1].b = isEmpty;
        emit(sendMessage(msgSeriesSet));
        break;
    }
    case Message::view_season_changed_controller:
    {
        int selectedSeason = msg.data[0].i;
        QString series = seriesData.getSeries();
        setSeries(series, selectedSeason);
        break;
    }
    case Message::view_directory_changed_controller:
    {
        QString directory = *msg.data[0].qsPointer;
        bool directorySet = setDirectory(QDir(directory));
        updateView();
    }
    case Message::view_rename_controller:
        renameFiles(); // Bool, operate on Output!
    default:
        break;
    }
}