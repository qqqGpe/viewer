#include "ConfigManager.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QFile>

namespace Viewer {

ConfigManager& ConfigManager::instance() {
    static ConfigManager inst;
    return inst;
}

ConfigManager::ConfigManager() {
    loadFromSettings();
}

void ConfigManager::saveConfig(const CurveConfig& cfg) {
    for (int i = 0; i < m_configs.size(); ++i) {
        if (m_configs[i].name == cfg.name) {
            m_configs.removeAt(i);
            break;
        }
    }
    m_configs.append(cfg);
    saveToSettings();
    emit configsChanged();
}

void ConfigManager::deleteConfig(const QString& name) {
    for (int i = 0; i < m_configs.size(); ++i) {
        if (m_configs[i].name == name) {
            m_configs.removeAt(i);
            break;
        }
    }
    saveToSettings();
    emit configsChanged();
}

QList<CurveConfig> ConfigManager::getAllConfigs() const {
    return m_configs;
}

CurveConfig ConfigManager::getConfig(const QString& name) const {
    for (const auto& c : m_configs) {
        if (c.name == name) return c;
    }
    return CurveConfig();
}

void ConfigManager::loadFromSettings() {
    QString configPath = QCoreApplication::applicationDirPath() + "/config";
    QDir configDir(configPath);
    if (!configDir.exists()) {
        configDir.mkpath(".");
    }
    
    QFile file(configPath + "/favor.json");
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    if (data.isEmpty()) return;
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return;
    QJsonArray arr = doc.array();
    for (const QJsonValue& v : arr) {
        m_configs.append(CurveConfig::fromJson(v.toObject()));
    }
}

void ConfigManager::saveToSettings() {
    QString configPath = QCoreApplication::applicationDirPath() + "/config";
    QDir configDir(configPath);
    if (!configDir.exists()) {
        configDir.mkpath(".");
    }
    
    QJsonArray arr;
    for (const auto& c : m_configs) {
        arr.append(c.toJson());
    }
    QJsonDocument doc(arr);
    
    QFile file(configPath + "/favor.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

} // namespace Viewer
