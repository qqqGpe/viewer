# LogViewer 配置功能重构计划

## TL;DR

> **快速摘要**：重构曲线配置功能，改为嵌入式UI面板，只保存曲线名称，存储到 config/config.json

> **交付物**：
> - 左下方面板显示保存的配置列表
> - 配置只包含曲线名称列表
> - 点击配置从当前数据加载曲线到新画布
> - 配置文件存储在 config/config.json

> **估计工作量**: Medium
> **并行执行**: YES - 3 waves

---

## Context

### 原始需求
用户希望修改配置功能：
1. 保存的配置直接显示在界面左下侧，形成嵌入式窗口
2. 曲线的配置只保存需要加载曲线的名称，不保存文件路径
3. 点击配置时从当前数据包中自动查找对应名称的曲线并画在新画布上
4. 配置保存到 config/config.json 文件

### 现有实现分析
- `CurveConfig` 结构体：包含 `name`, `sourceFile`, `seriesNames` 三个字段
- `ConfigManager`：使用 QSettings 保存到系统注册表/macOS plist
- `MainWindow`：通过 "Saved Configs" 菜单下拉显示配置
- 加载配置时会尝试加载 sourceFile，然后添加曲线

### 调研发现
- 用户明确要求使用 JSON 文件存储，便于手动查看和编辑
- 当前 `sourceFile` 字段需要移除
- 需要新增嵌入式 UI 面板组件

---

## Work Objectives

### 核心目标
重构配置管理功能，使其更符合用户工作流：
1. 配置只保存曲线名称，从当前数据匹配
2. UI 改为嵌入式面板而非菜单
3. 使用 JSON 文件存储

### 具体交付物
- [x] 修改 CurveConfig 结构体，移除 sourceFile
- [x] 修改 ConfigManager 读写 config/config.json
- [x] 在 MainWindow 左下侧添加配置面板
- [x] 修改加载配置逻辑，从当前数据匹配曲线
- [x] 构建验证

### 定义完成
- [ ] 配置文件正确创建在 config/config.json
- [ ] 左下方面板显示所有保存的配置
- [ ] 点击配置按钮创建新画布并加载匹配的曲线
- [ ] 找不到匹配曲线时显示提示信息

---

## Verification Strategy

### Test Decision
- **Infrastructure exists**: NO (Qt5 C++ 项目，无单元测试框架)
- **Automated tests**: NO
- **Agent-Executed QA**: 手动构建和运行验证

### QA Policy
每个任务完成后执行：
1. `cd build && cmake .. && make -j4` 编译检查
2. 运行程序验证功能

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (并行 - 数据结构修改):
├── Task 1: 修改 CurveConfig.h 移除 sourceFile 字段
├── Task 2: 修改 ConfigManager 读写 config/config.json
└── Task 3: 修改 MainWindow 相关代码适配新结构

Wave 2 (串行 - UI 面板):
└── Task 4: 在 MainWindow 左下侧添加配置面板

Wave 3 (串行 - 逻辑修改):
└── Task 5: 修改加载配置逻辑，从当前数据匹配曲线

Wave 4 (验证):
└── Task 6: 构建验证
```

### 依赖矩阵
- Task 1: — — 2, 3
- Task 2: 1 — 3, 4
- Task 3: 1, 2 — 4, 5
- Task 4: 2, 3 — 5
- Task 5: 3, 4 — 6
- Task 6: 1, 2, 3, 4, 5 — —

---

## TODOs

- [ ] 1. 修改 CurveConfig 结构体，移除 sourceFile 字段

  **What to do**:
  - 编辑 `src/models/CurveConfig.h`
  - 移除 `QString sourceFile;` 字段
  - 修改 `toJson()` 和 `fromJson()` 方法，移除 sourceFile 相关代码
  
  **Must NOT do**:
  - 不要添加新的字段

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []
  - Reason: 简单的单文件修改

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Task 2, 3)

  **References**:
  - `src/models/CurveConfig.h` - 需要修改的文件

  **Acceptance Criteria**:
  - [ ] CurveConfig 不包含 sourceFile 字段
  - [ ] JSON 序列化不包含 sourceFile

  **Commit**: YES
  - Message: `refactor(config): remove sourceFile from CurveConfig`
  - Files: `src/models/CurveConfig.h`

---

- [ ] 2. 修改 ConfigManager 读写 config/config.json

  **What to do**:
  - 编辑 `src/models/ConfigManager.cpp`
  - 将保存位置从 QSettings 改为 JSON 文件
  - 确保 config 目录存在，不存在则创建
  - 文件路径: `./config/config.json` (相对于程序运行目录)
  
  **Must NOT do**:
  - 不要修改 ConfigManager 的公共接口

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []
  - Reason: 文件 IO 修改

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Task 1, 3)

  **References**:
  - `src/models/ConfigManager.cpp` - 修改 saveToSettings 和 loadFromSettings

  **Acceptance Criteria**:
  - [ ] 配置文件保存在 config/config.json
  - [ ] 目录不存在时自动创建

  **Commit**: YES
  - Message: `refactor(config): save to config/config.json instead of QSettings`
  - Files: `src/models/ConfigManager.cpp`

---

- [ ] 3. 修改 MainWindow 保存配置逻辑

  **What to do**:
  - 编辑 `src/MainWindow.cpp`
  - 修改 `onSaveConfig()` 方法，不再保存 sourceFile
  - 修改 `loadSavedConfig()` 方法，移除加载 sourceFile 的逻辑
  
  **Must NOT do**:
  - 不要修改菜单结构（后续在 Task 4 处理）

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []
  - Reason: 简单的逻辑修改

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Task 1, 2)

  **References**:
  - `src/MainWindow.cpp:305-329` - onSaveConfig
  - `src/MainWindow.cpp:378-410` - loadSavedConfig

  **Acceptance Criteria**:
  - [ ] 保存配置时不保存 sourceFile
  - [ ] 加载配置时不尝试加载文件

  **Commit**: YES
  - Message: `refactor(config): remove sourceFile from save/load logic`
  - Files: `src/MainWindow.cpp`

---

- [ ] 4. 在 MainWindow 左下侧添加配置面板

  **What to do**:
  - 编辑 `src/MainWindow.h` - 添加配置面板成员变量
  - 编辑 `src/MainWindow.cpp`:
    - 添加 `ConfigPanelWidget` 类或使用现有组件
    - 在 `setupLayout()` 中将配置面板添加到左下侧
    - 使用 QSplitter 或嵌套 layout 实现
    - 面板包含: 配置列表 (QListWidget 或 QList<QPushButton>)、保存按钮、删除按钮
  - 配置面板需要响应 ConfigManager::configsChanged() 信号更新显示
  
  **Must NOT do**:
  - 不要删除菜单中的配置项（可以保留或移除）

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`
  - **Skills**: []
  - Reason: UI 组件开发

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Sequential**: After Wave 1

  **References**:
  - `src/MainWindow.cpp:201-210` - setupLayout()
  - `src/MainWindow.h` - 成员变量声明

  **Acceptance Criteria**:
  - [ ] 面板显示在左下侧
  - [ ] 显示所有保存的配置名称
  - [ ] 点击配置名称加载曲线
  - [ ] 有删除按钮

  **Commit**: YES
  - Message: `feat(config): add embedded config panel to left-bottom`
  - Files: `src/MainWindow.h`, `src/MainWindow.cpp`

---

- [ ] 5. 修改加载配置逻辑，从当前数据匹配曲线

  **What to do**:
  - 编辑 `src/MainWindow.cpp`
  - 修改 `loadSavedConfig()` 方法:
    - 不再尝试加载文件
    - 直接从 m_dataModel 获取当前数据
    - 检查曲线名称是否存在
    - 创建新画布，添加存在的曲线
    - 如果某些曲线不存在，显示提示
  
  **Must NOT do**:
  - 不要修改其他功能

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []
  - Reason: 逻辑修改

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Sequential**: After Task 4

  **References**:
  - `src/MainWindow.cpp:378-410` - loadSavedConfig()

  **Acceptance Criteria**:
  - [ ] 点击配置创建新画布
  - [ ] 只添加当前数据中存在的曲线
  - [ ] 提示哪些曲线未找到

  **Commit**: YES
  - Message: `refactor(config): load curves from current data by name`
  - Files: `src/MainWindow.cpp`

---

- [ ] 6. 构建验证

  **What to do**:
  - 运行构建命令验证编译通过
  - `cd /home/gao/3rd_party/viewer/build && cmake .. && make -j4`
  
  **Must NOT do**:
  - 不要修改任何文件

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []
  - Reason: 验证任务

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Sequential**: Final task

  **Acceptance Criteria**:
  - [ ] 编译成功，无错误
  - [ ] 可执行文件生成

  **Commit**: NO

---

## Final Verification Wave

- [ ] F1. **Plan Compliance Audit** — `oracle`
  - 读取计划文件，验证每个任务已实现
  - 验证所有 Must Have 存在
  - 验证没有添加 Must NOT Have 的内容

- [ ] F2. **Build Verification** — `unspecified-high`
  - 运行 cmake 和 make
  - 验证编译通过

- [ ] F3. **Manual QA** — `unspecified-high`
  - 运行程序
  - 验证配置面板显示在左下侧
  - 验证保存配置到 config/config.json
  - 验证加载配置从当前数据匹配

---

## Success Criteria

### Verification Commands
```bash
cd /home/gao/3rd_party/viewer/build
cmake ..
make -j4
```

### Final Checklist
- [ ] CurveConfig 不包含 sourceFile
- [ ] 配置保存在 config/config.json
- [ ] 左下方面板显示配置列表
- [ ] 点击配置加载匹配的曲线到新画布
- [ ] 编译通过
