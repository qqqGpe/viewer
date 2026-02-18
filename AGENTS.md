# LogViewer 项目说明

## 项目概述
Qt5 C++ 日志查看器应用，支持 CSV/二进制文件解析和多曲线可视化。

## 项目结构
```
viewer/
├── CMakeLists.txt              # CMake 构建配置
├── build/                      # 构建目录
├── src/                        # 源代码
│   ├── main.cpp               # 程序入口
│   ├── MainWindow.h/cpp       # 主窗口
│   ├── LeftPane.h/cpp         # 左侧数据面板（树形结构）
│   ├── CanvasTabWidget.h/cpp  # 画布标签页管理
│   ├── ChartWidget.h/cpp      # 图表组件
│   ├── DataModel.h/cpp        # 数据模型
│   ├── DataParser.h/cpp       # 文件解析器
│   └── models/
│       ├── SeriesData.h       # 数据系列结构
│       └── CategoryItem.h     # 分类项结构（展开状态）
└── viewer/                     # 资源目录
```

## 已实现功能

### 数据加载
- 支持 CSV 文件格式（时间戳, 分类, 数值）
- 支持二进制文件格式
- 数据按分类自动分组

### 曲线显示
- 多曲线同时显示，自动调整坐标轴范围
- 每个曲线固定颜色（基于名称哈希）
- 支持多画布标签页
- 鼠标悬停显示坐标信息

### 交互操作
- **勾选/取消勾选**：添加/移除曲线到当前画布
- **双击变量**：切换勾选状态
- **拖拽变量到画布**：添加曲线并自动勾选
- **切换画布**：自动同步勾选状态

### 缩放功能
- **鼠标滚轮**：以鼠标位置为中心整体缩放
- **X 键 + 鼠标滚轮**：水平方向缩放
- **V 键 + 鼠标滚轮**：垂直方向缩放
- **鼠标中键拖动**：平移画布
- **Fit 按钮（右上角四角图标）**：曲线自适应铺满画布

## 关键代码位置

### 拖拽功能
- `LeftPane.h/cpp`: `DragTreeWidget` 类实现拖拽发起
- `ChartWidget.h/cpp`: `CustomChartView` 类实现拖放接收

### 勾选状态同步
- `LeftPane.cpp`: `setSeriesSelected()`, `syncSelectionWithSeries()`
- `MainWindow.cpp`: `onCurrentCanvasChanged()`, `onCanvasCreated()`

### 缩放功能
- `ChartWidget.cpp`: `CustomChartView::wheelEvent()`, `mousePressEvent()`, `mouseMoveEvent()`, `mouseReleaseEvent()`
- `ChartWidget.cpp`: `FitButton` 类实现适配按钮

### 曲线显示
- `ChartWidget.cpp`: `addSeries()`, `updateAxesRange()`
- `ChartWidget.cpp`: `CoordinateOverlay` 类实现坐标显示

## 构建与运行

```bash
cd /home/gao/ws/viewer/build
cmake ..
make -j4
./LogViewer test_data.csv
```

## 测试数据
项目包含 `test_data.csv` 测试文件，包含 3 个传感器数据系列。

## 待开发功能
- 二进制文件导出
- 更多图表类型
- 数据导出功能
