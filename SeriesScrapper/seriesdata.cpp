#include "seriesdata.h"

#include <QDebug>

SeriesData::SeriesData() :
    seasonFileNameText(" - s"),
    episodeFileNameText("e"),
    amountEpisodes(0),
    amountSeasons(0),
    selectedSeason(1)
{
    workingDirectory = QDir("");
}

//----------------------- Setter -----------------------//
void SeriesData::setSeriesFileNameText(QString newSeriesText)
{
    seriesFileNameText = newSeriesText;
}

void SeriesData::setSeasonFileNameText(QString newSeasonText)
{
    seasonFileNameText = newSeasonText;
}

void SeriesData::setEpisodeFileNameText(QString newEpisodeText)
{
    episodeFileNameText = newEpisodeText;
}

void SeriesData::setSeries(QString newSeries)
{
    series = newSeries;
}

void SeriesData::setSuffix(int index, QString newSuffix)
{
    if (suffixesList.length() <= index) { // Prepare space if not enough available
        for (int i = suffixesList.length(); i <= index; i++)
            suffixesList << "";
    }
    suffixesList[index] = newSuffix;
}

void SeriesData::setSuffixes(QStringList newSuffixList)
{
    suffixesList = newSuffixList;
}

void SeriesData::setEpisode(int index, QString newEpisode)
{
    if (episodesList.length() > index)
        episodesList[index] = newEpisode;
}

void SeriesData::setEpisodes(QStringList newEpisodeList)
{
    episodesList = newEpisodeList;
    amountEpisodes = episodesList.length();
}

void SeriesData::setAmountSeasons(int newAmount)
{
    amountSeasons = newAmount;
}

void SeriesData::setSelectedSeason(int newSelectedSeason)
{
    if (newSelectedSeason < amountSeasons)
        selectedSeason = newSelectedSeason;
}

void SeriesData::setOldFileNames(QStringList oldFileNamesSortedWithSpaces)
{
    this->oldFileNamesSortedWithSpaces = oldFileNamesSortedWithSpaces;
}

void SeriesData::setWorkingDirectory(QDir directory)
{
    if (directory.exists())
        workingDirectory = directory;
}

//----------------------- Getter -----------------------//
QString SeriesData::getSeriesFileNameText()
{
    return seriesFileNameText;
}

QString SeriesData::getSeasonFileNameText()
{
    return seasonFileNameText;
}

QString SeriesData::getEpisodeFileNameText()
{
    return episodeFileNameText;
}

QString SeriesData::getSeries()
{
    return series;
}

QString SeriesData::getSuffix(int index)
{
    if (suffixesList.length() > index)
        return suffixesList.at(index);
    else
        return QString("No suffix available at index " + QString::number(index));
}

QString SeriesData::getEpisode(int index)
{
    if (episodesList.length() > index)
        return episodesList.at(index);
    else
        return QString("No episode available at index " + QString::number(index));
}

QStringList SeriesData::getEpisodes()
{
    return episodesList;
}

int SeriesData::getAmountEpisodes()
{
    return amountEpisodes;
}

int SeriesData::getAmountSeasons()
{
    return amountSeasons;
}

int SeriesData::getSelectedSeason()
{
    return selectedSeason;
}

QString SeriesData::getFileNameText(int index)
{
    if (index >= episodesList.length() || index >= suffixesList.length() || suffixesList.at(index) == "") // geht schöner
        return "";

    QString episodeName = episodesList.at(index);
    QString suffix = suffixesList.at(index);
    QString fileName = series + seriesFileNameText + seasonFileNameText + QString::number(selectedSeason)
            + episodeFileNameText + QString::number(index + 1) + " - " + episodeName + "." + suffix;
    return fileName;
}

QStringList SeriesData::getNewFileNamesForView()
{
    QStringList newFileNames;
    for (int i = 0; i < amountEpisodes; i++)
        newFileNames << getFileNameText(i);
    return newFileNames;
}

QStringList SeriesData::getOldFileNamesForView()
{
    QStringList oldFileNamesCropped;
    for (int i = 0; i < amountEpisodes; i++) {
        if (i < oldFileNamesSortedWithSpaces.length())
            oldFileNamesCropped << oldFileNamesSortedWithSpaces.at(i);
        else
            oldFileNamesCropped << "";
    }
    return oldFileNamesCropped;
}

QDir SeriesData::getWorkingDirectory()
{
    return workingDirectory;
}