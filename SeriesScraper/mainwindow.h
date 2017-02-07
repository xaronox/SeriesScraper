#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define APPLICATIONNAME "SeriesScraper"
#define NUMBER_PATH_STRUCTURE_COMBOBOXES 4

#include <QMainWindow>
#include <QDir>
#include <QLabel>
#include <QProgressBar>
#include <QGraphicsBlurEffect>
#include <QStyleFactory>
#include <QMessageBox>
#include <QComboBox>
#include "customshadoweffect.h"
#include "keypresseater.h"
#include "aboutdialog.h"
#include "message.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    Ui::MainWindow *ui;
    QTimer *setPathTimer;
    QTimer *seriesTextChangeTimer;
    QTimer *progressBarTimer;
    QTimer *disableSeriesProgressbarTimer;
    QTimer *clearStatusTextTimer;
    QLabel *seriesStatusLabel;
    QLabel *statusBarTypeImageLabel;
    QProgressBar *seriesProgressBar;
    QPoint *tableItemPoint;
    QGraphicsBlurEffect *blur;
    CustomShadowEffect *shadow;
    KeyPressEater *keyPressEaterEscape;
    KeyPressEater *keyPressEaterEnter;
    QMessageBox *renameConfirmationMessageBox;
    QMessageBox *posterConfirmationMessageBox;
    QMessageBox *undoRenameConfirmationBox;
    QComboBox* pathStructureComboBoxList[NUMBER_PATH_STRUCTURE_COMBOBOXES];

    QMenu *fileMenu;
    QMenu *helpMenu;
    QMenu *viewMenu;
    QMenu *appearanceMenu;
    QAction *savePosterAction;
    QAction *undoRenameAction;
    QAction *aboutAction;
    QAction *settingsAction;
    QAction *fullScreenAction;

    QDir chosenPath;
    int tableRows;
    int progressIncrement;
    QString whiteBackground;
    QString imageBackground;
    QStringList nameSchemeItemList;
    QString colorWhite = "QLineEdit { background: rgb(255, 255, 255); }";
    QString colorGreen = "QLineEdit { background: rgb(204, 255, 204); }";
    QString colorRed = "QLineEdit { background: rgb(255, 217, 204); }";
    QChar checkmark = QChar(0x13, 0x27);
    QChar times = QChar(0x15, 0x27);
    QPixmap seriesImage;
    bool fullScreenEnabled = false;
    bool seriesInformationEnabled = false;
    QString directorySelectionText = "Directory selection";

    void disableGUIControl();
    void enableGUIControl();
    void setUpGUI();
    void setUpKeyEvents();
    void setUpTable();
    void setUpMenuBar();
    void setUpConfirmationMessageBoxes();
    void setUpEpisodeEdit();
    void setSeriesAvailableStatus(bool status, bool isEmpty);
    void setAmountSeasons(int amount);
    void addNameSchemeItem(QString nameScheme);
    void removeNameSchemeItem(int itemIndex);
    void setStatusMessage(QString message, int type);
    void changeLocalization(QStringList translationList);
    void resizeEvent(QResizeEvent *event);
    void updateView(QStringList oldFileNames, QStringList newFileNames, int amountSeasons);
    void updateDirectoryWidget(std::vector<QStringList> pathStructure);
    void clearDirectoryWidget();
    void updateDirectoryWidgetVisibility();
    void changeToDarkTheme();

public:
    explicit MainWindow(QWidget *parent = 0);
    enum statusMessageType
    {
        error,
        success,
        info
    };

    ~MainWindow();

private slots:
    bool setRow(int row, QString oldFileName, QString newFileName);
    void clearTable();
    void openDirectory();
    void setPath();
    void startSetPathTimer();
    void onUpdateDirectory();
    void startSeriesTextChangeTimer();
    void disableSeriesProgressbar();
    void updateProgressbar();
    void onCellClicked(int row, int coloumn);
    void onSeriesTextChanged();
    void onSeasonChanged(int index);
    void onSeriesLanguageChanged(int index);
    void onRenameButtonPressed();
    void onNameSchemeChanged(int index);
    void onChangeEpisodeText();
    void onTableEnter();
    void clearStatusText();

    void savePoster();
    void undoRenaming();
    void showAboutDialog();
    void showSettingsWindow();
    void toggleFullScreen();

public slots:
    void notify(Message &msg);

signals:
    void sendMessage(Message &msg);
};

#endif // MAINWINDOW_H
