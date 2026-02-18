#ifndef CANVASTABWIDGET_H
#define CANVASTABWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include "ChartWidget.h"
#include "DataModel.h"
#include "models/SeriesData.h"

namespace Viewer {

class CanvasTabWidget : public QWidget {
    Q_OBJECT

public:
    explicit CanvasTabWidget(QWidget* parent = nullptr);

    void setDataModel(DataModel* model);

    void createNewCanvas(const QString& name = QString());
    void destroyCurrentCanvas();
    void destroyCanvas(int index);
    void renameCurrentCanvas();
    void renameCanvas(int index, const QString& newName);

    ChartWidget* currentCanvas() const;
    ChartWidget* canvasAt(int index) const;
    int canvasCount() const;

    void addSeriesToCurrentCanvas(const QString& seriesName, const QColor& color = QColor());
    void removeSeriesFromCurrentCanvas(const QString& seriesName);
    void clearCurrentCanvas();

signals:
    void canvasCreated(const QString& name);
    void canvasDestroyed(int index);
    void canvasRenamed(int index, const QString& newName);
    void currentChanged(int index);

private slots:
    void onTabCloseRequested(int index);
    void onCurrentChanged(int index);
    void onCreateNewCanvasClicked();
    void onPointShapeChanged(int index);
    void onPointSizeChanged(int value);
    void onLineStyleChanged(int index);
    void onLineWidthChanged(int value);
    void onSeriesSelected(const QString& name);

    void onDataLoaded(const QString& filePath);

private:
    QString generateCanvasName() const;
    QColor getSeriesColor(const QString& seriesName) const;
    void updateControlsFromSelection();
    void disconnectCanvasSignals(ChartWidget* canvas);
    void connectCanvasSignals(ChartWidget* canvas);

    QTabWidget* m_tabWidget;
    QPushButton* m_newCanvasButton;
    
    QLabel* m_selectedLabel;
    QLabel* m_pointShapeLabel;
    QComboBox* m_pointShapeCombo;
    QLabel* m_pointSizeLabel;
    QSpinBox* m_pointSizeSpin;
    QLabel* m_lineStyleLabel;
    QComboBox* m_lineStyleCombo;
    QLabel* m_lineWidthLabel;
    QSpinBox* m_lineWidthSpin;
    
    DataModel* m_dataModel = nullptr;

    int m_canvasCounter = 1;
};

} // namespace Viewer

#endif // CANVASTABWIDGET_H
