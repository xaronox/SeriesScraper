#include "directoryparser.h"
#include <QCoreApplication>
#include <QCollator>
#include <QDebug>
#include "mainwindow.h" // Path structure box number

int DirectoryParser::getNameSchemeType(QString filename)
{
    bool isSeasonAndEpisode = seasonAndEpisodeExpression.match(filename, 0, QRegularExpression::PartialPreferCompleteMatch).hasMatch();
    if (isSeasonAndEpisode)
        return seasonAndEpisode;


    bool isSeasonXOrDotEpisode = seasonXorDotEpisodeExpression.match(filename, 0, QRegularExpression::PartialPreferCompleteMatch).hasMatch();
    if (isSeasonXOrDotEpisode)
        return seasonXorDotEpisode;

    bool isDigitOnly = digitOnlySeasonAndEpisodeExpression.match(filename, 0, QRegularExpression::PartialPreferCompleteMatch).hasMatch();
    if (isDigitOnly)
        return digitOnly;

    return none;
}

QFileInfoList DirectoryParser::sortFiles(QFileInfoList files)
{
    QFileInfoList sortedFiles;
    QStringList fileList;

    for (int i = 0; i < files.size(); i++)  // Get strings to calculate positions
        fileList << files.at(i).fileName();

    std::vector<int> position = getEpisodePositions(fileList);

    if (int(position.size()) < files.size()) // Name scheme not found, natural sort
    {
        sortedFiles = naturalSort(files);
    }
    else // Sort by found positions
    {
        for (int i = 0; i < files.size(); i++)
        {
            qDebug() << getNameSchemeType(fileList.at(i));
            while (sortedFiles.size() <= position.at(i))  // Prepare space
                sortedFiles.push_back(QFileInfo(""));
            if (position.at(i) >= 0)
                sortedFiles[position.at(i)] = files.at(i);
        }
    }
    return sortedFiles;
}

QFileInfoList DirectoryParser::naturalSort(QFileInfoList files)
{
    // Use strings to use collators natural sort function
    QStringList fileList;
    for (int i = 0; i < files.size(); i ++)
        fileList << files.at(i).absoluteFilePath();

    QCollator collator;
    collator.setNumericMode(true);
    std::sort(fileList.begin(), fileList.end(),
              [&collator](const QString &file1, const QString &file2)
    {
        return collator.compare(file1, file2) < 0;
    });

    // Convert strings back to fileInfo
    QFileInfoList fileInfos;
    for (int i = 0; i < fileList.size(); i ++)
        fileInfos << QFileInfo(fileList.at(i));
    return fileInfos;
}

QStringList DirectoryParser::naturalSort(QStringList toSort)
{
    QStringList sorted = toSort;
    QCollator collator;
    collator.setNumericMode(true);
    std::sort(sorted.begin(), sorted.end(),
              [&collator](const QString &string1, const QString &string2)
    {
        return collator.compare(string1, string2) < 0;
    });
    return sorted;
}

std::vector<int> DirectoryParser::getEpisodePositions(QStringList episodeList)
{
    std::vector<int> episodePosition;
    QRegularExpressionMatch seasonAndEpisodeMatch;
    QRegularExpressionMatch episodeNumberMatch;
    QRegularExpressionMatch seasonNumberMatch;

    // Set season
    if (episodeList.size() > 0)
    {
        seasonAndEpisodeMatch = seasonAndEpisodeExpression.match
                (episodeList.at(0).toLower(), 0, QRegularExpression::PartialPreferCompleteMatch);

        if (seasonAndEpisodeMatch.hasMatch())
        {
            QString seasonAndEpisodeText = seasonAndEpisodeMatch.captured();

            seasonNumberMatch = seasonNumberExpression.match(seasonAndEpisodeText, 0);
            QString seasonNumberText = seasonNumberMatch.captured();
            seasonNumberMatch = numberExpression.match(seasonNumberText, 0);
            foundSeason = seasonNumberMatch.captured().toInt();
        }
    }

    // Get episode positions
    for (int i = 0; i < episodeList.size(); i++)
    {
        seasonAndEpisodeMatch = seasonAndEpisodeExpression.match
                (episodeList.at(i).toLower(), 0, QRegularExpression::PartialPreferCompleteMatch);

        if (seasonAndEpisodeMatch.hasMatch())
        {
            QString seasonAndEpisodeText = seasonAndEpisodeMatch.captured();

            episodeNumberMatch = episodeNumberExpression.match(seasonAndEpisodeText, 0);
            QString episodeNumberText = episodeNumberMatch.captured();
            episodeNumberMatch = numberExpression.match(episodeNumberText, 0);
            int foundEpisodePosition = episodeNumberMatch.captured().toInt() - 1;
            episodePosition.push_back(foundEpisodePosition);
        }
    }

    // Return empty vector, if multiple indexes with the same position exist
    std::vector<int> sortedEpisodePositions = episodePosition;
    std::sort(sortedEpisodePositions.begin(), sortedEpisodePositions.end());
    for (int i = 1; i < int(sortedEpisodePositions.size()); i++)
    {
        if (sortedEpisodePositions.at(i - 1) == sortedEpisodePositions.at(i))
            return std::vector<int>();
    }
    return episodePosition;
}

int DirectoryParser::getDirectoryPositionInList(QStringList directoryList, QString directoryToFind)
{
    for (int i = 0; i < directoryList.size(); i++)
    {
        if (directoryList.at(i) == directoryToFind) {
            return i;
        }
    }
    return -1; // Not found
}

void DirectoryParser::setPathStructure(int depth)
{
    std::vector<QStringList> pathStructure;
    if (directorySet)
    {
        QStringList currentDirectoryPositionList;
        QDir workingDirectory = directory;
        workingDirectory.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

        QDir parentDir = workingDirectory;
        parentDir.cdUp();

        if (workingDirectory.isRoot())
        {
            QStringList rootDriveList;
            QFileInfoList rootDrives = QDir::drives();
            for (int i = 0; i < rootDrives.size(); i++)
            {
                rootDriveList << rootDrives.at(i).absolutePath();
            }
            rootDriveList = naturalSort(rootDriveList);
            currentDirectoryPositionList << QString::number(getDirectoryPositionInList(rootDriveList, workingDirectory.absolutePath()));
            pathStructure.push_back(workingDirectory.entryList());
            pathStructure.push_back(rootDriveList);
            pathStructure.push_back(currentDirectoryPositionList);
            containsRoot = true;
        }
        else
        {
            // Last directory content where nothing is selected
            pathStructure.push_back(naturalSort(workingDirectory.entryList()));

            for (int i = 0; i < depth; i++)
            {
                QStringList directoryListToSearch;
                QString directoryToFind;

                directoryToFind = workingDirectory.dirName();
                directoryListToSearch = naturalSort(parentDir.entryList());
                int newDirectoryPosition = getDirectoryPositionInList(directoryListToSearch, directoryToFind);

                if (newDirectoryPosition == -1) // Workaround for not seeing Volumes on network drives on macOS
                {
                    directoryListToSearch << workingDirectory.dirName();
                    directoryListToSearch = naturalSort(directoryListToSearch);
                    newDirectoryPosition = getDirectoryPositionInList(directoryListToSearch, directoryToFind);
                }

                currentDirectoryPositionList <<  QString::number(newDirectoryPosition);
                pathStructure.push_back(directoryListToSearch);

                if (parentDir.isRoot() && (depth != i + 1))
                {
                    QStringList rootDriveList;
                    QFileInfoList rootDrives = QDir::drives();
                    for (int i = 0; i < rootDrives.size(); i++)
                    {
                        rootDriveList << rootDrives.at(i).absolutePath();
                    }
                    rootDriveList = naturalSort(rootDriveList);
                    currentDirectoryPositionList << QString::number(getDirectoryPositionInList(rootDriveList, parentDir.absolutePath()));
                    pathStructure.push_back(rootDriveList);
                    containsRoot = true;
                    break;
                }
                else
                {
                    workingDirectory.cdUp();
                    parentDir.cdUp();
                    containsRoot = false;
                }
            }
            pathStructure.push_back(currentDirectoryPositionList);
        }
    }
    this->pathStructure = pathStructure;
}

void DirectoryParser::setFileInformation()
{
    QFileInfoList fileInfoList = directory.entryInfoList(filter);
    QFileInfoList sortedFileInfoList = sortFiles(fileInfoList);

    QStringList newSortedFiles;
    QStringList newSortedFilesWithoutSuffix;
    QStringList newSuffixes;

    QFileInfo fileInfo;
    for (int i = 0; i < sortedFileInfoList.size(); ++i)
    {
        fileInfo = sortedFileInfoList.at(i);
        if (fileInfo.isFile())
        {
            newSortedFiles << fileInfo.fileName();
            newSortedFilesWithoutSuffix << fileInfo.completeBaseName();
            newSuffixes << fileInfo.suffix();
        } else if (!fileInfo.isDir())
        {
            newSortedFiles << "";
            newSortedFilesWithoutSuffix << "";
            newSuffixes << "";
        }
    }
    setPathStructure(NUMBER_PATH_STRUCTURE_COMBOBOXES);

    sortedFiles = newSortedFiles;
    sortedFileWithoutSuffix = newSortedFilesWithoutSuffix;
    suffixes = newSuffixes;
}

DirectoryParser::DirectoryParser()
{
    directory.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    directory.setSorting(QDir::NoSort);
}

void DirectoryParser::setFileTypes(QStringList fileTypes)
{
    for (int i = 0; i < fileTypes.size(); i++)
        fileTypes[i] = "*." + fileTypes.at(i);
    filter = fileTypes;
    directory.setNameFilters(filter);
}

bool DirectoryParser::initializeDirectory(QString path)
{
    QDir newDirectory(path);
    directorySet = newDirectory.exists();
    foundSeason = 0;
    if (directorySet)
    {
        directory = newDirectory;
        setFileInformation();
    }
    directoryPathInput = path;
    return directorySet;
}

std::vector<QStringList> DirectoryParser::getPathStructure()
{
    return pathStructure;
}

QString DirectoryParser::getDirectoryViaStructure(int level, int selection)
{
    if (level >= int(pathStructure.size() - 1))
        return "";

    QDir newDirectory = directory;
    QString directoryName = pathStructure.at(level).at(selection);

    for (int i = 0; i < level; i++)
    {
        newDirectory.cdUp();
    }
    newDirectory.cd(directoryName);
    return newDirectory.absolutePath();
}

bool DirectoryParser::getStructureContainsRoot()
{
    return containsRoot;
}

QString DirectoryParser::getDirectoryPathInput()
{
    return directoryPathInput;
}

QDir DirectoryParser::getDirectory()
{
    return directory;
}

int DirectoryParser::getFoundSeason()
{
    return foundSeason;
}

QStringList DirectoryParser::getFiles()
{
    return sortedFiles;
}

QStringList DirectoryParser::getFilesWithoutSuffix()
{
    return sortedFileWithoutSuffix;
}

QStringList DirectoryParser::getFilesSuffix()
{
    return suffixes;
}
