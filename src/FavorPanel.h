#ifndef FAVORPANEL_H
#define FAVORPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QMenu>

namespace Viewer {

class FavorPanel : public QWidget {
    Q_OBJECT

public:
    explicit FavorPanel(QWidget* parent = nullptr);

    void refreshConfigs();

signals:
    void applyConfigRequested(const QString& configName);

private slots:
    void onItemDoubleClicked(QListWidgetItem* item);
    void showContextMenu(const QPoint& pos);

private:
    QListWidget* m_listWidget;
};

} // namespace Viewer

#endif // FAVORPANEL_H
