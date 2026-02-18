#include "LeftPane.h"
#include "models/SeriesData.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDebug>
#include <QMouseEvent>
#include <QDrag>
#include <QApplication>
#include <QFileInfo>

namespace Viewer {

DragTreeWidget::DragTreeWidget(QWidget* parent)
    : QTreeWidget(parent) {
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

void DragTreeWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragItem = itemAt(event->pos());
        m_dragStartPos = event->pos();
    }
    QTreeWidget::mousePressEvent(event);
}

void DragTreeWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!(event->buttons() & Qt::LeftButton)) {
        QTreeWidget::mouseMoveEvent(event);
        return;
    }

    if ((event->pos() - m_dragStartPos).manhattanLength() < QApplication::startDragDistance()) {
        QTreeWidget::mouseMoveEvent(event);
        return;
    }

    if (!m_dragItem) {
        QTreeWidget::mouseMoveEvent(event);
        return;
    }

    QString seriesName = m_dragItem->data(0, Qt::UserRole).toString();
    if (seriesName.isEmpty()) {
        QTreeWidget::mouseMoveEvent(event);
        return;
    }

    qDebug() << "DragTreeWidget: Starting drag for" << seriesName;

    QMimeData* mimeData = new QMimeData();
    mimeData->setText(seriesName);
    mimeData->setData("application/x-series-name", seriesName.toUtf8());

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    Qt::DropAction result = drag->exec(Qt::CopyAction);
    qDebug() << "DragTreeWidget: Drag result" << result;
}

LeftPane::LeftPane(QWidget* parent)
    : QWidget(parent) {

    m_treeWidget = new DragTreeWidget(this);
    m_treeWidget->setHeaderLabels(QStringList() << "Data Files");
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeWidget->setRootIsDecorated(true);
    m_treeWidget->setColumnWidth(0, 200);

    m_seriesContextMenu = new QMenu(this);
    m_seriesContextMenu->addAction("Add to Canvas", this, [this]() {
        QStringList selected = getSelectedSeries().values();
        if (!selected.isEmpty()) {
            emit addToCanvasRequested(selected);
        }
    });
    m_seriesContextMenu->addAction("Remove from Canvas", this, [this]() {
        QStringList selected = getSelectedSeries().values();
        if (!selected.isEmpty()) {
            emit removeFromCanvasRequested(selected);
        }
    });
    m_seriesContextMenu->addSeparator();
    QAction* expandAllAction = m_seriesContextMenu->addAction("Expand All");
    QAction* collapseAllAction = m_seriesContextMenu->addAction("Collapse All");
    connect(expandAllAction, &QAction::triggered, m_treeWidget, &QTreeWidget::expandAll);
    connect(collapseAllAction, &QAction::triggered, m_treeWidget, &QTreeWidget::collapseAll);

    m_fileContextMenu = new QMenu(this);
    m_fileContextMenu->addAction("Close File", this, [this]() {
        QTreeWidgetItem* item = m_treeWidget->currentItem();
        if (item && !item->parent()) {
            QString filePath = item->data(0, Qt::UserRole).toString();
            emit closeFileRequested(filePath);
        }
    });
    m_fileContextMenu->addSeparator();
    m_fileContextMenu->addAction("Expand All", m_treeWidget, &QTreeWidget::expandAll);
    m_fileContextMenu->addAction("Collapse All", m_treeWidget, &QTreeWidget::collapseAll);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_treeWidget);

    connect(m_treeWidget, &QTreeWidget::itemChanged, this, &LeftPane::onItemChanged);
    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &LeftPane::onItemDoubleClicked);
    connect(m_treeWidget, &QTreeWidget::customContextMenuRequested, this, &LeftPane::showContextMenu);
}

void LeftPane::setDataModel(DataModel* model) {
    if (m_dataModel) {
        disconnect(m_dataModel, nullptr, this, nullptr);
    }

    m_dataModel = model;

    if (m_dataModel) {
        connect(m_dataModel, &DataModel::dataLoaded, this, &LeftPane::onDataLoaded);
        connect(m_dataModel, &DataModel::fileAdded, this, &LeftPane::onFileAdded);
        connect(m_dataModel, &DataModel::fileRemoved, this, &LeftPane::onFileRemoved);
        connect(m_dataModel, &DataModel::dataCleared, this, &LeftPane::onDataCleared);
    }
}

QSet<QString> LeftPane::getSelectedSeries() const {
    return m_selectedSeries;
}

void LeftPane::clearSelection() {
    m_selectedSeries.clear();
    QTreeWidgetItemIterator it(m_treeWidget);
    while (*it) {
        QString seriesName = (*it)->data(0, Qt::UserRole).toString();
        if (!seriesName.isEmpty() && (*it)->parent() && (*it)->parent()->parent()) {
            (*it)->setCheckState(0, Qt::Unchecked);
        }
        ++it;
    }
}

void LeftPane::setSeriesSelected(const QString& seriesName, bool selected) {
    if (seriesName.isEmpty()) return;
    
    QTreeWidgetItemIterator it(m_treeWidget);
    while (*it) {
        QString itemSeriesName = (*it)->data(0, Qt::UserRole).toString();
        if (itemSeriesName == seriesName) {
            bool wasBlocked = m_treeWidget->blockSignals(true);
            (*it)->setCheckState(0, selected ? Qt::Checked : Qt::Unchecked);
            m_treeWidget->blockSignals(wasBlocked);
            
            if (selected) {
                m_selectedSeries.insert(seriesName);
            } else {
                m_selectedSeries.remove(seriesName);
            }
            break;
        }
        ++it;
    }
}

void LeftPane::syncSelectionWithSeries(const QStringList& seriesNames) {
    m_selectedSeries.clear();
    m_selectedSeries = QSet<QString>::fromList(seriesNames);
    
    bool wasBlocked = m_treeWidget->blockSignals(true);
    
    QTreeWidgetItemIterator it(m_treeWidget);
    while (*it) {
        QString seriesName = (*it)->data(0, Qt::UserRole).toString();
        if (!seriesName.isEmpty() && (*it)->parent() && (*it)->parent()->parent()) {
            bool shouldSelect = seriesNames.contains(seriesName);
            (*it)->setCheckState(0, shouldSelect ? Qt::Checked : Qt::Unchecked);
        }
        ++it;
    }
    
    m_treeWidget->blockSignals(wasBlocked);
}

void LeftPane::onItemChanged(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);

    qDebug() << "LeftPane::onItemChanged";

    if (!item->parent()) {
        Qt::CheckState state = item->checkState(0);
        qDebug() << "  File item:" << item->text(0) << "state:" << state;
        for (int i = 0; i < item->childCount(); ++i) {
            QTreeWidgetItem* categoryItem = item->child(i);
            categoryItem->setCheckState(0, state);
        }
        return;
    }

    if (!item->parent()->parent()) {
        Qt::CheckState state = item->checkState(0);
        qDebug() << "  Category item:" << item->text(0) << "state:" << state;
        for (int i = 0; i < item->childCount(); ++i) {
            item->child(i)->setCheckState(0, state);
        }
        return;
    }

    QString seriesName = item->data(0, Qt::UserRole).toString();
    bool selected = item->checkState(0) == Qt::Checked;

    qDebug() << "  Series item:" << seriesName << "selected:" << selected;

    if (selected && !m_selectedSeries.contains(seriesName)) {
        m_selectedSeries.insert(seriesName);
        qDebug() << "  Emitting seriesSelected for:" << seriesName;
        emit seriesSelected(QStringList(seriesName));
    } else if (!selected && m_selectedSeries.contains(seriesName)) {
        m_selectedSeries.remove(seriesName);
        qDebug() << "  Emitting seriesDeselected for:" << seriesName;
        emit seriesDeselected(QStringList(seriesName));
    }
}

void LeftPane::onItemDoubleClicked(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);

    QString seriesName = item->data(0, Qt::UserRole).toString();
    if (!seriesName.isEmpty() && item->parent() && item->parent()->parent()) {
        bool currentState = item->checkState(0) == Qt::Checked;
        item->setCheckState(0, currentState ? Qt::Unchecked : Qt::Checked);
    }
}

void LeftPane::showContextMenu(const QPoint& pos) {
    QTreeWidgetItem* item = m_treeWidget->itemAt(pos);
    if (!item) return;

    if (!item->parent()) {
        m_fileContextMenu->exec(m_treeWidget->mapToGlobal(pos));
    } else if (item->parent()->parent()) {
        m_seriesContextMenu->exec(m_treeWidget->mapToGlobal(pos));
    }
}

void LeftPane::onDataLoaded(const QString& filePath) {
    Q_UNUSED(filePath);
    populateTree();
}

void LeftPane::onFileAdded(const QString& filePath) {
    Q_UNUSED(filePath);
    populateTree();
}

void LeftPane::onFileRemoved(const QString& filePath) {
    Q_UNUSED(filePath);
    populateTree();
}

void LeftPane::onDataCleared() {
    m_treeWidget->clear();
    m_selectedSeries.clear();
}

void LeftPane::populateTree() {
    m_treeWidget->clear();
    m_selectedSeries.clear();

    if (!m_dataModel) {
        return;
    }

    auto fileNames = m_dataModel->getFileNames();
    for (const auto& filePath : fileNames) {
        auto fileItem = createFileItem(filePath);
        auto categories = m_dataModel->getCategoriesForFile(filePath);
        
        for (const auto& category : categories) {
            auto categoryItem = createCategoryItem(filePath, category, fileItem);
            auto seriesList = m_dataModel->getSeriesByFileAndCategory(filePath, category);

            for (const auto& series : seriesList) {
                QTreeWidgetItem* seriesItem = new QTreeWidgetItem(categoryItem);
                seriesItem->setData(0, Qt::UserRole, series.name);
                seriesItem->setText(0, series.name);
                seriesItem->setCheckState(0, Qt::Unchecked);
            }
        }
    }
}

QTreeWidgetItem* LeftPane::createFileItem(const QString& filePath) {
    QTreeWidgetItem* item = new QTreeWidgetItem(m_treeWidget);
    item->setData(0, Qt::UserRole, filePath);
    QFileInfo fileInfo(filePath);
    item->setText(0, fileInfo.fileName());
    item->setExpanded(true);
    item->setCheckState(0, Qt::Unchecked);
    return item;
}

QTreeWidgetItem* LeftPane::findFileItem(const QString& filePath) {
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);
        if (item->data(0, Qt::UserRole).toString() == filePath) {
            return item;
        }
    }
    return nullptr;
}

QTreeWidgetItem* LeftPane::createCategoryItem(const QString& filePath, const QString& category, QTreeWidgetItem* parent) {
    Q_UNUSED(filePath);
    QTreeWidgetItem* item = new QTreeWidgetItem(parent);
    item->setData(0, Qt::UserRole, category);
    item->setText(0, category);
    item->setExpanded(true);
    item->setCheckState(0, Qt::Unchecked);
    return item;
}

QTreeWidgetItem* LeftPane::findCategoryItem(const QString& filePath, const QString& category) {
    QTreeWidgetItem* fileItem = findFileItem(filePath);
    if (!fileItem) return nullptr;
    
    for (int i = 0; i < fileItem->childCount(); ++i) {
        QTreeWidgetItem* item = fileItem->child(i);
        if (item->text(0) == category) {
            return item;
        }
    }
    return nullptr;
}

void LeftPane::updateSeriesSelection() {
    QTreeWidgetItemIterator it(m_treeWidget);
    while (*it) {
        QString seriesName = (*it)->data(0, Qt::UserRole).toString();
        if (!seriesName.isEmpty() && (*it)->parent() && (*it)->parent()->parent()) {
            (*it)->setCheckState(0, m_selectedSeries.contains(seriesName) ? Qt::Checked : Qt::Unchecked);
        }
        ++it;
    }
}

} // namespace Viewer
