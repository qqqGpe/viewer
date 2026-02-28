# Curve Config Internal Storage Enhancement

## TL;DR

> **Quick Summary**: Modify curve config save/load to store internally in app (QSettings) with user-defined names, add "Saved Configs" submenu to display configs, auto-create canvas when loading config.

> **Deliverables**:
> - Save:弹出名称输入对话框，保存到内部存储
> - Menu:添加 "Saved Configs" 子菜单显示所有配置
> - Load:点击配置自动新开画布显示曲线

> **Estimated Effort**: Medium
> **Parallel Execution**: NO - sequential

---

## Context

### Original Request
User wants to:
1. Click "Save Curve Config" →弹出对话框输入名称 → 直接保存在软件内部（不选择文件位置）
2. 在菜单栏下添加 "Saved Configs" 子菜单，显示之前保存的配置
3. 点击配置时 → 从当前数据文件找到对应曲线 → 新开画布 → 显示在右侧

### Current Implementation
- CurveConfig: name, sourceFile, seriesNames
- Save:弹出 QFileDialog 选择保存位置
- Load:弹出 QFileDialog 选择文件

### Required Changes
- 修改保存逻辑：弹出 QInputDialog 让用户输入名称，保存到 QSettings
- 添加 ConfigManager 类：管理配置的增删改查
- 添加 "Saved Configs" 子菜单：动态显示所有保存的配置
- 修改加载逻辑：新开画布，显示曲线

---

## Work Objectives

### Core Objective
将曲线配置从外部文件存储改为内部存储（QSettings），支持命名管理。

### Concrete Deliverables
1. CurveConfig 添加 name 字段（已部分完成）
2. ConfigManager 类：内存+QSettings持久化
3. onSaveConfig 修改：弹出名称输入对话框
4. 添加 Saved Configs 子菜单
5. 加载配置时新开画布显示曲线
6. 支持删除配置

---

## Implementation Steps

### Step 1: 修改 CurveConfig
**File**: `src/models/CurveConfig.h`

```cpp
struct CurveConfig {
    QString name;            // NEW: User-defined name
    QString sourceFile;
    QStringList seriesNames;
    // ... (keep existing serialization)
};
```

### Step 2: 创建 ConfigManager 类
**New File**: `src/models/ConfigManager.h`

```cpp
#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "CurveConfig.h"
#include <QList>
#include <QObject>

namespace Viewer {

class ConfigManager : public QObject {
    Q_OBJECT
public:
    static ConfigManager& instance();
    
    void saveConfig(const CurveConfig& config);
    void deleteConfig(const QString& name);
    QList<CurveConfig> getAllConfigs() const;
    CurveConfig getConfig(const QString& name) const;
    
signals:
    void configsChanged();
    
private:
    ConfigManager();
    void loadFromSettings();
    void saveToSettings();
};

} // namespace Viewer
#endif
```

**New File**: `src/models/ConfigManager.cpp`

```cpp
#include "ConfigManager.h"
#include <QSettings>
#include <QJsonArray>
#include <QJsonDocument>

namespace Viewer {

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager() {
    loadFromSettings();
}

void ConfigManager::saveConfig(const CurveConfig& config) {
    // Remove existing config with same name
    for (int i = 0; i < m_configs.size(); ++i) {
        if (m_configs[i].name == config.name) {
            m_configs.removeAt(i);
            break;
        }
    }
    m_configs.append(config);
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
    for (const auto& config : m_configs) {
        if (config.name == name) {
            return config;
        }
    }
    return CurveConfig();
}

void ConfigManager::loadFromSettings() {
    QSettings settings;
    QByteArray data = settings.value("curveConfigs").toByteArray();
    if (data.isEmpty()) return;
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray arr = doc.array();
    for (const QJsonValue& val : arr) {
        m_configs.append(CurveConfig::fromJson(val.toObject()));
    }
}

void ConfigManager::saveToSettings() {
    QJsonArray arr;
    for (const auto& config : m_configs) {
        arr.append(config.toJson());
    }
    QJsonDocument doc(arr);
    QSettings settings;
    settings.setValue("curveConfigs", doc.toJson());
}

} // namespace Viewer
```

### Step 3: 修改 MainWindow.h
**File**: `src/MainWindow.h`

添加:
```cpp
#include "models/ConfigManager.h"

// 在 private slots 添加:
void onLoadSavedConfig(const QString& configName);
void onDeleteConfig(const QString& configName);

// 在 private 添加:
void updateSavedConfigsMenu();
```

### Step 4: 修改 MainWindow.cpp
**File**: `src/MainWindow.cpp`

#### 4.1 添加 include
```cpp
#include "models/ConfigManager.h"
```

#### 4.2 修改 createMenus()
```cpp
void MainWindow::createMenus() {
    QMenu* fileMenu = menuBar()->addMenu("File");
    // ... existing items ...
    fileMenu->addSeparator();
    fileMenu->addAction(m_saveConfigAction);
    fileMenu->addAction(m_loadConfigAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    // NEW: Saved Configs submenu
    m_savedConfigsMenu = fileMenu->addMenu("Saved Configs");
    updateSavedConfigsMenu();

    // ... rest ...
}
```

#### 4.3 添加成员变量
```cpp
QMenu* m_savedConfigsMenu;
```

#### 4.4 修改 onSaveConfig()
```cpp
void MainWindow::onSaveConfig() {
    if (m_dataModel->getFileCount() == 0) {
        QMessageBox::warning(this, "Warning", "No file loaded.");
        return;
    }

    QSet<QString> selectedSet = m_leftPane->getSelectedSeries();
    QStringList seriesNames = selectedSet.toList();

    if (seriesNames.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No curves selected.");
        return;
    }

    // NEW:弹出输入对话框让用户输入名称
    bool ok;
    QString configName = QInputDialog::getText(this, "Save Curve Config",
        "Enter a name for this configuration:",
        QLineEdit::Normal, "", &ok);
    
    if (!ok || configName.isEmpty()) {
        return;
    }

    CurveConfig config;
    config.name = configName;
    config.sourceFile = m_dataModel->getFileNames().first();
    config.seriesNames = seriesNames;

    ConfigManager::instance().saveConfig(config);
    QMessageBox::information(this, "Success", 
        QString("Configuration '%1' saved!").arg(configName));
}
```

#### 4.5 修改 onLoadConfig() - 改为从内部存储加载
```cpp
void MainWindow::onLoadConfig() {
    // Show list of saved configs
    QList<CurveConfig> configs = ConfigManager::instance().getAllConfigs();
    if (configs.isEmpty()) {
        QMessageBox::information(this, "Info", "No saved configurations.");
        return;
    }

    // Show dialog to select config
    QStringList names;
    for (const auto& c : configs) {
        names.append(c.name);
    }

    bool ok;
    QString selected = QInputDialog::getItem(this, "Load Curve Config",
        "Select a configuration:", names, 0, false, &ok);
    
    if (!ok || selected.isEmpty()) {
        return;
    }

    loadSavedConfig(selected);
}

void MainWindow::loadSavedConfig(const QString& configName) {
    CurveConfig config = ConfigManager::instance().getConfig(configName);
    if (config.name.isEmpty()) {
        QMessageBox::warning(this, "Error", "Configuration not found.");
        return;
    }

    // Check if matching file is open
    if (m_dataModel->getFileCount() == 0) {
        QMessageBox::warning(this, "Warning", 
            QString("Configuration '%1' requires file:\n%2\n\nNo file loaded.")
            .arg(configName).arg(QFileInfo(config.sourceFile).fileName()));
        return;
    }

    QString currentFile = m_dataModel->getFileNames().first();
    if (currentFile != config.sourceFile) {
        QMessageBox::warning(this, "Warning", 
            QString("Configuration '%1' was saved for:\n%2\n\nCurrently loaded:\n%3")
            .arg(configName)
            .arg(QFileInfo(config.sourceFile).fileName())
            .arg(QFileInfo(currentFile).fileName()));
        return;
    }

    // NEW: Create new canvas and show curves
    m_canvasArea->createNewCanvas();
    m_leftPane->syncSelectionWithSeries(config.seriesNames);
    
    // Add series to new canvas
    for (const QString& seriesName : config.seriesNames) {
        m_canvasArea->addSeriesToCurrentCanvas(seriesName);
    }

    QMessageBox::information(this, "Success", 
        QString("Loaded %1 curve(s) from '%2'.")
        .arg(config.seriesNames.size()).arg(configName));
}
```

#### 4.6 添加 updateSavedConfigsMenu()
```cpp
void MainWindow::updateSavedConfigsMenu() {
    m_savedConfigsMenu->clear();
    
    QList<CurveConfig> configs = ConfigManager::instance().getAllConfigs();
    
    if (configs.isEmpty()) {
        QAction* emptyAction = new QAction("(No saved configs)", this);
        emptyAction->setEnabled(false);
        m_savedConfigsMenu->addAction(emptyAction);
    } else {
        for (const CurveConfig& config : configs) {
            QAction* action = new QAction(config.name, this);
            connect(action, &QAction::triggered, [this, config]() {
                loadSavedConfig(config.name);
            });
            m_savedConfigsMenu->addAction(action);
        }
        
        m_savedConfigsMenu->addSeparator();
        
        // Add delete option
        QAction* deleteAction = new QAction("Delete Config...", this);
        connect(deleteAction, &QAction::triggered, this, [this]() {
            QStringList names;
            for (const auto& c : ConfigManager::instance().getAllConfigs()) {
                names.append(c.name);
            }
            
            bool ok;
            QString selected = QInputDialog::getItem(this, "Delete Configuration",
                "Select configuration to delete:", names, 0, false, &ok);
            
            if (ok && !selected.isEmpty()) {
                ConfigManager::instance().deleteConfig(selected);
                updateSavedConfigsMenu();
                QMessageBox::information(this, "Success", 
                    QString("Configuration '%1' deleted.").arg(selected));
            }
        });
        m_savedConfigsMenu->addAction(deleteAction);
    }
}
```

### Step 5: 更新 CMakeLists.txt
**File**: `CMakeLists.txt`

添加:
```cmake
set(HEADERS
    src/models/ConfigManager.h
    # ... other headers ...
)
```

---

## Acceptance Criteria

- [ ] Save Curve Config弹出名称输入对话框
- [ ] 配置保存到QSettings（重启后仍在）
- [ ] File菜单下出现"Saved Configs"子菜单
- [ ] 子菜单显示所有保存的配置名称
- [ ] 点击配置名称自动新开画布并显示曲线
- [ ] 可以删除配置
- [ ] 构建成功
