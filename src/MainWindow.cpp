#include "MainWindow.h"
#include "DataParser.h"
#include "models/ConfigManager.h"
#include "models/CurveConfig.h"
#include <QApplication>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QSize>
#include <QStyle>
#include <QDebug>

namespace Viewer {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    setWindowTitle("Log Viewer");
    resize(1200, 800);

    m_dataModel = new DataModel(this);
    m_leftPane = new LeftPane(this);
    m_favorPanel = new FavorPanel(this);
    m_canvasArea = new CanvasTabWidget(this);
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_leftSplitter = new QSplitter(Qt::Vertical, this);

    m_leftPane->setDataModel(m_dataModel);
    m_canvasArea->setDataModel(m_dataModel);

    createActions();
    createMenus();
    createToolBar();
    createStatusBar();
    setupLayout();

    connect(m_dataModel, &DataModel::dataLoaded, this, &MainWindow::onDataLoaded);
    connect(m_dataModel, &DataModel::fileAdded, this, &MainWindow::onFileAdded);
    connect(m_dataModel, &DataModel::fileRemoved, this, &MainWindow::onFileRemoved);
    connect(m_dataModel, &DataModel::dataCleared, this, &MainWindow::onDataCleared);
    connect(m_dataModel, &DataModel::dataError, this, &MainWindow::onDataError);

    connect(m_leftPane, &LeftPane::seriesSelected, this, &MainWindow::onSeriesSelected);
    connect(m_leftPane, &LeftPane::seriesDeselected, this, &MainWindow::onSeriesDeselected);
    connect(m_leftPane, &LeftPane::addToCanvasRequested, this, &MainWindow::onAddToCanvasRequested);
    connect(m_leftPane, &LeftPane::removeFromCanvasRequested, this, &MainWindow::onRemoveFromCanvasRequested);
    connect(m_leftPane, &LeftPane::closeFileRequested, this, &MainWindow::onCloseSpecificFile);

    connect(m_canvasArea, &CanvasTabWidget::canvasCreated, this, &MainWindow::onCanvasCreated);
    connect(m_canvasArea, &CanvasTabWidget::canvasDestroyed, this, &MainWindow::onCanvasDestroyed);
    connect(m_canvasArea, &CanvasTabWidget::currentChanged, this, &MainWindow::onCurrentCanvasChanged);
    connect(m_canvasArea, &CanvasTabWidget::saveConfigRequested, this, &MainWindow::onSaveCurveConfig);

    auto connectChartSignals = [this](ChartWidget* chart) {
        // Use UniqueConnection to prevent duplicate connections when switching tabs
        connect(chart, &ChartWidget::coordinateSelected, this, &MainWindow::onCoordinateSelected,
                Qt::UniqueConnection);
        connect(chart, &ChartWidget::seriesRemoved, this, &MainWindow::onSeriesRemoved,
                Qt::UniqueConnection);
        connect(chart, &ChartWidget::seriesDropped, this, &MainWindow::onSeriesDropped,
                Qt::UniqueConnection);
    };

    connectChartSignals(m_canvasArea->currentCanvas());
    connect(m_canvasArea, &CanvasTabWidget::currentChanged, [this, connectChartSignals](int index) {
        ChartWidget* chart = m_canvasArea->canvasAt(index);
        if (chart) {
            connectChartSignals(chart);
        }
    });

    connect(&ConfigManager::instance(), &ConfigManager::configsChanged,
            m_favorPanel, &FavorPanel::refreshConfigs);
    connect(m_favorPanel, &FavorPanel::applyConfigRequested,
            this, &MainWindow::onApplyConfig);

    m_closeFileAction->setEnabled(false);
    m_closeAllAction->setEnabled(false);
    m_exportBinaryAction->setEnabled(false);
}

MainWindow::~MainWindow() {
}

void MainWindow::loadFile(const QString& filePath) {
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists()) {
        QMessageBox::critical(this, "Error", QString("File not found: %1").arg(filePath));
        return;
    }

    QString error;
    if (fileInfo.suffix().toLower() == "bin") {
        m_dataModel->loadBinary(filePath, error);
    } else {
        m_dataModel->loadCSV(filePath, error);
    }

    if (!error.isEmpty()) {
        QMessageBox::critical(this, "Error", error);
    }
}

void MainWindow::createActions() {
    m_openCSVAction = new QAction("Open CSV...", this);
    m_openCSVAction->setShortcut(QKeySequence::Open);
    m_openCSVAction->setStatusTip("Open a CSV log file");
    connect(m_openCSVAction, &QAction::triggered, this, &MainWindow::onOpenCSV);

    m_openBinaryAction = new QAction("Open Binary...", this);
    m_openBinaryAction->setStatusTip("Open a binary log file");
    connect(m_openBinaryAction, &QAction::triggered, this, &MainWindow::onOpenBinary);

    m_closeFileAction = new QAction("Close All Files", this);
    m_closeFileAction->setShortcut(QKeySequence::Close);
    m_closeFileAction->setStatusTip("Close all loaded files");
    connect(m_closeFileAction, &QAction::triggered, this, &MainWindow::onCloseAllFiles);

    m_closeAllAction = new QAction("Close All Files", this);
    m_closeAllAction->setStatusTip("Close all loaded files");
    connect(m_closeAllAction, &QAction::triggered, this, &MainWindow::onCloseAllFiles);

    m_exportBinaryAction = new QAction("Export Binary...", this);
    m_exportBinaryAction->setStatusTip("Export data to binary format");
    connect(m_exportBinaryAction, &QAction::triggered, this, &MainWindow::onExportBinary);

    m_exitAction = new QAction("Exit", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip("Exit the application");
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::onExit);

    m_newCanvasAction = new QAction("New Canvas", this);
    m_newCanvasAction->setShortcut(QKeySequence::New);
    m_newCanvasAction->setStatusTip("Create a new canvas");
    connect(m_newCanvasAction, &QAction::triggered, this, &MainWindow::onCreateNewCanvas);

    m_destroyCanvasAction = new QAction("Close Canvas", this);
    m_destroyCanvasAction->setStatusTip("Close the current canvas");
    connect(m_destroyCanvasAction, &QAction::triggered, this, &MainWindow::onDestroyCanvas);

    m_renameCanvasAction = new QAction("Rename Canvas", this);
    m_renameCanvasAction->setShortcut(QKeySequence("F2"));
    m_renameCanvasAction->setStatusTip("Rename the current canvas");
    connect(m_renameCanvasAction, &QAction::triggered, this, &MainWindow::onRenameCanvas);

    m_saveCurveConfigAction = new QAction(this);
    m_saveCurveConfigAction->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    m_saveCurveConfigAction->setText("Save Config");
    m_saveCurveConfigAction->setToolTip("Save current canvas curves as a named config");
    m_saveCurveConfigAction->setStatusTip("Save current canvas curves as a named config");
    connect(m_saveCurveConfigAction, &QAction::triggered, this, &MainWindow::onSaveCurveConfig);

    m_aboutAction = new QAction("About", this);
    m_aboutAction->setStatusTip("Show information about this application");
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::createMenus() {
    QMenu* fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(m_openCSVAction);
    fileMenu->addAction(m_openBinaryAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_closeAllAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exportBinaryAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    QMenu* viewMenu = menuBar()->addMenu("View");
    viewMenu->addAction(m_newCanvasAction);
    viewMenu->addAction(m_destroyCanvasAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_renameCanvasAction);

    QMenu* helpMenu = menuBar()->addMenu("Help");
    helpMenu->addAction(m_aboutAction);
}

void MainWindow::createToolBar() {
    QToolBar* toolBar = addToolBar("Main");
    toolBar->setMovable(false);

    toolBar->addAction(m_openCSVAction);
    toolBar->addAction(m_openBinaryAction);
    toolBar->addSeparator();
    toolBar->addAction(m_closeAllAction);
    toolBar->addSeparator();
    toolBar->addAction(m_newCanvasAction);
    toolBar->addAction(m_destroyCanvasAction);
}

void MainWindow::createStatusBar() {
    m_statusBar = statusBar();

    m_coordinateLabel = new QLabel("Position: -", this);
    m_coordinateLabel->setMinimumWidth(150);

    m_fileInfoLabel = new QLabel("No file loaded", this);

    m_statusBar->addWidget(m_fileInfoLabel);
    m_statusBar->addPermanentWidget(m_coordinateLabel);
}

void MainWindow::setupLayout() {
    // Left side: series tree (top) + saved configs panel (bottom)
    m_leftSplitter->addWidget(m_leftPane);
    m_leftSplitter->addWidget(m_favorPanel);
    m_leftSplitter->setSizes({550, 180});
    m_leftSplitter->setStretchFactor(0, 1);
    m_leftSplitter->setStretchFactor(1, 0);

    m_splitter->addWidget(m_leftSplitter);
    m_splitter->addWidget(m_canvasArea);
    m_splitter->setSizes({250, 950});
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);

    setCentralWidget(m_splitter);
}

void MainWindow::onOpenCSV() {
    QString fileName = getOpenCSVFileName();
    if (fileName.isEmpty()) {
        return;
    }

    QString error;
    if (m_dataModel->getFileCount() == 0) {
        m_dataModel->loadCSV(fileName, error);
    } else {
        m_dataModel->addCSV(fileName, error);
    }

    if (!error.isEmpty()) {
        QMessageBox::critical(this, "Error", error);
    }
}

void MainWindow::onOpenBinary() {
    QString fileName = getOpenBinaryFileName();
    if (fileName.isEmpty()) {
        return;
    }

    QString error;
    if (m_dataModel->getFileCount() == 0) {
        m_dataModel->loadBinary(fileName, error);
    } else {
        m_dataModel->addBinary(fileName, error);
    }

    if (!error.isEmpty()) {
        QMessageBox::critical(this, "Error", error);
    }
}

void MainWindow::onCloseFile() {
    m_dataModel->clear();
}

void MainWindow::onCloseSpecificFile(const QString& filePath) {
    m_dataModel->closeFile(filePath);
}

void MainWindow::onCloseAllFiles() {
    m_dataModel->clear();
}

void MainWindow::onExportBinary() {
    QString fileName = getSaveBinaryFileName();
    if (fileName.isEmpty()) {
        return;
    }

    QString error;
    if (!DataParser::exportBinary(m_dataModel->getAllSeries(), fileName, error)) {
        QMessageBox::critical(this, "Error", error);
    } else {
        QMessageBox::information(this, "Success", "Data exported successfully!");
    }
}

void MainWindow::onExit() {
    close();
}

void MainWindow::onCreateNewCanvas() {
    m_canvasArea->createNewCanvas();
}

void MainWindow::onDestroyCanvas() {
    m_canvasArea->destroyCurrentCanvas();
}

void MainWindow::onRenameCanvas() {
    m_canvasArea->renameCurrentCanvas();
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About Log Viewer",
        "<h3>Log Viewer</h3>"
        "<p>A tool for visualizing log data as curves.</p>"
        "<p>Features:</p>"
        "<ul>"
        "<li>Load multiple CSV and binary log files</li>"
        "<li>Visualize data by category</li>"
        "<li>Multiple canvas tabs</li>"
        "<li>Click to view coordinates</li>"
        "<li>Zoom and pan functionality</li>"
        "</ul>"
        "<p>Version 1.1.0</p>");
}

void MainWindow::onDataLoaded(const QString& filePath) {
    Q_UNUSED(filePath);
    updateFileInfoLabel();

    m_closeFileAction->setEnabled(true);
    m_closeAllAction->setEnabled(true);
    m_exportBinaryAction->setEnabled(true);
}

void MainWindow::onFileAdded(const QString& filePath) {
    Q_UNUSED(filePath);
    updateFileInfoLabel();

    m_closeFileAction->setEnabled(true);
    m_closeAllAction->setEnabled(true);
    m_exportBinaryAction->setEnabled(true);

    // populateTree() was called by LeftPane (clears all checkboxes); re-sync with current canvas
    ChartWidget* canvas = m_canvasArea->currentCanvas();
    if (canvas) {
        m_leftPane->syncSelectionWithSeries(canvas->getSeriesNames());
    }
}

void MainWindow::onFileRemoved(const QString& filePath) {
    updateFileInfoLabel();

    bool hasFiles = m_dataModel->getFileCount() > 0;
    m_closeFileAction->setEnabled(hasFiles);
    m_closeAllAction->setEnabled(hasFiles);
    m_exportBinaryAction->setEnabled(hasFiles);

    // Remove series from ALL canvases belonging to the removed file
    const QString prefix = filePath + QChar('\x1E');
    for (int i = 0; i < m_canvasArea->canvasCount(); ++i) {
        ChartWidget* canvas = m_canvasArea->canvasAt(i);
        if (!canvas) continue;
        const QStringList compoundKeys = canvas->getSeriesNames();
        for (const QString& key : compoundKeys) {
            if (key.startsWith(prefix)) {
                canvas->removeSeries(key);
            }
        }
    }

    // populateTree() was called by LeftPane (clears all checkboxes); re-sync with current canvas
    ChartWidget* canvas = m_canvasArea->currentCanvas();
    if (canvas) {
        m_leftPane->syncSelectionWithSeries(canvas->getSeriesNames());
    }
}

void MainWindow::onDataCleared() {
    m_fileInfoLabel->setText("No file loaded");
    m_coordinateLabel->setText("Position: -");

    m_closeFileAction->setEnabled(false);
    m_closeAllAction->setEnabled(false);
    m_exportBinaryAction->setEnabled(false);

    m_leftPane->clearSelection();

    // Clear ALL canvases, not just the current one
    for (int i = 0; i < m_canvasArea->canvasCount(); ++i) {
        ChartWidget* canvas = m_canvasArea->canvasAt(i);
        if (canvas) {
            canvas->clearAll();
        }
    }
}

void MainWindow::onDataError(const QString& error) {
    QMessageBox::critical(this, "Error", error);
}

void MainWindow::onSeriesSelected(const QStringList& seriesNames) {
    qDebug() << "MainWindow::onSeriesSelected:" << seriesNames;
    for (const QString& name : seriesNames) {
        qDebug() << "  Adding series to canvas:" << name;
        m_canvasArea->addSeriesToCurrentCanvas(name);
    }
}

void MainWindow::onSeriesDeselected(const QStringList& seriesNames) {
    for (const QString& name : seriesNames) {
        m_canvasArea->removeSeriesFromCurrentCanvas(name);
    }
}

void MainWindow::onAddToCanvasRequested(const QStringList& seriesNames) {
    for (const QString& name : seriesNames) {
        m_canvasArea->addSeriesToCurrentCanvas(name);
    }
}

void MainWindow::onRemoveFromCanvasRequested(const QStringList& seriesNames) {
    for (const QString& name : seriesNames) {
        m_canvasArea->removeSeriesFromCurrentCanvas(name);
    }
}

void MainWindow::onCoordinateSelected(const QString& seriesName, double x, double y) {
    m_coordinateLabel->setText(QString("Position: %1 | (%2, %3)")
        .arg(seriesName)
        .arg(x, 0, 'f', 4)
        .arg(y, 0, 'f', 4));
}

void MainWindow::onSeriesDropped(const QString& seriesName) {
    m_leftPane->setSeriesSelected(seriesName, true);
}

void MainWindow::onSeriesRemoved(const QString& name) {
    // Only uncheck if the current canvas no longer has this series.
    // A non-current canvas may emit seriesRemoved while the current canvas still shows it.
    ChartWidget* current = m_canvasArea->currentCanvas();
    if (!current || !current->getSeriesNames().contains(name)) {
        m_leftPane->setSeriesSelected(name, false);
    }
}

void MainWindow::onCurrentCanvasChanged(int index) {
    ChartWidget* canvas = m_canvasArea->canvasAt(index);
    if (canvas) {
        QStringList seriesNames = canvas->getSeriesNames();
        m_leftPane->syncSelectionWithSeries(seriesNames);
    }
}

void MainWindow::onCanvasCreated(const QString& name) {
    Q_UNUSED(name);
    m_leftPane->syncSelectionWithSeries(QStringList());
}

void MainWindow::onCanvasDestroyed(int index) {
    Q_UNUSED(index);
}

QString MainWindow::getOpenCSVFileName() {
    return QFileDialog::getOpenFileName(
        this,
        "Open CSV File",
        QString(),
        "CSV Files (*.csv *.txt);;All Files (*)"
    );
}

QString MainWindow::getOpenBinaryFileName() {
    return QFileDialog::getOpenFileName(
        this,
        "Open Binary File",
        QString(),
        "Binary Files (*.bin);;All Files (*)"
    );
}

QString MainWindow::getSaveBinaryFileName() {
    return QFileDialog::getSaveFileName(
        this,
        "Export Binary File",
        QString(),
        "Binary Files (*.bin);;All Files (*)"
    );
}

void MainWindow::updateFileInfoLabel() {
    int fileCount = m_dataModel->getFileCount();
    int seriesCount = m_dataModel->getSeriesCount();
    int categoryCount = m_dataModel->getCategories().size();

    if (fileCount == 0) {
        m_fileInfoLabel->setText("No file loaded");
    } else if (fileCount == 1) {
        QString fileName = QFileInfo(m_dataModel->getFileNames().first()).fileName();
        m_fileInfoLabel->setText(QString("File: %1 | Series: %2 | Categories: %3")
            .arg(fileName)
            .arg(seriesCount)
            .arg(categoryCount));
    } else {
        m_fileInfoLabel->setText(QString("Files: %1 | Series: %2 | Categories: %3")
            .arg(fileCount)
            .arg(seriesCount)
            .arg(categoryCount));
    }
}

void MainWindow::onSaveCurveConfig() {
    ChartWidget* canvas = m_canvasArea->currentCanvas();
    if (!canvas) return;

    QStringList compoundKeys = canvas->getSeriesNames();
    if (compoundKeys.isEmpty()) {
        QMessageBox::information(this, "Empty Canvas",
                                 "Current canvas has no curves to save.");
        return;
    }

    // Extract display names from compound keys
    QStringList displayNames;
    for (const QString& key : compoundKeys) {
        int sep = key.indexOf('\x1E');
        displayNames.append(sep >= 0 ? key.mid(sep + 1) : key);
    }

    bool ok;
    QString name = QInputDialog::getText(this, "Save Config",
                                         "Config name:", QLineEdit::Normal,
                                         QString(), &ok);
    if (!ok || name.trimmed().isEmpty()) return;

    CurveConfig cfg;
    cfg.name = name.trimmed();
    cfg.seriesNames = displayNames;
    ConfigManager::instance().saveConfig(cfg);
}

void MainWindow::onApplyConfig(const QString& configName) {
    CurveConfig cfg = ConfigManager::instance().getConfig(configName);
    if (cfg.name.isEmpty() || cfg.seriesNames.isEmpty()) return;

    if (m_dataModel->getFileCount() == 0) {
        QMessageBox::warning(this, "No Data",
                             "Please load a data file first.");
        return;
    }

    // Build set of target display names for fast lookup
    QSet<QString> targets = QSet<QString>::fromList(cfg.seriesNames);

    // Collect matching compound keys across all loaded files
    QStringList toAdd;
    for (const QString& filePath : m_dataModel->getFileNames()) {
        for (const SeriesData& series : m_dataModel->getSeriesForFile(filePath)) {
            if (targets.contains(series.name)) {
                toAdd.append(filePath + QChar('\x1E') + series.name);
            }
        }
    }

    if (toAdd.isEmpty()) {
        QMessageBox::information(this, "No Match",
            QString("None of the variables in config \"%1\" were found "
                    "in the currently loaded data.").arg(configName));
        return;
    }

    // Create a new canvas named after the config and populate it
    m_canvasArea->createNewCanvas(cfg.name);
    for (const QString& key : toAdd) {
        m_canvasArea->addSeriesToCurrentCanvas(key);
    }

    // Sync left-pane checkboxes with the new canvas
    ChartWidget* newCanvas = m_canvasArea->currentCanvas();
    if (newCanvas) {
        m_leftPane->syncSelectionWithSeries(newCanvas->getSeriesNames());
    }
}

} // namespace Viewer
