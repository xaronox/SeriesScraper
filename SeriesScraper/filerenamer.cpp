#include "filerenamer.h"
#include <QDebug>

FileRenamer::FileRenamer()
{

}

void FileRenamer::setOldFileNames(QStringList oldFileNames)
{
    this->oldFileNameList = oldFileNames;
}

void FileRenamer::setNewFileNames(QStringList newFileNames)
{
    this->newFileNameList = newFileNames;
}

void FileRenamer::setDirectory(QDir directory)
{
    workingDirectory = directory;
}

void FileRenamer::setSuffixes(QStringList suffixList)
{
    suffixes = suffixList;
}

bool FileRenamer::rename()
{
    int amountOldFiles = oldFileNameList.size();
    int amountNewNames = newFileNameList.size();
    int amountSuffixes = suffixes.size();

    qDebug() << amountNewNames << amountOldFiles << amountSuffixes;
    if (amountOldFiles != amountSuffixes)
        return false;

    bool renamingSucceded = false;

    if (workingDirectory.exists())
    {
        renamingSucceded = true;
        int amountToRename = std::min(amountNewNames, amountOldFiles);
        for (int i = 0; i < amountToRename; i++)
        {
            QString fileToRename = oldFileNameList.at(i);

            if (fileToRename.isEmpty()) // Do not rename empty files
                continue;

            QString newFileName = newFileNameList.at(i) + "." + suffixes.at(i);
            bool renameSuccess = workingDirectory.rename(fileToRename, newFileName);
            if (!renameSuccess)
                renamingSucceded = renameSuccess;
        }

    }
    return renamingSucceded;
}
