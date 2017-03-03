#include "directoryhandler.h"

bool DirectoryHandler::renameFiles(bool isUndo)
{
    bool renameSuccess = fileRenamer.isRenamePossible();
    if (renameSuccess)
    {
        int amountToRename = fileRenamer.getRenameAmount();
        for (int i = 0; i < amountToRename; i++)
        {
            QString oldFileName = fileRenamer.getOldFileName(i);
            QString newFileName = fileRenamer.getNewFileName(i);
            emit updateRenameProgress(amountToRename, i+1, oldFileName, newFileName);

            renameSuccess = fileRenamer.renameFile(i);
            if (!renameSuccess)
                break;
        }
        if (renameSuccess && !isUndo)
            fileRenamer.addToUndoQueue();
    }
    return renameSuccess;
}

DirectoryHandler::DirectoryHandler(QObject *parent) : QObject(parent)
{

}

void DirectoryHandler::setFileTypes(QStringList fileTypes)
{
    directoryParser.setFileTypes(fileTypes);
}

std::vector<QStringList> DirectoryHandler::getPathStructure()
{
    return directoryParser.getPathStructure();
}

QString DirectoryHandler::getDirectoryViaStructure(int level, int selection)
{
    return directoryParser.getDirectoryViaStructure(level, selection);
}

bool DirectoryHandler::getStructureContainsRoot()
{
    return directoryParser.getStructureContainsRoot();
}

QString DirectoryHandler::getDirectoryPathInput()
{
    return directoryParser.getDirectoryPathInput();
}

int DirectoryHandler::getFoundSeason()
{
    return directoryParser.getFoundSeason();
}

QStringList DirectoryHandler::getFiles()
{
    return directoryParser.getFiles();
}

QStringList DirectoryHandler::getFilesWithoutSuffix()
{
    return directoryParser.getFilesWithoutSuffix();
}

QStringList DirectoryHandler::getFilesSuffix()
{
    return directoryParser.getFilesSuffix();
}

void DirectoryHandler::setNewFileNames(QStringList newFileNameList)
{
    fileRenamer.setNewFileNames(newFileNameList);
}

bool DirectoryHandler::isUndoPossible()
{
    return fileRenamer.isUndoPossible();
}

void DirectoryHandler::initializeDirectory(QString path)
{
    bool initialized = directoryParser.initializeDirectory(path);
    if (initialized)
    {
        fileRenamer.setDirectory(directoryParser.getDirectory());
        fileRenamer.setOldFileNames(directoryParser.getFiles());
    }
    emit directoryInitialized(initialized);
}

void DirectoryHandler::rename()
{
    bool renameSuccess = renameFiles();
    emit renameDone(renameSuccess);
}

void DirectoryHandler::undoRename()
{
    if (fileRenamer.isUndoPossible() && fileRenamer.prepareUndo())
    {
        bool undoSuccess = renameFiles(true);
        emit undoRenameDone(undoSuccess);
    }
}
