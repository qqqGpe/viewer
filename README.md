# LogViewer

A Qt5-based log visualization tool for viewing and analyzing time-series data from CSV and binary files.

![Qt5](https://img.shields.io/badge/Qt-5.x-green.svg)
![C++17](https://img.shields.io/badge/C++-17-blue.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

## Features

- **Multi-format Support**: Parse CSV and custom binary log files
- **Multi-curve Visualization**: Display multiple data series simultaneously
- **Multi-canvas Support**: Create multiple tabs to organize different views
- **Interactive Operations**:
  - Check/uncheck to add/remove curves
  - Double-click to toggle selection
  - Drag and drop variables to canvas
  - Auto-sync selection state across canvases
- **Flexible Zoom**:
  - Mouse wheel: zoom centered on cursor
  - `X` + wheel: horizontal zoom only
  - `V` + wheel: vertical zoom only
  - Middle mouse drag: pan canvas
  - Fit button: auto-fit curves to canvas
- **Coordinate Overlay**: Hover to display coordinates

## Screenshots

> Add your screenshots here

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

If no file is specified, you can open files via the menu.

### CSV Format

The expected CSV format is:

```csv
timestamp,category,value
0.001,sensor1,23.5
0.002,sensor1,24.1
0.001,sensor2,100.2
```

- **timestamp**: Time value (double)
- **category**: Series identifier (string)
- **value**: Data value (double)

The first line is treated as a header and skipped if it contains non-numeric data.

### Binary Format

Binary files use a custom format for efficient storage:

```
Header: "LOG" (3 bytes)
Version: uint32_t (4 bytes)
SeriesCount: uint32_t (4 bytes)
Per Series:
  CategoryNameLength: uint16_t (2 bytes)
  CategoryName: char[] (variable)
  SeriesNameLength: uint16_t (2 bytes)
  SeriesName: char[] (variable)
  DataPointCount: uint32_t (4 bytes)
  DataPoints: { timestamp: double, value: double }[] (16 bytes each)
```

## Keyboard & Mouse Shortcuts

| Action | Input |
|--------|-------|
| Zoom in/out | Mouse wheel |
| Horizontal zoom | Hold `X` + wheel |
| Vertical zoom | Hold `V` + wheel |
| Pan canvas | Middle mouse button drag |
| Toggle curve | Double-click variable |
| Add curve | Drag variable to canvas |

## Project Structure

```
LogViewer/
├── CMakeLists.txt          # Build configuration
├── src/
│   ├── main.cpp            # Application entry
│   ├── MainWindow.h/cpp    # Main window
│   ├── LeftPane.h/cpp      # Left panel with data tree
│   ├── CanvasTabWidget.h/cpp # Canvas tab management
│   ├── ChartWidget.h/cpp   # Chart visualization
│   ├── DataParser.h/cpp    # File parsing
│   ├── DataModel.h/cpp     # Data model
│   └── models/
│       ├── SeriesData.h    # Data series structure
│       └── CategoryItem.h  # Category item structure
└── README.md
```

## License

MIT License
