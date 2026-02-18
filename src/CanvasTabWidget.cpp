#include "CanvasTabWidget.h"
#include "DataModel.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

namespace Viewer {

CanvasTabWidget::CanvasTabWidget(QWidget* parent)
    : QWidget(parent) {

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setTabBarAutoHide(false);

    m_newCanvasButton = new QPushButton("New Canvas", this);
    m_newCanvasButton->setMaximumWidth(100);

    m_selectedLabel = new QLabel("Selected: None", this);
    
    m_pointShapeLabel = new QLabel("Point:", this);
    m_pointShapeCombo = new QComboBox(this);
    m_pointShapeCombo->addItem("None", static_cast<int>(PointShape::None));
    m_pointShapeCombo->addItem("Circle", static_cast<int>(PointShape::Circle));
    m_pointShapeCombo->addItem("Square", static_cast<int>(PointShape::Square));
    m_pointShapeCombo->addItem("Triangle", static_cast<int>(PointShape::Triangle));
    m_pointShapeCombo->addItem("Star", static_cast<int>(PointShape::Star));
    m_pointShapeCombo->setCurrentIndex(1);
    m_pointShapeCombo->setMaximumWidth(80);

    m_pointSizeLabel = new QLabel("Size:", this);
    m_pointSizeSpin = new QSpinBox(this);
    m_pointSizeSpin->setRange(1, 20);
    m_pointSizeSpin->setValue(8);
    m_pointSizeSpin->setMaximumWidth(50);

    m_lineStyleLabel = new QLabel("Line:", this);
    m_lineStyleCombo = new QComboBox(this);
    m_lineStyleCombo->addItem("Solid", static_cast<int>(LineStyle::Solid));
    m_lineStyleCombo->addItem("Dash", static_cast<int>(LineStyle::Dash));
    m_lineStyleCombo->addItem("Dot", static_cast<int>(LineStyle::Dot));
    m_lineStyleCombo->addItem("DashDot", static_cast<int>(LineStyle::DashDot));
    m_lineStyleCombo->setMaximumWidth(80);

    m_lineWidthLabel = new QLabel("Width:", this);
    m_lineWidthSpin = new QSpinBox(this);
    m_lineWidthSpin->setRange(1, 10);
    m_lineWidthSpin->setValue(3);
    m_lineWidthSpin->setMaximumWidth(50);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_newCanvasButton);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(m_selectedLabel);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(m_pointShapeLabel);
    buttonLayout->addWidget(m_pointShapeCombo);
    buttonLayout->addWidget(m_pointSizeLabel);
    buttonLayout->addWidget(m_pointSizeSpin);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(m_lineStyleLabel);
    buttonLayout->addWidget(m_lineStyleCombo);
    buttonLayout->addWidget(m_lineWidthLabel);
    buttonLayout->addWidget(m_lineWidthSpin);
    buttonLayout->addStretch();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(m_tabWidget);

    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &CanvasTabWidget::onTabCloseRequested);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &CanvasTabWidget::onCurrentChanged);
    connect(m_newCanvasButton, &QPushButton::clicked, this, &CanvasTabWidget::onCreateNewCanvasClicked);
    connect(m_pointShapeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CanvasTabWidget::onPointShapeChanged);
    connect(m_pointSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &CanvasTabWidget::onPointSizeChanged);
    connect(m_lineStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CanvasTabWidget::onLineStyleChanged);
    connect(m_lineWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &CanvasTabWidget::onLineWidthChanged);

    createNewCanvas();
}

void CanvasTabWidget::setDataModel(DataModel* model) {
    if (m_dataModel) {
        disconnect(m_dataModel, nullptr, this, nullptr);
    }

    m_dataModel = model;

    if (m_dataModel) {
        connect(m_dataModel, &DataModel::dataLoaded, this, &CanvasTabWidget::onDataLoaded);
    }
}

void CanvasTabWidget::createNewCanvas(const QString& name) {
    QString canvasName = name.isEmpty() ? generateCanvasName() : name;

    ChartWidget* chart = new ChartWidget(this);
    chart->setTitle(canvasName);
    chart->setDataModel(m_dataModel);

    connectCanvasSignals(chart);

    int index = m_tabWidget->addTab(chart, canvasName);
    m_tabWidget->setCurrentIndex(index);

    m_canvasCounter++;

    emit canvasCreated(canvasName);
}

void CanvasTabWidget::destroyCurrentCanvas() {
    int index = m_tabWidget->currentIndex();
    if (index >= 0) {
        destroyCanvas(index);
    }
}

void CanvasTabWidget::destroyCanvas(int index) {
    if (index < 0 || index >= m_tabWidget->count()) {
        return;
    }

    if (m_tabWidget->count() <= 1) {
        QMessageBox::information(this, "Info", "Cannot remove the last canvas.");
        return;
    }

    QWidget* widget = m_tabWidget->widget(index);
    m_tabWidget->removeTab(index);
    delete widget;

    emit canvasDestroyed(index);
}

void CanvasTabWidget::renameCurrentCanvas() {
    int index = m_tabWidget->currentIndex();
    if (index >= 0) {
        bool ok;
        QString newName = QInputDialog::getText(
            this,
            "Rename Canvas",
            "Enter new name:",
            QLineEdit::Normal,
            m_tabWidget->tabText(index),
            &ok
        );

        if (ok && !newName.isEmpty()) {
            renameCanvas(index, newName);
        }
    }
}

void CanvasTabWidget::renameCanvas(int index, const QString& newName) {
    if (index >= 0 && index < m_tabWidget->count()) {
        m_tabWidget->setTabText(index, newName);

        ChartWidget* canvas = canvasAt(index);
        if (canvas) {
            canvas->setTitle(newName);
        }

        emit canvasRenamed(index, newName);
    }
}

ChartWidget* CanvasTabWidget::currentCanvas() const {
    int index = m_tabWidget->currentIndex();
    return canvasAt(index);
}

ChartWidget* CanvasTabWidget::canvasAt(int index) const {
    if (index >= 0 && index < m_tabWidget->count()) {
        return qobject_cast<ChartWidget*>(m_tabWidget->widget(index));
    }
    return nullptr;
}

int CanvasTabWidget::canvasCount() const {
    return m_tabWidget->count();
}

void CanvasTabWidget::addSeriesToCurrentCanvas(const QString& seriesName, const QColor& color) {
    if (!m_dataModel) {
        return;
    }

    ChartWidget* canvas = currentCanvas();
    if (!canvas) {
        return;
    }

    SeriesData seriesData = m_dataModel->getSeriesByName(seriesName);

    if (!seriesData.isEmpty()) {
        QColor seriesColor = color.isValid() ? color : getSeriesColor(seriesName);
        canvas->addSeries(seriesData, seriesColor);
    }
}

void CanvasTabWidget::removeSeriesFromCurrentCanvas(const QString& seriesName) {
    ChartWidget* canvas = currentCanvas();
    if (canvas) {
        canvas->removeSeries(seriesName);
    }
}

void CanvasTabWidget::clearCurrentCanvas() {
    ChartWidget* canvas = currentCanvas();
    if (canvas) {
        canvas->clearAll();
    }
}

void CanvasTabWidget::onTabCloseRequested(int index) {
    ChartWidget* canvas = canvasAt(index);
    if (canvas) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Close Canvas",
            QString("Are you sure you want to close '%1'?").arg(m_tabWidget->tabText(index)),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            destroyCanvas(index);
        }
    }
}

void CanvasTabWidget::onCurrentChanged(int index) {
    ChartWidget* canvas = canvasAt(index);
    if (canvas) {
        disconnectCanvasSignals(canvas);
        connectCanvasSignals(canvas);
        updateControlsFromSelection();
    }
    emit currentChanged(index);
}

void CanvasTabWidget::onCreateNewCanvasClicked() {
    createNewCanvas();
}

void CanvasTabWidget::onPointShapeChanged(int index) {
    ChartWidget* canvas = currentCanvas();
    QString selected = canvas ? canvas->selectedSeries() : QString();
    
    if (canvas && !selected.isEmpty()) {
        PointShape shape = static_cast<PointShape>(m_pointShapeCombo->itemData(index).toInt());
        canvas->setSeriesPointShape(selected, shape);
    }
}

void CanvasTabWidget::onPointSizeChanged(int value) {
    ChartWidget* canvas = currentCanvas();
    QString selected = canvas ? canvas->selectedSeries() : QString();
    
    if (canvas && !selected.isEmpty()) {
        canvas->setSeriesPointSize(selected, value);
    }
}

void CanvasTabWidget::onLineStyleChanged(int index) {
    ChartWidget* canvas = currentCanvas();
    QString selected = canvas ? canvas->selectedSeries() : QString();
    
    if (canvas && !selected.isEmpty()) {
        LineStyle style = static_cast<LineStyle>(m_lineStyleCombo->itemData(index).toInt());
        canvas->setSeriesLineStyle(selected, style);
    }
}

void CanvasTabWidget::onLineWidthChanged(int value) {
    ChartWidget* canvas = currentCanvas();
    QString selected = canvas ? canvas->selectedSeries() : QString();
    
    if (canvas && !selected.isEmpty()) {
        canvas->setSeriesLineWidth(selected, value);
    }
}

void CanvasTabWidget::onSeriesSelected(const QString& name) {
    m_selectedLabel->setText(name.isEmpty() ? "Selected: None" : QString("Selected: %1").arg(name));
    updateControlsFromSelection();
}

void CanvasTabWidget::onDataLoaded(const QString& filePath) {
    Q_UNUSED(filePath);
}

QString CanvasTabWidget::generateCanvasName() const {
    return QString("Canvas %1").arg(m_canvasCounter);
}

QColor CanvasTabWidget::getSeriesColor(const QString& seriesName) const {
    static const QList<QColor> colorPalette = {
        QColor(41, 128, 185),
        QColor(192, 57, 43),
        QColor(39, 174, 96),
        QColor(243, 156, 18),
        QColor(142, 68, 173),
        QColor(44, 62, 80),
        QColor(22, 160, 133),
        QColor(211, 84, 0),
        QColor(127, 140, 141),
        QColor(52, 152, 219),
        QColor(231, 76, 60),
        QColor(46, 204, 113),
    };

    int index = qHash(seriesName) % colorPalette.size();
    if (index < 0) index = -index;
    return colorPalette[index];
}

void CanvasTabWidget::updateControlsFromSelection() {
    ChartWidget* canvas = currentCanvas();
    QString selected = canvas ? canvas->selectedSeries() : QString();
    
    m_selectedLabel->setText(selected.isEmpty() ? "Selected: None" : QString("Selected: %1").arg(selected));
    
    if (!selected.isEmpty() && canvas) {
        SeriesStyle style = canvas->getSeriesStyle(selected);
        
        m_pointShapeCombo->blockSignals(true);
        m_pointShapeCombo->setCurrentIndex(static_cast<int>(style.pointShape));
        m_pointShapeCombo->blockSignals(false);
        
        m_pointSizeSpin->blockSignals(true);
        m_pointSizeSpin->setValue(style.pointSize);
        m_pointSizeSpin->blockSignals(false);
        
        m_lineStyleCombo->blockSignals(true);
        m_lineStyleCombo->setCurrentIndex(static_cast<int>(style.lineStyle));
        m_lineStyleCombo->blockSignals(false);
        
        m_lineWidthSpin->blockSignals(true);
        m_lineWidthSpin->setValue(style.lineWidth);
        m_lineWidthSpin->blockSignals(false);
    }
}

void CanvasTabWidget::disconnectCanvasSignals(ChartWidget* canvas) {
    if (canvas) {
        disconnect(canvas, &ChartWidget::seriesDropped, this, nullptr);
        disconnect(canvas, &ChartWidget::seriesSelected, this, nullptr);
    }
}

void CanvasTabWidget::connectCanvasSignals(ChartWidget* canvas) {
    if (canvas) {
        connect(canvas, &ChartWidget::seriesDropped, this, [this](const QString& seriesName) {
            addSeriesToCurrentCanvas(seriesName);
        });
        connect(canvas, &ChartWidget::seriesSelected, this, &CanvasTabWidget::onSeriesSelected);
    }
}

} // namespace Viewer
