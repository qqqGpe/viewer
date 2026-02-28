#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "CurveConfig.h"
#include <QList>
#include <QObject>
#include <QSettings>

namespace Viewer {

class ConfigManager : public QObject {
    Q_OBJECT
public:
    static ConfigManager& instance();
    void saveConfig(const CurveConfig& cfg);
    void deleteConfig(const QString& name);
    QList<CurveConfig> getAllConfigs() const;
    CurveConfig getConfig(const QString& name) const;
signals:
    void configsChanged();
private:
    ConfigManager();
    void loadFromSettings();
    void saveToSettings();
    QList<CurveConfig> m_configs;
};

} // namespace Viewer

#endif
