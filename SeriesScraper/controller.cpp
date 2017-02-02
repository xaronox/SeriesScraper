#include "controller.h"
#include <QDebug>
#include <vector>

void Controller::initializeFileTypes()
{
    bool loadingSuccessful = fileTypeHandler.loadFileTypeFile();

    QStringList fileTypes = fileTypeHandler.getFileTypes();
    directoryParser.setFileTypes(fileTypes);

    Message msgSetFileTypes;
    msgSetFileTypes.type = Message::controller_setFileTypes_settings;
    msgSetFileTypes.data[0].qsListPointer = &fileTypes;
    emit(sendMessage(msgSetFileTypes));

    if (!loadingSuccessful)
    {
        // Todo localize
        QString fileFormatNotFound = interfaceLanguageHandler.getTranslation(LanguageData::fileFormatNotFound);
        setStatusMessage(fileFormatNotFound);
    }
}

void Controller::initializeNameSchemes()
{
    QStringList nameSchemeRepresentationList;
    QStringList nameSchemeList;
    bool fileRead = nameSchemeHandler.readNameSchemeFile();
    int amountNameSchemes = nameSchemeHandler.getAmountNameSchemes();

    // Read all name schemes
    if (fileRead && amountNameSchemes > 0)
    {
        for (int i = 0; i < amountNameSchemes; i++)
        {
            nameSchemeHandler.setNameScheme(i);
            nameSchemeRepresentationList <<  nameSchemeHandler.getNameSchemeRepresentation();
            nameSchemeList << nameSchemeHandler.getNameScheme();
        }
    } else // Add default entry if name scheme list not found or empty
    {
        nameSchemeHandler.addNameScheme("|Simple| <series> - S<season(2)>E<episode(2)> - <episodeName>");
        nameSchemeRepresentationList << nameSchemeHandler.getNameSchemeRepresentation();
        // Error message
        QString nameSchemeFileNotFound = interfaceLanguageHandler.getTranslation(LanguageData::nameSchemeFileNotFound);
        setStatusMessage(nameSchemeFileNotFound);
    }

    // Send name scheme list to view
    Message msgAddNameSchemes;
    msgAddNameSchemes.type = Message::controller_addNameSchemes_view;
    msgAddNameSchemes.data[0].qsListPointer = &nameSchemeRepresentationList;
    emit(sendMessage(msgAddNameSchemes));

    // Send raw name scheme list to settings
    Message msgSetRawNameSchemes;
    msgSetRawNameSchemes.type = Message::controller_setRawNameSchemes_settings;
    msgSetRawNameSchemes.data[0].qsListPointer = &nameSchemeList;
    emit(sendMessage(msgSetRawNameSchemes));
}

void Controller::initializeSeriesLanguages()
{
    bool fileRead = seriesLanguageHandler.loadSeriesLanguageFile();
    QStringList seriesLanguageList = seriesLanguageHandler.getLanguageList();

    // Add default entry if series language file not found or empty
    if (!fileRead || seriesLanguageList.empty())
    {
        seriesLanguageList << QString("English");
        // Error message
        QString languageFileReadingFailure = interfaceLanguageHandler.getTranslation(LanguageData::seriesLanguageNotFound);
        setStatusMessage(languageFileReadingFailure);
    }

    // Send series language list to view
    Message msgAddSeriesLanguages;
    msgAddSeriesLanguages.type = Message::controller_addSeriesLanguages_view;
    msgAddSeriesLanguages.data[0].qsListPointer = &seriesLanguageList;
    emit(sendMessage(msgAddSeriesLanguages));

    QString languageShortName = seriesLanguageHandler.getShortName(0); // Settings
    seriesData.setSelectedLanguage(languageShortName); // Settings
}

void Controller::initializeInterfaceLanguages()
{
    bool languageInitSuccess = interfaceLanguageHandler.initialize();
    if (languageInitSuccess)
    {
        QStringList interfaceLanguageList = interfaceLanguageHandler.getLanguageList();
        // Send gui language list to settings
        Message msgAddGUILanguages;
        msgAddGUILanguages.type = Message::controller_addGUILanguages_settings;
        msgAddGUILanguages.data[0].qsListPointer = &interfaceLanguageList;
        emit(sendMessage(msgAddGUILanguages));
    } else // No language files found
    {
        Message msgNoGUILanguagesFound;
        msgNoGUILanguagesFound.type = Message::controller_noGUILanguagesFound_settings;
        emit(sendMessage(msgNoGUILanguagesFound));
        // Error message
        setStatusMessage("No language files found");
    }
}

void Controller::initializeSettings()
{
    settings.loadSettingsFile();

    int selectedSeriesParser = settings.getSeriesDatabase();
    int selectedNameScheme = settings.getNameScheme();
    QString selectedInterfaceLanguage = settings.getGuiLanguage();
    QString selectedSeriesLanguage = settings.getSeriesLanguage();
    bool saveSeries = settings.getSaveSeries();
    bool savePath = settings.getSavePath();
    bool savePosterInDirectory = settings.getSavePosterInDirectory();
    bool setDetectedSeason = settings.getAutoSetDetectedSeason();
    bool useDarkTheme = settings.getDarkTheme();
    bool showSeriesInfo = settings.getShowSeriesInfo();

    // Dont change theme afterwards!
    if (useDarkTheme)
    {
        Message msgUseDarkTheme;
        msgUseDarkTheme.type = Message::controller_useDarkTheme_view;
        emit(sendMessage(msgUseDarkTheme));
    }

    changeInterfaceLanguage(selectedInterfaceLanguage);
    changeSeriesParser(selectedSeriesParser);
    changeNameScheme(selectedNameScheme);
    changeSeriesLanguage(selectedSeriesLanguage);
    changeSaveSeries(saveSeries);
    changeSavePath(savePath);

    Message msgSavePosterInDirectory;
    msgSavePosterInDirectory.type = Message::controller_savePoster_settings;
    msgSavePosterInDirectory.data[0].b = savePosterInDirectory;
    emit(sendMessage(msgSavePosterInDirectory));

    Message msgSetDetectedSeason;
    msgSetDetectedSeason.type = Message::controller_setDetectedSeason_settings;
    msgSetDetectedSeason.data[0].b = setDetectedSeason;
    emit(sendMessage(msgSetDetectedSeason));

    Message msgUseDarkTheme;
    msgUseDarkTheme.type = Message::controller_useDarkTheme_settings;
    msgUseDarkTheme.data[0].b = useDarkTheme;
    emit(sendMessage(msgUseDarkTheme));

    Message msgShowSeriesInfo;
    msgShowSeriesInfo.type = Message::controller_showSeriesInfo_settings;
    msgShowSeriesInfo.data[0].b = showSeriesInfo;
    emit(sendMessage(msgShowSeriesInfo));
}

void Controller::updateNewFileNames()
{
    QString series = seriesData.getSeries();
    QString airDate = seriesData.getAirDate();
    int season = seriesData.getSelectedSeason();
    int amountEpisodes = seriesData.getAmountEpisodes();
    QStringList episodeList = seriesData.getEpisodes();
    QStringList suffixList = seriesData.getSuffixes();

    QStringList newFileNames;
    QStringList newFileNamesWithoutSuffix = nameSchemeHandler.getFileNameList(series, airDate, season, amountEpisodes, episodeList);

    int minimumSize = std::min(newFileNamesWithoutSuffix.size(), suffixList.size());
    for (int i = 0; i < minimumSize; i++)
        newFileNames << (newFileNamesWithoutSuffix.at(i) + "." + suffixList.at(i));

    seriesData.setNewFileNames(newFileNames);
    seriesData.setNewFileNamesWithoutSuffix(newFileNamesWithoutSuffix);
}

void Controller::setStatusMessage(QString status)
{
    Message msgSetStatus;
    msgSetStatus.type = Message::controller_setStatus_view;
    msgSetStatus.data[0].qsPointer =  &status;
    emit(sendMessage(msgSetStatus));
}

void Controller::setSeriesInformation()
{
    QString posterUrl = seriesData.getPosterUrl();
    QByteArray *poster = NULL;
    if (fileDownloader.downloadFile(posterUrl))
        poster = fileDownloader.getDownloadedData();
    QString totalEpisodes = QString::number(seriesData.getAmountEpisodes());
    QString season = QString::number(seriesData.getSelectedSeason());
    QString totalSeasons = QString::number(seriesData.getAmountSeasons());
    QString seriesName = seriesData.getSeries();
    QString airDate = seriesData.getAirDate();
    QString plot = seriesData.getPlot();

    // Set default values for view, if no series set
    if (seriesName.isEmpty() && totalSeasons == "0")
    {
        poster = NULL;
        totalEpisodes = "-";
        season = "-";
        totalSeasons = "-";
        seriesName = "";
        airDate = "-";
        plot = "";
    }

    Message msgSetSeriesInfo;
    msgSetSeriesInfo.type = Message::controller_setSeriesInfo_view;
    msgSetSeriesInfo.data[0].qbPointer = poster;
    msgSetSeriesInfo.data[1].qsPointer = &totalEpisodes;
    msgSetSeriesInfo.data[2].qsPointer = &season;
    msgSetSeriesInfo.data[3].qsPointer = &totalSeasons;
    msgSetSeriesInfo.data[4].qsPointer = &seriesName;
    msgSetSeriesInfo.data[5].qsPointer = &airDate;
    msgSetSeriesInfo.data[6].qsPointer = &plot;
    emit(sendMessage(msgSetSeriesInfo));
}

Controller::Controller(QObject *parent) : QObject(parent)
{
}

Controller::~Controller()
{
    bool saveSeries = settings.getSaveSeries();
    bool savePath = settings.getSavePath();
    if (saveSeries)
    {
        QString series = seriesParser.getSeriesInput();
        int season = seriesData.getSelectedSeason();
        settings.setSeries(series);
        settings.setSeason(season);
    } else
    {
        settings.setSeries("");
        settings.setSeason(1);
    }

    if (savePath)
    {
        QString path = directoryParser.getDirectoryPathInput();
        settings.setPath(path);
    } else
        settings.setPath("");

    settings.saveSettingsFile();
}

void Controller::initialize()
{
    initializeFileTypes();
    initializeSeriesLanguages();
    initializeInterfaceLanguages();
    initializeNameSchemes();
    initializeSettings();
}

bool Controller::loadSeries(QString series, int season)
{
    // Set default values
    QString newSeriesName("");
    QString newAirDate("");
    QString newPlot("");
    QString newPosterUrl("");
    int newSelectedSeason = 1;
    int newAmountSeasons = 0;
    QStringList newEpisodeList;
    bool seriesFound = false;

    // Dont scrape the empty string
    if (!series.isEmpty())
    {
        // Start loading animation
        Message msgStartLoading;
        msgStartLoading.type = Message::controller_startSeriesLoading_view;
        emit(sendMessage(msgStartLoading));

        seriesFound = seriesParser.scrapeSeries(series);
        if (seriesFound)
        {
            QString seriesLanguage = seriesData.getSelectedLanguage();
            newAmountSeasons = seriesParser.getAmountSeasons();

            int seasonToLoad = season;
            if (season > newAmountSeasons) // Avoid loading not existing seasons
            {
                seasonToLoad = 1;
            }
            newEpisodeList = seriesParser.getEpisodeList(seasonToLoad, seriesLanguage);
            newSelectedSeason = seasonToLoad;
            newSeriesName = seriesParser.getSeriesName();
            newPosterUrl = seriesParser.getPosterUrl();
            newPlot = seriesParser.getPlot();
            newAirDate = seriesParser.getSeriesYear();

            // Finish loading animation
            Message msgSuccessLoading;
            msgSuccessLoading.type = Message::controller_successSeriesLoading_view;
            emit(sendMessage(msgSuccessLoading));
        } else // No series found
        {
            // Finish loading animation with failure message
            Message msgFailureLoading;
            msgFailureLoading.type = Message::controller_failureSeriesLoading_view;
            emit(sendMessage(msgFailureLoading));
        }
    } else
        seriesParser.scrapeSeries(""); // Reset

    seriesData.setSeries(newSeriesName);
    seriesData.setAirDate(newAirDate);
    seriesData.setPosterUrl(newPosterUrl);
    seriesData.setPlot(newPlot);
    seriesData.setAmountSeasons(newAmountSeasons);
    seriesData.setSelectedSeason(newSelectedSeason);
    seriesData.setEpisodes(newEpisodeList);

    if (settings.getShowSeriesInfo())
        setSeriesInformation();

    updateNewFileNames();
    updateView();
    return seriesFound;
}

bool Controller::changeSeason(int season)
{
    QString series = seriesParser.getSeriesInput();
    return loadSeries(series, season);
}

bool Controller::changeInterfaceLanguage(QString language)
{
    bool loadingSuccessful = interfaceLanguageHandler.loadLanguage(language);
    QString interfaceLanguage = "English";
    if (loadingSuccessful)
    {
        interfaceLanguage = language;
        QStringList translationList = interfaceLanguageHandler.getTranslationList();
        // Send translations to view, about and settings
        Message msgChangeLocalization;
        msgChangeLocalization.type = Message::controller_changeLocalization_view;
        msgChangeLocalization.data[0].qsListPointer = &translationList;
        msgChangeLocalization.data[1].qsPointer = &language;
        emit(sendMessage(msgChangeLocalization));
    } else
    {
        // Error message
        setStatusMessage("Could not read language file " + language + ".json");
    }

    settings.setGuiLanguage(interfaceLanguage);
    return loadingSuccessful;
}

void Controller::changeSeriesLanguage(QString language)
{
    Message msgChangeSeriesLanguage;
    msgChangeSeriesLanguage.type = Message::controller_changeSeriesLanguage_view;
    msgChangeSeriesLanguage.data[0].qsPointer = &language;
    emit(sendMessage(msgChangeSeriesLanguage));

    QString selectedLanguage = seriesLanguageHandler.getShortName(language);
    seriesData.setSelectedLanguage(selectedLanguage);
    settings.setSeriesLanguage(language);
    QString series = seriesParser.getSeriesInput();
    int season = seriesData.getSelectedSeason();
    loadSeries(series, season);
}

void Controller::changeSeriesParser(int selectedSeriesParser)
{
    Message msgChangeSeriesParser;
    msgChangeSeriesParser.type = Message::controller_changeSeriesParser_view;
    msgChangeSeriesParser.data[0].i = selectedSeriesParser;
    emit(sendMessage(msgChangeSeriesParser));

    int oldSeriesParser = seriesParser.getSeriesParser();
    if (selectedSeriesParser != oldSeriesParser)
    {
        QString series = seriesParser.getSeriesInput();
        int season = seriesData.getSelectedSeason();
        seriesParser.setSeriesParser(selectedSeriesParser);
        settings.setSeriesDatabase(selectedSeriesParser);
        if (!series.isEmpty())
            loadSeries(series, season);
    }
}

void Controller::changeSeries(QString series, int season)
{
    bool isEmpty = series.isEmpty();
    bool seriesSet = loadSeries(series, season);

    // Send scraping status to view
    Message msgSeriesSet;
    msgSeriesSet.type = Message::controller_seriesSet_view;
    msgSeriesSet.data[0].b = seriesSet;
    msgSeriesSet.data[1].b = isEmpty;
    emit(sendMessage(msgSeriesSet));

    // Update series and season on view
    Message msgSetSeries;
    msgSetSeries.type = Message::controller_setSeries_view;
    msgSetSeries.data[0].qsPointer = &series;
    msgSetSeries.data[1].i = seriesData.getSelectedSeason();
    emit(sendMessage(msgSetSeries));
}

void Controller::changeSaveSeries(bool saveSeries)
{
    if (saveSeries)
    {
        QString series = settings.getSeries();
        int season = settings.getSeason();
        changeSeries(series, season);

        Message msgSetSeries;
        msgSetSeries.type = Message::controller_setSeries_view;
        msgSetSeries.data[0].qsPointer = &series;
        msgSetSeries.data[1].i = season;
        emit(sendMessage(msgSetSeries));
    }
    Message msgSaveSeries;
    msgSaveSeries.type = Message::controller_saveSeries_settings;
    msgSaveSeries.data[0].b = saveSeries;
    emit(sendMessage(msgSaveSeries));
}

void Controller::changeSavePath(bool savePath)
{
    if (savePath)
    {
        QString path = settings.getPath();
        Message msgSetPath;
        msgSetPath.type = Message::controller_setPath_view;
        msgSetPath.data[0].qsPointer = &path;
        emit(sendMessage(msgSetPath));
    }
    Message msgSavePath;
    msgSavePath.type = Message::controller_savePath_settings;
    msgSavePath.data[0].b = savePath;
    emit(sendMessage(msgSavePath));
}

void Controller::changeNameScheme(int nameScheme)
{
    int amountNameSchemes = nameSchemeHandler.getAmountNameSchemes();
    int newNameScheme = 0;

    if (nameScheme < amountNameSchemes)
        newNameScheme = nameScheme;
    else
    {
        QString nameSchemeLineNotFound = interfaceLanguageHandler.getTranslation(LanguageData::nameSchemeLineNotFound);
        setStatusMessage(nameSchemeLineNotFound + " " +  QString::number(nameScheme + 1));
    }

    settings.setNameScheme(newNameScheme);
    nameSchemeHandler.setNameScheme(newNameScheme);
    Message msgChangeNameScheme;
    msgChangeNameScheme.type = Message::controller_changeNameScheme_view;
    msgChangeNameScheme.data[0].i = newNameScheme;
    emit(sendMessage(msgChangeNameScheme));
}

void Controller::savePoster()
{
    if (!fileDownloader.fileExists())
    {
        if (fileDownloader.saveFile())
            setStatusMessage(interfaceLanguageHandler.getTranslation(LanguageData::posterSaved));
        else
            setStatusMessage(interfaceLanguageHandler.getTranslation(LanguageData::posterNotSaved));
    }
    else
    {
        Message msgPosterAlreadyExists;
        msgPosterAlreadyExists.type = Message::controller_posterAlreadyExists_view;
        emit(sendMessage(msgPosterAlreadyExists));
    }
}

bool Controller::setDirectory(QString path)
{
    // Discard undo action when the path changed
    if (seriesData.getWorkingDirectory().absolutePath() != path)
    {
        fileRenamer.deleteLastUndo();

        Message msgDisableUndoRenaming;
        msgDisableUndoRenaming.type = Message::controller_enableUndoRenaming_view;
        msgDisableUndoRenaming.data[0].b = false;
        emit(sendMessage(msgDisableUndoRenaming));
    }

    bool directoryExists = directoryParser.initializeDirectory(path);
    QDir newDirectory("");
    QStringList newOldFileNames;
    QStringList newOldFileNamesWithoutSuffixes;
    QStringList newSuffixes;

    // Update directory and file infos
    if (directoryExists)
    {
        newDirectory = QDir(path);
        newOldFileNames = directoryParser.getFiles();
        newSuffixes = directoryParser.getFilesSuffix();
        newOldFileNamesWithoutSuffixes = directoryParser.getFilesWithoutSuffix();

        fileDownloader.setFilePath(path, "poster.jpg");
    }

    seriesData.setWorkingDirectory(newDirectory);
    seriesData.setOldFileNames(newOldFileNames);
    seriesData.setOldFileNamesWithoutSuffix(newOldFileNamesWithoutSuffixes);
    seriesData.setSuffixes(newSuffixes);

    return directoryExists;
}

bool Controller::renameFiles()
{
    QDir directory = seriesData.getWorkingDirectory();
    QStringList newFileNameList = seriesData.getNewFileNames();
    QStringList oldFileNameList = seriesData.getOldFileNames();

    fileRenamer.setDirectory(directory);
    fileRenamer.setOldFileNames(oldFileNameList);
    fileRenamer.setNewFileNames(newFileNameList);
    bool renameSuccess = fileRenamer.rename();

    if (settings.getSavePosterInDirectory())
        savePoster();

    if (renameSuccess)
    {
        directoryParser.initializeDirectory(directory.absolutePath());
        QStringList renamedFiles = directoryParser.getFiles();
        QStringList renamedFilesWithoutSuffix = directoryParser.getFilesWithoutSuffix();
        seriesData.setOldFileNames(renamedFiles);
        seriesData.setOldFileNamesWithoutSuffix(renamedFilesWithoutSuffix);
        updateView();

        // Enable undo action
        Message msgEnableUndoRenaming;
        msgEnableUndoRenaming.type = Message::controller_enableUndoRenaming_view;
        msgEnableUndoRenaming.data[0].b = true;
        emit(sendMessage(msgEnableUndoRenaming));

        // Success Message
        QString renameSuccessful = interfaceLanguageHandler.getTranslation(LanguageData::renameSuccess);
        setStatusMessage(renameSuccessful);
    } else
    {
        // Failure Message
        QString renameFailure = interfaceLanguageHandler.getTranslation(LanguageData::renameFailed);
        setStatusMessage(renameFailure);
    }
    Message msgRenameFinished;
    msgRenameFinished.type = Message::controller_renameFinished_view;
    emit(sendMessage(msgRenameFinished));

    return renameSuccess;
}

bool Controller::undoRenameFiles()
{
    if (fileRenamer.isUndoPossible())
    {
        bool undoRenameSuccess = fileRenamer.undo();

        if (undoRenameSuccess)
        {
            directoryParser.initializeDirectory(seriesData.getWorkingDirectory().absolutePath());
            QStringList renamedFiles = directoryParser.getFiles();
            QStringList renamedFilesWithoutSuffix = directoryParser.getFilesWithoutSuffix();
            seriesData.setOldFileNames(renamedFiles);
            seriesData.setOldFileNamesWithoutSuffix(renamedFilesWithoutSuffix);
            updateView();

            // Success Message
            QString undoRenameSuccessful = interfaceLanguageHandler.getTranslation(LanguageData::undoRenamingSuccessful);
            setStatusMessage(undoRenameSuccessful);
        } else
        {
            // Failure Message
            QString undoRenameFailure = interfaceLanguageHandler.getTranslation(LanguageData::undoRenamingFailed);
            setStatusMessage(undoRenameFailure);
        }
        Message msgRenameFinished;
        msgRenameFinished.type = Message::controller_renameFinished_view;
        emit(sendMessage(msgRenameFinished));

        return undoRenameSuccess;
    } else
    {
        return false;
    }
}

void Controller::updateView()
{
    QStringList newFileNameList = seriesData.getNewFileNamesWithoutSuffix();
    QStringList oldFileNameList = seriesData.getOldFileNamesWithoutSuffix();
    int amountSeasons = seriesData.getAmountSeasons();

    Message msgViewUpdate;
    msgViewUpdate.type = Message::controller_updateView_view;
    msgViewUpdate.data[0].i = amountSeasons;
    msgViewUpdate.data[1].qsListPointer = &oldFileNameList;
    msgViewUpdate.data[2].qsListPointer = &newFileNameList;
    emit(sendMessage(msgViewUpdate));
    updateRenameButtonAndSavePoster();
}

void Controller::updateRenameButtonAndSavePoster()
{
    QDir testDir("");
    bool differentFileNames = false;
    QStringList oldNames = seriesData.getOldFileNames();
    QStringList newNames = seriesData.getNewFileNames();
    int minimumSize = std::min(oldNames.size(), newNames.size());
    for (int i = 0; i < minimumSize; i ++)
    {
        bool differentNames = (oldNames.at(i) != newNames.at(i));
        bool oldNameIsEmpty = oldNames.at(i).isEmpty();
        bool newNameIsEmpty = newNames.at(i).isEmpty();
        if (differentNames && !oldNameIsEmpty && !newNameIsEmpty)
        {
            differentFileNames = true;
            break;
        }
    }

    bool directorySet = (seriesData.getWorkingDirectory().absolutePath() != testDir.absolutePath());
    bool seriesSet = !seriesData.getSeries().isEmpty();
    bool enableRenameButton = seriesSet & directorySet & differentFileNames;
    bool enableSavePoster = seriesSet & directorySet;

    Message msgEnableRenameButton;
    msgEnableRenameButton.type = Message::controller_enableRenameButton_view;
    msgEnableRenameButton.data[0].b = enableRenameButton;
    emit(sendMessage(msgEnableRenameButton));

    Message msgEnableSavePoster;
    msgEnableSavePoster.type = Message::controller_enableSavePoster_view;
    msgEnableSavePoster.data[0].b = enableSavePoster;
    emit(sendMessage(msgEnableSavePoster));
}

void Controller::removeFileType(int index)
{
    fileTypeHandler.removeFileType(index);
    QStringList fileTypes = fileTypeHandler.getFileTypes();

    directoryParser.setFileTypes(fileTypes);
    QDir directory = seriesData.getWorkingDirectory();
    setDirectory(directory.absolutePath());
    updateView();

    Message msgRemoveFileType;
    msgRemoveFileType.type = Message::controller_removeFileType_settings;
    msgRemoveFileType.data[0].i = index;
    emit(sendMessage(msgRemoveFileType));

}

void Controller::notify(Message &msg)
{
    switch (msg.type)
    {
    case Message::view_changeSeriesText_controller:
    {
        QString series = *msg.data[0].qsPointer;
        QString oldSeries = seriesParser.getSeriesInput();
        // Only write on change
        if (series != oldSeries)
        {
            int season = 1;
            if (settings.getAutoSetDetectedSeason())
            {
                int foundSeason = directoryParser.getFoundSeason();
                if (foundSeason > 0 && foundSeason <= 100)
                {
                    season = foundSeason;
                }
            }
            changeSeries(series, season);
        }
        break;
    }
    case Message::view_changeSeason_controller:
    {
        int selectedSeason = msg.data[0].i;
        int oldSelectedSeason = seriesData.getSelectedSeason();
        // Only write on change
        if (selectedSeason != oldSelectedSeason)
            changeSeason(selectedSeason);
        break;
    }
    case Message::view_changeSeriesLanguage_controller:
    {
        int languageIndex = msg.data[0].i;
        QString language = seriesLanguageHandler.getLanguageList().at(languageIndex);
        QString languageShortName = seriesLanguageHandler.getShortName(languageIndex + 1);
        QString oldLanguageShortName = seriesData.getSelectedLanguage();
        // Only write on change
        if (oldLanguageShortName != languageShortName)
            changeSeriesLanguage(language);
        break;
    }
    case Message::view_changeDirectory_controller:
    {
        QString path = *msg.data[0].qsPointer;
        setDirectory(path);
        updateNewFileNames();
        updateView();
        break;
    }
    case Message::view_updateDirectory_controller:
    {
        QString path = *msg.data[0].qsPointer;
        setDirectory(path);
        updateNewFileNames();
        updateView();
        break;
    }
    case Message::view_changeEpisodeName_controller:
    {
        int episode = msg.data[0].i;
        QString newEpisodeName = *msg.data[1].qsPointer;
        QString oldEpisodeName = seriesData.getEpisode(episode);
        if (oldEpisodeName != newEpisodeName) // Only write on change
        {
            seriesData.setEpisode(episode, newEpisodeName);
            updateNewFileNames();
            updateView();
        }
        break;
    }
    case Message::view_getEpisodeName_controller:
    {
        int episode = msg.data[0].i;
        QString episodeName = seriesData.getEpisode(episode);

        Message msgReturnEpisodeName;
        msgReturnEpisodeName.type = Message::controller_returnEpisodeName_view;
        msgReturnEpisodeName.data[0].qsPointer = &episodeName;
        emit(sendMessage(msgReturnEpisodeName));
        break;
    }
    case Message::view_rename_controller:
    {
        int foundSeason = directoryParser.getFoundSeason();
        int selectedSeason =  seriesData.getSelectedSeason();

        if (foundSeason == 0 || foundSeason == selectedSeason)
            renameFiles();
        else
        {
            Message msgSeasonMismatch;
            msgSeasonMismatch.type = Message::controller_seasonMismatch_view;
            emit(sendMessage(msgSeasonMismatch));
        }
        break;
    }
    case Message::view_forceRename_controller:
    {
        renameFiles();
        break;
    }
    case Message::view_undoRenaming_controller:
    {
        undoRenameFiles();
        break;
    }
    case Message::view_changeNameScheme_controller:
    {
        int selectedNameScheme = msg.data[0].i;
        changeNameScheme(selectedNameScheme);
        updateNewFileNames();
        updateView();
        break;
    }
    case Message::settings_changeGUILanguage_controller:
    {
        QString language = *msg.data[0].qsPointer;
        changeInterfaceLanguage(language);
        break;
    }
    case Message::view_savePoster_controller:
    {
        savePoster();
        break;
    }
    case Message::view_forceSavePoster_conroller:
    {
        if (fileDownloader.saveFile(true))
            setStatusMessage(interfaceLanguageHandler.getTranslation(LanguageData::posterSaved));
        else
            setStatusMessage(interfaceLanguageHandler.getTranslation(LanguageData::posterNotSaved));
        break;
    }
    case Message::view_showAboutDialog_controller:
    {
        Message msgShowAboutDialog;
        msgShowAboutDialog.type = Message::controller_showAboutDialog_about;
        emit(sendMessage(msgShowAboutDialog));
        break;
    }
    case Message::view_showSettingsWindow_controller:
    {
        Message msgShowSettingsDialog;
        msgShowSettingsDialog.type = Message::controller_showSettingsWindow_settings;
        emit(sendMessage(msgShowSettingsDialog));
        break;
    }
    case Message::settings_changeSeriesParser_controller:
    {
        int selectedSeriesParser = msg.data[0].i;
        changeSeriesParser(selectedSeriesParser);
        break;
    }
    case Message::settings_saveSeries_controller:
    {
        bool saveSeries = msg.data[0].b;
        settings.setSaveSeries(saveSeries);
        break;
    }
    case Message::settings_savePath_controller:
    {
        bool savePath = msg.data[0].b;
        settings.setSavePath(savePath);
        break;
    }
    case Message::settings_savePoster_controller:
    {
        bool savePoster = msg.data[0].b;
        settings.setSavePosterInDirectory(savePoster);
        break;
    }
    case Message::settings_setDetectedSeason_controller:
    {
        bool setDetectedSeason = msg.data[0].b;
        settings.setAutoSetDetectedSeason(setDetectedSeason);
        break;
    }
    case Message::settings_reset_controller:
    {
        settings.resetSettingsFile();
        initializeSettings();
        break;
    }
    case Message::settings_useDarkTheme_controller:
    {
        bool useDarkTheme = msg.data[0].b;
        settings.setDarkTheme(useDarkTheme);
        break;
    }
    case Message::settings_showSeriesInfo_controller:
    {
        bool showSeriesInfo = msg.data[0].b;
        settings.setShowSeriesInfo(showSeriesInfo);

        if (showSeriesInfo)
            setSeriesInformation();

        Message msgShowSeriesInfo;
        msgShowSeriesInfo.type = Message::controller_showSeriesInfo_view;
        msgShowSeriesInfo.data[0].b = showSeriesInfo;
        emit(sendMessage(msgShowSeriesInfo));

        break;
    }
    case Message::settings_addNameScheme_controller:
    {
        QString nameScheme = *msg.data[0].qsPointer;
        int oldSelectedNameScheme = nameSchemeHandler.getSelectedNameSchemeIndex();

        nameSchemeHandler.addNameScheme(nameScheme);
        int lastIndex = nameSchemeHandler.getAmountNameSchemes() - 1;
        nameSchemeHandler.setNameScheme(lastIndex);
        QString nameSchemeRepresentation = nameSchemeHandler.getNameSchemeRepresentation();
        QString newNameScheme = nameSchemeHandler.getNameScheme();
        nameSchemeHandler.setNameScheme(oldSelectedNameScheme);

        Message msgAddNameSchemeView;
        msgAddNameSchemeView.type = Message::controller_addNameScheme_view;
        msgAddNameSchemeView.data[0].qsPointer = &nameSchemeRepresentation;
        emit(sendMessage(msgAddNameSchemeView));

        Message msgAddRawNameSchemeSettings;
        msgAddRawNameSchemeSettings.type = Message::controller_addRawNameScheme_settings;
        msgAddRawNameSchemeSettings.data[0].qsPointer = &newNameScheme;
        emit(sendMessage(msgAddRawNameSchemeSettings));

        break;
    }
    case Message::settings_removeNameScheme_controller:
    {
        int indexToRemove = msg.data[0].i;
        nameSchemeHandler.removeNameScheme(indexToRemove);

        Message msgRemoveNameScheme;
        msgRemoveNameScheme.type = Message::controller_removeNameScheme_view;
        msgRemoveNameScheme.data[0].i = indexToRemove;
        emit(sendMessage(msgRemoveNameScheme));

        break;
    }
    case Message::settings_replaceNameScheme_controller:
    {
        int indexToChange = msg.data[0].i;
        QString newNameScheme = *msg.data[1].qsPointer;

        if (newNameScheme.isEmpty()) // Remove
        {
            nameSchemeHandler.removeNameScheme(indexToChange);

            Message msgRemoveNameScheme;
            msgRemoveNameScheme.type = Message::controller_removeNameScheme_view;
            msgRemoveNameScheme.data[0].i = indexToChange;
            emit(sendMessage(msgRemoveNameScheme));
        }
        else // Replace
        {
            int oldSelectedNameSchemeIndex = nameSchemeHandler.getSelectedNameSchemeIndex();

            nameSchemeHandler.replaceNameScheme(indexToChange, newNameScheme);
            nameSchemeHandler.setNameScheme(indexToChange);
            QString newNameSchemeRepresentation = nameSchemeHandler.getNameSchemeRepresentation();
            nameSchemeHandler.setNameScheme(oldSelectedNameSchemeIndex);

            Message msgReplaceNameScheme;
            msgReplaceNameScheme.type = Message::controller_replaceNameScheme_view;
            msgReplaceNameScheme.data[0].i = indexToChange;
            msgReplaceNameScheme.data[1].qsPointer = &newNameSchemeRepresentation;
            emit(sendMessage(msgReplaceNameScheme));

            if (oldSelectedNameSchemeIndex == indexToChange)
            {
                changeNameScheme(oldSelectedNameSchemeIndex);
                updateNewFileNames();
                updateView();
            }
        }
        break;
    }
    case Message::settings_addFileType_controller:
    {
        QString fileType = *msg.data[0].qsPointer;
        int position = fileTypeHandler.addFileType(fileType);

        QStringList fileTypes = fileTypeHandler.getFileTypes();
        directoryParser.setFileTypes(fileTypes);
        QDir directory = seriesData.getWorkingDirectory();
        setDirectory(directory.absolutePath());
        updateView();

        Message msgAddFileTypeSettings;
        msgAddFileTypeSettings.type = Message::controller_addFileType_settings;
        msgAddFileTypeSettings.data[0].i = position;
        msgAddFileTypeSettings.data[1].qsListPointer = &fileTypes;
        emit(sendMessage(msgAddFileTypeSettings));

        break;
    }
    case Message::settings_removeFileType_controller:
    {
        int indexToRemove = msg.data[0].i;
        removeFileType(indexToRemove);
        break;
    }
    case Message::settings_replaceFileType_controller:
    {
        int indexToChange = msg.data[0].i;
        QString newNameScheme = *msg.data[1].qsPointer;

        if (newNameScheme.isEmpty()) // Remove
        {
            removeFileType(indexToChange);
        }
        else // Replace
        {
            int position = fileTypeHandler.setFileType(indexToChange, newNameScheme);
            QStringList fileTypes = fileTypeHandler.getFileTypes();

            Message msgChangeFileType;
            msgChangeFileType.type = Message::controller_replaceFileType_settings;
            msgChangeFileType.data[0].i = position;
            msgChangeFileType.data[1].qsListPointer = &fileTypes;
            emit(sendMessage(msgChangeFileType));
        }
        break;
    }
    default:
        break;
    }
}
