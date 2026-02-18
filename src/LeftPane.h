#ifndef LEFTPANE_H
#define LEFTPANE_H

#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QDrag>
#include <QMimeData>
#include "DataModel.h"

namespace Viewer {

class DragTreeWidget : public QTreeWidget {
    Q_OBJECT

public:
    explicit DragTreeWidget(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QTreeWidgetItem* m_dragItem = nullptr;
    QPoint m_dragStartPos;
};

class LeftPane : public QWidget {
    Q_OBJECT

public:
    explicit LeftPane(QWidget* parent = nullptr);

    void setDataModel(DataModel* model);

    QSet<QString> getSelectedSeries() const;
    void clearSelection();
    void setSeriesSelected(const QString& seriesName, bool selected);
    void syncSelectionWithSeries(const QStringList& seriesNames);

signals:
    void seriesSelected(const QStringList& seriesNames);
    void seriesDeselected(const QStringList& seriesNames);
    void addToCanvasRequested(const QStringList& seriesNames);
    void removeFromCanvasRequested(const QStringList& seriesNames);
    void closeFileRequested(const QString& filePath);

private slots:
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void showContextMenu(const QPoint& pos);

    void onDataLoaded(const QString& filePath);
    void onFileAdded(const QString& filePath);
    void onFileRemoved(const QString& filePath);
    void onDataCleared();

private:
    void populateTree();
    QTreeWidgetItem* createFileItem(const QString& filePath);
    QTreeWidgetItem* findFileItem(const QString& filePath);
    QTreeWidgetItem* createCategoryItem(const QString& filePath, const QString& category, QTreeWidgetItem* parent);
    QTreeWidgetItem* findCategoryItem(const QString& filePath, const QString& category);
    void updateSeriesSelection();

    DragTreeWidget* m_treeWidget;
    DataModel* m_dataModel = nullptr;
    QMenu* m_seriesContextMenu;
    QMenu* m_fileContextMenu;

    QSet<QString> m_selectedSeries;
};

} // namespace Viewer

#endif // LEFTPANE_H
