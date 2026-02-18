#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QLabel>
#include "LeftPane.h"
#include "CanvasTabWidget.h"
#include "DataModel.h"

namespace Viewer {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void loadFile(const QString& filePath);

private slots:
    void onOpenCSV();
    void onOpenBinary();
    void onCloseFile();
    void onCloseSpecificFile(const QString& filePath);
    void onCloseAllFiles();
    void onExportBinary();
    void onExit();

    void onCreateNewCanvas();
    void onDestroyCanvas();
    void onRenameCanvas();

    void onAbout();

    void onDataLoaded(const QString& filePath);
    void onFileAdded(const QString& filePath);
    void onFileRemoved(const QString& filePath);
    void onDataCleared();
    void onDataError(const QString& error);

    void onSeriesSelected(const QStringList& seriesNames);
    void onSeriesDeselected(const QStringList& seriesNames);
    void onAddToCanvasRequested(const QStringList& seriesNames);
    void onRemoveFromCanvasRequested(const QStringList& seriesNames);

    void onCoordinateSelected(const QString& seriesName, double x, double y);
    void onSeriesRemoved(const QString& name);
    void onCurrentCanvasChanged(int index);
    void onCanvasCreated(const QString& name);
    void onCanvasDestroyed(int index);

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void createStatusBar();

    void setupLayout();

    QString getOpenCSVFileName();
    QString getOpenBinaryFileName();
    QString getSaveBinaryFileName();
    void updateFileInfoLabel();

    DataModel* m_dataModel;
    LeftPane* m_leftPane;
    CanvasTabWidget* m_canvasArea;
    QSplitter* m_splitter;

    QStatusBar* m_statusBar;
    QLabel* m_coordinateLabel;
    QLabel* m_fileInfoLabel;

    QAction* m_openCSVAction;
    QAction* m_openBinaryAction;
    QAction* m_closeFileAction;
    QAction* m_closeAllAction;
    QAction* m_exportBinaryAction;
    QAction* m_exitAction;

    QAction* m_newCanvasAction;
    QAction* m_destroyCanvasAction;
    QAction* m_renameCanvasAction;

    QAction* m_aboutAction;
};

} // namespace Viewer

#endif // MAINWINDOW_H
