# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

```bash
cd /home/gao/3rd_party/viewer/build
cmake ..
make -j4
./LogViewer test_data.csv
```

No tests exist. Verification is done by running the binary manually.

## Architecture

**Namespace**: All code lives in `Viewer::`.

**Data flow**:
```
DataParser (static) → DataModel (QObject, signals) → MainWindow
                                                    ↓
                                              LeftPane  CanvasTabWidget
                                                    ↓        ↓
                                              (checkbox sync) ChartWidget(s)
```

**Component responsibilities**:
- `DataModel` — owns all loaded data (`QMap<filePath, QList<SeriesData>>`), emits `dataLoaded`, `fileAdded`, `fileRemoved`, `dataCleared`
- `LeftPane` — tree widget (File → Category → Series), with checkboxes that drive add/remove of curves. Emits `seriesSelected(QStringList)` / `seriesDeselected(QStringList)`. Has `syncSelectionWithSeries(QStringList)` to reflect a canvas's current series back onto the checkboxes.
- `CanvasTabWidget` — manages multiple `ChartWidget` tabs. Routes series add/remove to the active tab.
- `ChartWidget` — wraps `QChart` + `CustomChartView`. Owns `QLineSeries` and optional `QScatterSeries` per curve. Emits `seriesRemoved(name)` when a curve is truly deleted.
- `MainWindow` — wires all signals together. Critical connections: tab switch → `syncSelectionWithSeries`; `seriesRemoved` → uncheck in LeftPane; `seriesDropped` (drag) → check in LeftPane.

## Checkbox ↔ Canvas Invariant

The checkbox state in LeftPane, the curves visible in ChartWidget, and the legend labels at the bottom of the chart must always be in strict three-way sync. This is the most important invariant:

- **Tab switch** (`onCurrentCanvasChanged`): calls `syncSelectionWithSeries(canvas->getSeriesNames())` to update all checkboxes.
- **`addSeries` replacing an existing series**: must NOT call `removeSeries()` internally (which emits `seriesRemoved` and unchecks the checkbox). Remove the old series data directly without the signal.
- **File close** (`onFileRemoved`): must remove orphaned series (those no longer in DataModel) from ALL canvases.
- **Data clear** (`onDataCleared`): must clear ALL canvases, not just the current one.

## CSV Formats

Two formats are auto-detected:

1. **Standard**: `timestamp,category,value[,seriesName]` — series name defaults to `category_data`
2. **TUM** (lines starting with `#`): space-separated columns; first line is `# col0 col1 col2 ...`; each non-timestamp column becomes a series named by the header.

Both formats normalize timestamps to start at 0 (subtract `minTimestamp`).

## Key Enums & Structs

- `SeriesData` (`models/SeriesData.h`): `name`, `category`, `timestamps[]`, `values[]`
- `SeriesStyle` (`ChartWidget.h`): `pointShape`, `pointSize`, `lineStyle`, `lineWidth`
- `CurveConfig` (`models/CurveConfig.h`): named preset of `seriesNames[]`, JSON-serializable
- `ConfigManager` (`models/ConfigManager.h`): singleton, persists `CurveConfig` via `QSettings`

## Zoom & Interaction (CustomChartView)

- Scroll wheel: zoom centered on cursor
- `X` + wheel: horizontal zoom only
- `V` + wheel: vertical zoom only
- Middle-drag: pan
- Left-drag (≥10px threshold): rectangle rubber-band zoom
- Drop: emits `seriesDropped(seriesName)` via MIME type `application/x-series-name`
