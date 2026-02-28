#include "FavorPanel.h"
#include "models/ConfigManager.h"
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>

namespace Viewer {

FavorPanel::FavorPanel(QWidget* parent)
    : QWidget(parent) {

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QLabel* header = new QLabel("Saved Configs", this);
    header->setStyleSheet(
        "font-weight: bold; padding: 4px 6px;"
        "background: #d0d8e8; border-top: 1px solid #b0b8c8;");
    layout->addWidget(header);

    m_listWidget = new QListWidget(this);
    m_listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listWidget->setAlternatingRowColors(true);
    m_listWidget->setToolTip("Double-click to apply config to a new canvas");
    layout->addWidget(m_listWidget);

    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this, &FavorPanel::onItemDoubleClicked);
    connect(m_listWidget, &QListWidget::customContextMenuRequested,
            this, &FavorPanel::showContextMenu);

    refreshConfigs();
}

void FavorPanel::refreshConfigs() {
    m_listWidget->clear();
    for (const CurveConfig& cfg : ConfigManager::instance().getAllConfigs()) {
        QListWidgetItem* item = new QListWidgetItem(cfg.name);
        item->setToolTip(cfg.seriesNames.join("\n"));
        m_listWidget->addItem(item);
    }
}

void FavorPanel::onItemDoubleClicked(QListWidgetItem* item) {
    emit applyConfigRequested(item->text());
}

void FavorPanel::showContextMenu(const QPoint& pos) {
    QListWidgetItem* item = m_listWidget->itemAt(pos);
    if (!item) return;

    QMenu menu(this);
    QAction* applyAction  = menu.addAction("Apply to New Canvas");
    QAction* deleteAction = menu.addAction("Delete");

    QAction* chosen = menu.exec(m_listWidget->mapToGlobal(pos));
    if (chosen == applyAction) {
        emit applyConfigRequested(item->text());
    } else if (chosen == deleteAction) {
        ConfigManager::instance().deleteConfig(item->text());
    }
}

} // namespace Viewer
