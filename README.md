# LogViewer

A Qt5-based log visualization tool for viewing and analyzing time-series data from CSV and binary files.

![Qt5](https://img.shields.io/badge/Qt-5.x-green.svg)
![C++17](https://img.shields.io/badge/C++-17-blue.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

## Features

- **Multi-format Support**: Parse CSV (standard and TUM format) and custom binary log files
- **Multi-file Loading**: Load multiple files simultaneously; all series are merged into one tree view
- **Multi-canvas Tabs**: Create multiple canvases via the `+` tab; each canvas holds its own set of curves
- **Curve Management**:
  - Check/uncheck series in the tree to add/remove from the active canvas
  - Double-click a series to toggle it
  - Drag series from the tree onto any canvas
  - Right-click the tree for "Add to Canvas" / "Remove from Canvas" context menu
  - Close a single file via right-click → "Close File"
- **Curve Style Controls** (toolbar above tabs):
  - Point shape: None / Circle / Square / Triangle / Star
  - Point size and line width (spinboxes)
  - Line style: Solid / Dash / Dot / DashDot
- **Saved Configs**: Save the current canvas's curves as a named config; double-click in the "Saved Configs" panel to recreate that view on a new canvas
- **Flexible Zoom & Pan**:
  - Mouse wheel: zoom centered on cursor
  - `X` + wheel: horizontal zoom only
  - `V` + wheel: vertical zoom only
  - Left-drag (≥10 px): rectangle rubber-band zoom
  - Middle-drag: pan canvas
  - Fit button (top-right corner): auto-fit all curves
- **Coordinate Overlay**: Click or hover a curve to display the nearest point's coordinates
- **Canvas Rename**: Right-click any canvas tab → "Rename"
- **Canvas Save**: Right-click any canvas tab → "Save" to save its curves as a named config

## Requirements

- Qt5 (Core, Widgets, Charts modules)
- CMake 3.5+
- C++17 compiler

## Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Usage

```bash
./LogViewer [file]
```

If no file is specified, open files via **File → Open CSV** or **File → Open Binary**.

## CSV Formats

Two formats are auto-detected:

### Standard Format

```
timestamp,category,value[,seriesName]
0.001,sensor1,23.5
0.002,sensor1,24.1
0.001,sensor2,100.2
```

- `timestamp`: time value (double)
- `category`: series group label
- `value`: data value (double)
- `seriesName` (optional): display name; defaults to `category_data`

### TUM Format

Lines starting with `#` indicate TUM format. The first `#` line is the column header:

```
# timestamp x y z
0.001 1.0 2.0 3.0
0.002 1.1 2.1 3.1
```

Each non-timestamp column becomes a separate series named by its header.

Both formats normalize timestamps so the first sample starts at 0.

## Binary Format

```
Header:      "LOG" (3 bytes)
Version:     uint32_t (4 bytes)
SeriesCount: uint32_t (4 bytes)
Per series:
  CategoryNameLength: uint16_t
  CategoryName:       char[]
  SeriesNameLength:   uint16_t
  SeriesName:         char[]
  DataPointCount:     uint32_t
  DataPoints:         { timestamp: double, value: double }[] (16 bytes each)
```

## Keyboard & Mouse Reference

| Action | Input |
|--------|-------|
| Zoom in/out | Mouse wheel |
| Horizontal zoom only | Hold `X` + wheel |
| Vertical zoom only | Hold `V` + wheel |
| Rectangle zoom | Left-drag ≥ 10 px |
| Pan canvas | Middle mouse button drag |
| Auto-fit curves | Fit button (top-right) |
| Toggle curve | Double-click series in tree |
| Add curve via drag | Drag series onto canvas |
| New canvas | Click `+` tab |
| Rename / Save canvas | Right-click canvas tab |
| Close a file | Right-click file in tree → "Close File" |

## Project Structure

```
LogViewer/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── MainWindow.h/cpp        # Main window, menus, signal wiring
│   ├── LeftPane.h/cpp          # Data-file tree with checkboxes
│   ├── FavorPanel.h/cpp        # Saved configs panel
│   ├── CanvasTabWidget.h/cpp   # Tab management, curve-style toolbar
│   ├── ChartWidget.h/cpp       # Chart view (CustomChartView, FitButton, CoordinateOverlay)
│   ├── DataParser.h/cpp        # CSV / binary parsing
│   ├── DataModel.h/cpp         # Central data store (signals: dataLoaded, fileAdded, …)
│   └── models/
│       ├── SeriesData.h        # name, category, timestamps[], values[]
│       ├── CategoryItem.h      # Tree category item (expand state)
│       ├── CurveConfig.h       # Named preset: name + seriesNames[]
│       └── ConfigManager.h/cpp # QSettings-based config persistence (singleton)
└── README.md
```

## License

MIT License
