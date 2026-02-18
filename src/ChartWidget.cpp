#include "ChartWidget.h"
#include <QPainter>
#include <QPixmap>
#include <QPolygonF>
#include <QBrush>
#include <QResizeEvent>
#include <QShowEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsLayout>
#include <QtMath>
#include <QWheelEvent>
#include <QDebug>
#include <QMimeData>
#include <QtCharts/QLegendMarker>

QT_CHARTS_USE_NAMESPACE

namespace Viewer {

FitButton::FitButton(QWidget* parent)
    : QWidget(parent) {
    setFixedSize(18, 18);
    setCursor(Qt::PointingHandCursor);
    setToolTip("Fit curves to view");
}

void FitButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bgColor = m_hovered ? QColor(220, 220, 220) : QColor(255, 255, 255, 200);
    painter.fillRect(rect(), bgColor);
    painter.setPen(QPen(QColor(150, 150, 150), 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));

    QPen pen(QColor(80, 80, 80), 1.5);
    painter.setPen(pen);

    int margin = 2;
    int cornerLen = 4;
    int w = width();
    int h = height();

    painter.drawLine(margin, margin, margin + cornerLen, margin);
    painter.drawLine(margin, margin, margin, margin + cornerLen);

    painter.drawLine(w - margin - cornerLen, margin, w - margin, margin);
    painter.drawLine(w - margin, margin, w - margin, margin + cornerLen);

    painter.drawLine(margin, h - margin, margin + cornerLen, h - margin);
    painter.drawLine(margin, h - margin - cornerLen, margin, h - margin);

    painter.drawLine(w - margin - cornerLen, h - margin, w - margin, h - margin);
    painter.drawLine(w - margin, h - margin - cornerLen, w - margin, h - margin);
}

void FitButton::mousePressEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    emit clicked();
    update();
}

void FitButton::enterEvent(QEvent* event) {
    Q_UNUSED(event);
    m_hovered = true;
    update();
}

void FitButton::leaveEvent(QEvent* event) {
    Q_UNUSED(event);
    m_hovered = false;
    update();
}

CoordinateOverlay::CoordinateOverlay(QGraphicsItem* parent)
    : QGraphicsItem(parent) {
    setZValue(1000);
}

QRectF CoordinateOverlay::boundingRect() const {
    return QRectF(0, 0, 200, 30);
}

void CoordinateOverlay::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!m_visible) {
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing);

    QRectF rect = boundingRect();
    painter->fillRect(rect, QColor(0, 0, 0, 180));

    painter->setPen(QPen(QColor(100, 200, 255), 1));
    painter->drawRect(rect);

    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setPointSize(9);
    painter->setFont(font);
    painter->drawText(rect.adjusted(5, 2, -5, -2), Qt::AlignLeft | Qt::AlignVCenter, m_text);
}

void CoordinateOverlay::setText(const QString& text) {
    m_text = text;
    update();
}

void CoordinateOverlay::setVisible(bool visible) {
    m_visible = visible;
    update();
}

CustomChartView::CustomChartView(QChart* chart, QWidget* parent)
    : QChartView(chart, parent) {
    setRenderHint(QPainter::Antialiasing);
    setAcceptDrops(true);
}

void CustomChartView::setWheelZoomEnabled(bool enabled) {
    m_wheelZoomEnabled = enabled;
}

void CustomChartView::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasFormat("application/x-series-name")) {
        event->acceptProposedAction();
    }
}

void CustomChartView::dragMoveEvent(QDragMoveEvent* event) {
    if (event->mimeData()->hasFormat("application/x-series-name")) {
        event->acceptProposedAction();
    }
}

void CustomChartView::dropEvent(QDropEvent* event) {
    QString seriesName = QString::fromUtf8(event->mimeData()->data("application/x-series-name"));
    if (!seriesName.isEmpty()) {
        emit seriesDropped(seriesName);
    }
    event->acceptProposedAction();
}

void CustomChartView::wheelEvent(QWheelEvent* event) {
    if (!m_wheelZoomEnabled) {
        QChartView::wheelEvent(event);
        return;
    }

    double zoomFactor = event->angleDelta().y() > 0 ? 0.9 : 1.1;
    QRectF rect = chart()->plotArea();

    if (m_xKeyPressed) {
        double newWidth = rect.width() * zoomFactor;
        double newLeft = rect.left() - (newWidth - rect.width()) / 2;
        QRectF newRect(newLeft, rect.top(), newWidth, rect.height());
        chart()->zoomIn(newRect);
        event->accept();
    } else if (m_vKeyPressed) {
        double newHeight = rect.height() * zoomFactor;
        double newTop = rect.top() - (newHeight - rect.height()) / 2;
        QRectF newRect(rect.left(), newTop, rect.width(), newHeight);
        chart()->zoomIn(newRect);
        event->accept();
    } else {
        QPointF chartPos = mapToScene(event->pos());
        double newWidth = rect.width() * zoomFactor;
        double newHeight = rect.height() * zoomFactor;
        double newLeft = chartPos.x() - (chartPos.x() - rect.left()) * zoomFactor;
        double newTop = chartPos.y() - (chartPos.y() - rect.top()) * zoomFactor;
        QRectF newRect(newLeft, newTop, newWidth, newHeight);
        chart()->zoomIn(newRect);
        event->accept();
    }
}

void CustomChartView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_X) {
        m_xKeyPressed = true;
    } else if (event->key() == Qt::Key_V) {
        m_vKeyPressed = true;
    }
    QChartView::keyPressEvent(event);
}

void CustomChartView::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_X) {
        m_xKeyPressed = false;
    } else if (event->key() == Qt::Key_V) {
        m_vKeyPressed = false;
    }
    QChartView::keyReleaseEvent(event);
}

void CustomChartView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        m_midButtonPressed = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else if (event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());
        QPointF chartPos = chart()->mapToValue(chart()->mapFromScene(scenePos));
        emit pointClicked(chartPos);
    }
    QChartView::mousePressEvent(event);
}

void CustomChartView::mouseMoveEvent(QMouseEvent* event) {
    if (m_midButtonPressed) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();
        
        chart()->scroll(-delta.x(), delta.y());
        event->accept();
    } else {
        QChartView::mouseMoveEvent(event);
    }
}

void CustomChartView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        m_midButtonPressed = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QChartView::mouseReleaseEvent(event);
    }
}

ChartWidget::ChartWidget(QWidget* parent)
    : QWidget(parent) {

    m_chart = new QChart();

    m_chart->setBackgroundBrush(QBrush(QColor(240, 240, 245)));
    m_chart->setPlotAreaBackgroundBrush(QBrush(Qt::white));
    m_chart->setPlotAreaBackgroundVisible(true);
    m_chart->setMargins(QMargins(10, 10, 10, 10));

    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);

    m_axisX = new QValueAxis();
    m_axisX->setTitleText("Time");
    m_axisX->setLabelsVisible(true);
    m_axisX->setGridLineVisible(true);
    m_axisX->setGridLinePen(QPen(QColor(200, 200, 200), 1, Qt::DashLine));
    m_axisX->setMinorGridLineVisible(false);
    m_axisX->setLabelFormat("%.1f");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    m_axisY = new QValueAxis();
    m_axisY->setTitleText("Value");
    m_axisY->setLabelsVisible(true);
    m_axisY->setGridLineVisible(true);
    m_axisY->setGridLinePen(QPen(QColor(200, 200, 200), 1, Qt::DashLine));
    m_axisY->setMinorGridLineVisible(false);
    m_axisY->setLabelFormat("%.1f");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_axisX->setRange(0, 10);
    m_axisY->setRange(0, 100);
    m_axisX->setTickCount(11);
    m_axisY->setTickCount(11);

    m_chartView = new CustomChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setRubberBand(QChartView::RectangleRubberBand);
    m_chartView->setDragMode(QGraphicsView::ScrollHandDrag);
    m_chartView->setWheelZoomEnabled(true);
    m_chartView->setFocusPolicy(Qt::StrongFocus);
    connect(m_chartView, &CustomChartView::seriesDropped, this, &ChartWidget::seriesDropped);
    connect(m_chartView, &CustomChartView::pointClicked, this, &ChartWidget::onChartClicked);

    m_coordinateOverlay = new CoordinateOverlay(m_chart);
    m_coordinateOverlay->setPos(10, 10);
    m_coordinateOverlay->setVisible(false);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_chartView);

    m_fitButton = new FitButton(this);
    connect(m_fitButton, &FitButton::clicked, this, [this]() {
        updateAxesRange();
    });

    m_chartView->setMouseTracking(true);
    m_chartView->setFocus();
}

ChartWidget::~ChartWidget() {
    clearAll();
}

void ChartWidget::setDataModel(DataModel* model) {
    m_dataModel = model;
}

void ChartWidget::addSeries(const SeriesData& data, const QColor& color) {
    m_chartView->setFocus();

    if (data.isEmpty()) {
        return;
    }

    if (m_seriesMap.contains(data.name)) {
        removeSeries(data.name);
    }

    QLineSeries* series = new QLineSeries();
    series->setName(data.name);

    SeriesStyle style;
    QPen pen(color);
    pen.setWidth(style.lineWidth);
    pen.setStyle(Qt::SolidLine);
    series->setPen(pen);
    series->setPointsVisible(false);

    for (int i = 0; i < data.pointCount(); ++i) {
        series->append(data.timestamps[i], data.values[i]);
    }

    m_chart->addSeries(series);
    series->attachAxis(m_axisX);
    series->attachAxis(m_axisY);

    m_seriesMap[data.name] = series;
    m_seriesDataMap[data.name] = data;
    m_colorMap[data.name] = color;
    m_styleMap[data.name] = style;

    QTimer::singleShot(0, this, [this]() {
        QList<QLegendMarker*> markers = m_chart->legend()->markers();
        for (QLegendMarker* marker : markers) {
            disconnect(marker, &QLegendMarker::clicked, this, nullptr);
            connect(marker, &QLegendMarker::clicked, this, &ChartWidget::onLegendMarkerClicked);
        }
    });

    updateScatterSeries(data.name);

    connect(series, &QLineSeries::clicked, this, &ChartWidget::onSeriesClicked);
    connect(series, &QLineSeries::hovered, this, &ChartWidget::onHovered);

    updateAxesRange();
    m_chart->update();
}

void ChartWidget::removeSeries(const QString& name) {
    if (!m_seriesMap.contains(name)) {
        return;
    }

    if (m_selectedSeries == name) {
        m_selectedSeries.clear();
        m_selectedMarker = nullptr;
    }

    QLineSeries* series = m_seriesMap[name];
    m_chart->removeSeries(series);
    delete series;
    m_seriesMap.remove(name);

    if (m_scatterMap.contains(name)) {
        QScatterSeries* scatter = m_scatterMap[name];
        m_chart->removeSeries(scatter);
        delete scatter;
        m_scatterMap.remove(name);
    }

    m_seriesDataMap.remove(name);
    m_colorMap.remove(name);
    m_styleMap.remove(name);

    emit seriesRemoved(name);
}

void ChartWidget::clearAll() {
    for (auto* series : m_seriesMap) {
        m_chart->removeSeries(series);
        delete series;
    }
    for (auto* scatter : m_scatterMap) {
        m_chart->removeSeries(scatter);
        delete scatter;
    }

    m_seriesMap.clear();
    m_scatterMap.clear();
    m_seriesDataMap.clear();
    m_colorMap.clear();
    m_styleMap.clear();
    m_selectedSeries.clear();
    m_selectedMarker = nullptr;
}

QStringList ChartWidget::getSeriesNames() const {
    return m_seriesMap.keys();
}

QString ChartWidget::getTitle() const {
    return m_title;
}

void ChartWidget::setTitle(const QString& title) {
    m_title = title;
    m_chart->setTitle(title);
}

void ChartWidget::setZoomEnabled(bool enabled) {
    m_zoomEnabled = enabled;
    m_chartView->setWheelZoomEnabled(enabled);
    if (enabled) {
        m_chartView->setRubberBand(QChartView::RectangleRubberBand);
    } else {
        m_chartView->setRubberBand(QChartView::NoRubberBand);
    }
}

void ChartWidget::setPanEnabled(bool enabled) {
    m_panEnabled = enabled;
    if (enabled) {
        m_chartView->setDragMode(QGraphicsView::ScrollHandDrag);
    } else {
        m_chartView->setDragMode(QGraphicsView::NoDrag);
    }
}

void ChartWidget::fitToView() {
    m_chart->zoomReset();
}

void ChartWidget::resetZoom() {
    m_chart->zoomReset();
}

QString ChartWidget::selectedSeries() const {
    return m_selectedSeries;
}

void ChartWidget::setSelectedSeries(const QString& name) {
    if (m_seriesMap.contains(name) || name.isEmpty()) {
        m_selectedSeries = name;
        emit seriesSelected(name);
    }
}

void ChartWidget::setSeriesPointShape(const QString& name, PointShape shape) {
    if (!m_styleMap.contains(name)) return;
    m_styleMap[name].pointShape = shape;
    updateScatterSeries(name);
}

void ChartWidget::setSeriesPointSize(const QString& name, int size) {
    if (!m_styleMap.contains(name)) return;
    m_styleMap[name].pointSize = size;
    updateScatterSeries(name);
}

void ChartWidget::setSeriesLineStyle(const QString& name, LineStyle style) {
    if (!m_styleMap.contains(name)) return;
    m_styleMap[name].lineStyle = style;
    updateLineStyle(name);
}

void ChartWidget::setSeriesLineWidth(const QString& name, int width) {
    if (!m_styleMap.contains(name)) return;
    m_styleMap[name].lineWidth = width;
    updateLineStyle(name);
}

SeriesStyle ChartWidget::getSeriesStyle(const QString& name) const {
    return m_styleMap.value(name, SeriesStyle());
}

void ChartWidget::onChartClicked(const QPointF& point) {
    if (m_seriesMap.isEmpty()) {
        return;
    }

    QString nearestSeries;
    QPointF nearestPoint;
    double minDistance = std::numeric_limits<double>::max();

    for (auto it = m_seriesMap.begin(); it != m_seriesMap.end(); ++it) {
        const QString& name = it.key();
        QLineSeries* series = it.value();

        for (const QPointF& p : series->points()) {
            double distance = qSqrt(qPow(p.x() - point.x(), 2) + qPow(p.y() - point.y(), 2));
            if (distance < minDistance) {
                minDistance = distance;
                nearestSeries = name;
                nearestPoint = p;
            }
        }
    }

    if (!nearestSeries.isEmpty()) {
        m_coordinateOverlay->setText(QString("X: %1, Y: %2").arg(nearestPoint.x(), 0, 'f', 4).arg(nearestPoint.y(), 0, 'f', 4));
        m_coordinateOverlay->setVisible(true);
        emit coordinateSelected(nearestSeries, nearestPoint.x(), nearestPoint.y());
    }
}

void ChartWidget::onSeriesClicked(const QPointF& point) {
    m_coordinateOverlay->setText(QString("X: %1, Y: %2").arg(point.x()).arg(point.y()));
    m_coordinateOverlay->setVisible(true);

    QLineSeries* series = qobject_cast<QLineSeries*>(sender());
    if (series) {
        emit coordinateSelected(series->name(), point.x(), point.y());
    }
}

void ChartWidget::onHovered(const QPointF& point, bool state) {
    if (state) {
        m_coordinateOverlay->setText(QString("X: %1, Y: %2").arg(point.x(), 0, 'f', 4).arg(point.y(), 0, 'f', 4));
        m_coordinateOverlay->setVisible(true);
    } else {
        m_coordinateOverlay->setVisible(false);
    }
}

void ChartWidget::onLegendMarkerClicked() {
    QLegendMarker* marker = qobject_cast<QLegendMarker*>(sender());
    if (marker) {
        QString seriesName = marker->series()->name();
        if (seriesName.endsWith("_points")) {
            seriesName = seriesName.left(seriesName.length() - 7);
        }
        
        if (m_selectedSeries == seriesName) {
            setSelectedSeries(QString());
        } else {
            setSelectedSeries(seriesName);
            m_selectedMarker = marker;
        }
        updateLegendMarkerHighlight();
    }
}

void ChartWidget::updateLegendMarkerHighlight() {
    QList<QLegendMarker*> markers = m_chart->legend()->markers();
    for (QLegendMarker* marker : markers) {
        QString name = marker->series()->name();
        if (name.endsWith("_points")) continue;
        
        QFont font = marker->font();
        if (name == m_selectedSeries) {
            font.setBold(true);
            marker->setFont(font);
            marker->setLabelBrush(QBrush(Qt::red));
        } else {
            font.setBold(false);
            marker->setFont(font);
            marker->setLabelBrush(QBrush(Qt::black));
        }
    }
}

void ChartWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    QRectF plotArea = m_chart->plotArea();
    
    if (m_fitButton && plotArea.width() > 0 && plotArea.height() > 0) {
        int buttonX = m_chartView->geometry().left() + plotArea.right() - m_fitButton->width() - 5;
        int buttonY = m_chartView->geometry().top() + plotArea.top() + 5;
        m_fitButton->move(buttonX, buttonY);
        m_fitButton->show();
    }

    if (m_coordinateOverlay) {
        m_coordinateOverlay->setPos(event->size().width() - 210, 10);
    }
}

void ChartWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    
    QTimer::singleShot(0, this, [this]() {
        QRectF plotArea = m_chart->plotArea();
        if (m_fitButton && plotArea.width() > 0 && plotArea.height() > 0) {
            int buttonX = m_chartView->geometry().left() + plotArea.right() - m_fitButton->width() - 5;
            int buttonY = m_chartView->geometry().top() + plotArea.top() + 5;
            m_fitButton->move(buttonX, buttonY);
            m_fitButton->show();
        }
    });
}

void ChartWidget::updateAxesRange() {
    if (m_seriesDataMap.isEmpty()) {
        m_axisX->setRange(0, 10);
        m_axisY->setRange(0, 10);
        return;
    }

    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    for (const auto& data : m_seriesDataMap) {
        if (!data.timestamps.isEmpty()) {
            minX = std::min(minX, *std::min_element(data.timestamps.begin(), data.timestamps.end()));
            maxX = std::max(maxX, *std::max_element(data.timestamps.begin(), data.timestamps.end()));
        }
        if (!data.values.isEmpty()) {
            minY = std::min(minY, *std::min_element(data.values.begin(), data.values.end()));
            maxY = std::max(maxY, *std::max_element(data.values.begin(), data.values.end()));
        }
    }

    if (minX == std::numeric_limits<double>::max()) minX = 0;
    if (maxX == std::numeric_limits<double>::lowest()) maxX = 10;
    if (minY == std::numeric_limits<double>::max()) minY = 0;
    if (maxY == std::numeric_limits<double>::lowest()) maxY = 10;

    double paddingX = (maxX - minX) * 0.05;
    double paddingY = (maxY - minY) * 0.05;
    if (paddingX == 0) paddingX = 1;
    if (paddingY == 0) paddingY = 1;

    m_axisX->setRange(minX - paddingX, maxX + paddingX);
    m_axisY->setRange(minY - paddingY, maxY + paddingY);
}

void ChartWidget::updateScatterSeries(const QString& name) {
    if (!m_seriesMap.contains(name)) return;

    if (m_scatterMap.contains(name)) {
        QScatterSeries* oldScatter = m_scatterMap[name];
        m_chart->removeSeries(oldScatter);
        delete oldScatter;
        m_scatterMap.remove(name);
    }

    SeriesStyle style = m_styleMap[name];
    
    if (style.pointShape == PointShape::None) {
        return;
    }

    const SeriesData& data = m_seriesDataMap[name];
    QColor color = m_colorMap[name];

    QScatterSeries* scatter = new QScatterSeries();
    scatter->setName(name + "_points");
    scatter->setMarkerSize(style.pointSize);

    qreal size = style.pointSize;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color, 1));
    painter.setBrush(QBrush(color));

    switch (style.pointShape) {
        case PointShape::Circle:
            scatter->setMarkerShape(QScatterSeries::MarkerShapeCircle);
            scatter->setColor(color);
            scatter->setBorderColor(color);
            break;
        case PointShape::Square:
            scatter->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
            scatter->setColor(color);
            scatter->setBorderColor(color);
            break;
        case PointShape::Triangle: {
            painter.drawPolygon(QPolygonF() 
                << QPointF(size/2, 1) 
                << QPointF(1, size-1) 
                << QPointF(size-1, size-1));
            scatter->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
            scatter->setBrush(pixmap);
            scatter->setPen(Qt::NoPen);
            break;
        }
        case PointShape::Star: {
            QPolygonF star;
            for (int i = 0; i < 5; ++i) {
                qreal outerAngle = i * 72 * M_PI / 180 - M_PI/2;
                qreal innerAngle = outerAngle + 36 * M_PI / 180;
                star << QPointF(size/2 + qCos(outerAngle) * size/2, size/2 + qSin(outerAngle) * size/2);
                star << QPointF(size/2 + qCos(innerAngle) * size/4, size/2 + qSin(innerAngle) * size/4);
            }
            painter.drawPolygon(star);
            scatter->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
            scatter->setBrush(pixmap);
            scatter->setPen(Qt::NoPen);
            break;
        }
        default:
            break;
    }

    for (int i = 0; i < data.pointCount(); ++i) {
        scatter->append(data.timestamps[i], data.values[i]);
    }

    m_chart->addSeries(scatter);
    scatter->attachAxis(m_axisX);
    scatter->attachAxis(m_axisY);
    m_scatterMap[name] = scatter;

    QList<QLegendMarker*> markers = m_chart->legend()->markers(scatter);
    for (QLegendMarker* marker : markers) {
        marker->setVisible(false);
    }

    connect(scatter, &QScatterSeries::clicked, this, &ChartWidget::onSeriesClicked);
    connect(scatter, &QScatterSeries::hovered, this, &ChartWidget::onHovered);
}

void ChartWidget::updateLineStyle(const QString& name) {
    if (!m_seriesMap.contains(name)) return;

    QLineSeries* series = m_seriesMap[name];
    SeriesStyle style = m_styleMap[name];
    QColor color = m_colorMap[name];

    QPen pen(color);
    pen.setWidth(style.lineWidth);
    
    switch (style.lineStyle) {
        case LineStyle::Solid:
            pen.setStyle(Qt::SolidLine);
            break;
        case LineStyle::Dash:
            pen.setStyle(Qt::DashLine);
            break;
        case LineStyle::Dot:
            pen.setStyle(Qt::DotLine);
            break;
        case LineStyle::DashDot:
            pen.setStyle(Qt::DashDotLine);
            break;
    }
    
    series->setPen(pen);
}

QColor ChartWidget::getSeriesColor(const QString& seriesName) const {
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

} // namespace Viewer
