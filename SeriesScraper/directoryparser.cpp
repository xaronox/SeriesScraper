#include "directoryparser.h"
#include <QDebug>
#include <QCollator>

QStringList DirectoryParser::sortFiles(QStringList files)
{
    QStringList sortedFiles;
    std::vector<int> position = getEpisodePositions(files);

    for (int i = 0; i < files.length(); i++) {
        while (sortedFiles.length() <= position.at(i)) { // Prepare space
            sortedFiles.push_back("");
        }
        sortedFiles[position.at(i)] = files.at(i);
    }
    return sortedFiles;
}

QFileInfoList DirectoryParser::sortFiles(QFileInfoList files)
{

    QFileInfoList unsortedFiles = files;
    QCollator collator;
    collator.setNumericMode(true);

    std::sort(unsortedFiles.begin(), unsortedFiles.end(),
              [&collator](const QFileInfo &file1, QFileInfo &file2)
    {
        return collator.compare(file1.path(), file2.path()) < 0;
    });

    return unsortedFiles;
}

void DirectoryParser::setNameFilterToAll()
{
    filter.clear();
    filter << "*.avi" << "*.mkv" << "*.mp4" << "*.m4v" << "*.mpg" << "*.flv" << ".*webm" << "*.ogv" << "*.mov" << "*.wmv";
}

DirectoryParser::DirectoryParser()
{
    directory.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    directory.setSorting(QDir::NoSort);
    directory.setPath("");
    setNameFilterToAll();

    episodeNumberExpression.setPattern("s[0-9]+e[0-9]+");
    numberFromEpisodeNumberExpression.setPattern("[0-9]*$");
}

bool DirectoryParser::initializeDirectory(QDir directory)
{
    if (directory.exists()) {
        this->directory = directory;
        return true;
    }
    else {
        return false;
    }
}

bool DirectoryParser::directoryExists()
{
    return directory.exists();
}

QStringList DirectoryParser::getFiles()
{
    directory.setNameFilters(filter);
    QFileInfo fileInfo;
    QFileInfoList list = directory.entryInfoList();
    QStringList filesToReturn;
    for (int i = 0; i < list.size(); ++i) {
        fileInfo = list.at(i);
        if (fileInfo.isFile())
            filesToReturn << fileInfo.fileName();
    }
    filesToReturn = sortFiles(filesToReturn);
    return filesToReturn;
}

QStringList DirectoryParser::getFiles(QString extension)
{
    filter = (QStringList() << extension);
    QStringList fileList = getFiles();
    setNameFilterToAll();
    return fileList;
}

QStringList DirectoryParser::getFilesSuffix()
{
    QStringList suffixes;

    if (directory.exists()) {
        directory.setNameFilters(filter);
        QFileInfoList fileList = directory.entryInfoList();
        fileList = sortFiles(fileList);

        for (int i = 0; i < fileList.length(); i++) {
            if (fileList.at(i).isFile())
                suffixes << fileList.at(i).suffix();
        }
    }
    else {
        suffixes << "";
    }
    return suffixes;
}

std::vector<int> DirectoryParser::getEpisodePositions(QStringList episodeList)
{
    std::vector<int> episodePosition;
    QRegularExpressionMatch match;
    QRegularExpressionMatch matchEpisodeNumber;

    for (int i = 0; i < episodeList.length(); i++) {
        match = episodeNumberExpression.match
                (episodeList.at(i).toLower(), 0, QRegularExpression::PartialPreferCompleteMatch);

        if (match.hasMatch()) {
            QString capturedEpisodeString = match.captured();
            matchEpisodeNumber = numberFromEpisodeNumberExpression.match
                    (capturedEpisodeString, 0, QRegularExpression::PartialPreferCompleteMatch);
            int actualPosition = matchEpisodeNumber.captured().toInt() - 1;
            episodePosition.push_back(actualPosition);
        }
    }
    return episodePosition;
}

QStringList DirectoryParser::getOldFileNameList()
{
    QStringList oldFileNameList;
    if (directory.exists()) {
        oldFileNameList = getFiles();
    }
    else {
        oldFileNameList << "";
    }
    return oldFileNameList;
}

