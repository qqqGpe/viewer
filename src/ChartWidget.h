#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QColor>
#include <QHash>
#include <QPoint>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QPushButton>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include "models/SeriesData.h"

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLegendMarker>

QT_CHARTS_USE_NAMESPACE

namespace Viewer {

enum class PointShape {
    None,
    Circle,
    Square,
    Triangle,
    Star
};

enum class LineStyle {
    Solid,
    Dash,
    Dot,
    DashDot
};

struct SeriesStyle {
    PointShape pointShape = PointShape::Circle;
    int pointSize = 8;
    LineStyle lineStyle = LineStyle::Solid;
    int lineWidth = 3;
};

class DataModel;

class FitButton : public QWidget {
    Q_OBJECT

public:
    explicit FitButton(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

signals:
    void clicked();

private:
    bool m_hovered = false;
};

class CoordinateOverlay : public QGraphicsItem {
public:
    explicit CoordinateOverlay(QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void setText(const QString& text);
    void setVisible(bool visible);

private:
    QString m_text;
    bool m_visible = false;
};

class CustomChartView : public QChartView {
    Q_OBJECT

public:
    explicit CustomChartView(QChart* chart, QWidget* parent = nullptr);

    void setWheelZoomEnabled(bool enabled);

signals:
    void seriesDropped(const QString& seriesName);
    void pointClicked(const QPointF& point);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    bool m_wheelZoomEnabled = true;
    bool m_xKeyPressed = false;
    bool m_vKeyPressed = false;
    bool m_midButtonPressed = false;
    bool m_leftButtonPressed = false;
    bool m_isRubberBandActive = false;
    QPoint m_lastMousePos;
    QPoint m_leftButtonStartPos;
    static const int m_rubberBandThreshold = 10;
};

class ChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChartWidget(QWidget* parent = nullptr);
    ~ChartWidget();

    void setDataModel(DataModel* model);
    void addSeries(const SeriesData& data, const QColor& color, const QString& compoundKey);
    void removeSeries(const QString& name);
    void clearAll();

    QStringList getSeriesNames() const;

    QString getTitle() const;
    void setTitle(const QString& title);

    void setZoomEnabled(bool enabled);
    void setPanEnabled(bool enabled);

    void fitToView();
    void resetZoom();

    QString selectedSeries() const;
    void setSelectedSeries(const QString& name);
    
    void setSeriesPointShape(const QString& name, PointShape shape);
    void setSeriesPointSize(const QString& name, int size);
    void setSeriesLineStyle(const QString& name, LineStyle style);
    void setSeriesLineWidth(const QString& name, int width);
    
    SeriesStyle getSeriesStyle(const QString& name) const;

signals:
    void coordinateSelected(const QString& seriesName, double x, double y);
    void seriesRemoved(const QString& name);
    void seriesDropped(const QString& seriesName);
    void seriesSelected(const QString& name);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void onChartClicked(const QPointF& point);
    void onSeriesClicked(const QPointF& point);
    void onHovered(const QPointF& point, bool state);
    void onLegendMarkerClicked();

private:
    void updateAxesRange();
    void updateScatterSeries(const QString& name);
    void updateLineStyle(const QString& name);
    void updateLegendMarkerHighlight();
    void updateHighlightPoint(const QPointF& point, const QColor& color);
    QColor getSeriesColor(const QString& seriesName) const;

    QChart* m_chart;
    CustomChartView* m_chartView;
    QValueAxis* m_axisX;
    QValueAxis* m_axisY;
    CoordinateOverlay* m_coordinateOverlay;
    DataModel* m_dataModel = nullptr;
    FitButton* m_fitButton;

    QHash<QString, QLineSeries*> m_seriesMap;
    QHash<QString, QScatterSeries*> m_scatterMap;
    QHash<QString, SeriesData> m_seriesDataMap;
    QHash<QString, QColor> m_colorMap;
    QHash<QString, SeriesStyle> m_styleMap;
    QHash<QString, QString> m_displayNameMap;           // compoundKey -> displayName
    QHash<QString, QString> m_compoundKeyByDisplayName; // displayName -> compoundKey

    QString m_title;
    bool m_zoomEnabled = true;
    bool m_panEnabled = true;
    QString m_selectedSeries;
    QLegendMarker* m_selectedMarker = nullptr;
    QScatterSeries* m_highlightPoint = nullptr;
    QColor m_highlightColor;
};

} // namespace Viewer

#endif // CHARTWIDGET_H
