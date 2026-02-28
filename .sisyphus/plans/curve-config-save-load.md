# Curve Configuration Save/Load Feature

## TL;DR

> **Quick Summary**: Add ability to save current curve visibility configuration to JSON file and load it later. Configuration is tied to a data file - only applies when the same file is loaded.
> 
> **Deliverables**:
> - Save Config: Menu action to save current curve selection to JSON file
> - Load Config: Menu action to load curve config from JSON file
> - File association: Config remembers which data file it belongs to
> 
> **Estimated Effort**: Short
> **Parallel Execution**: NO - sequential (small feature)
> **Critical Path**: Define config structure → Implement save → Implement load → Test

---

## Context

### Original Request
User wants to save curve configurations in Viewer to quickly restore curve visibility. For example, after displaying some curves from a data file, they want to save the curve names so they can quickly view them when opening the same or other data files later.

### Interview Summary
**Key Discussions**:
- **Save Format**: JSON file (human readable, version control friendly)
- **Load Trigger**: Manual file selection via menu
- **File Association**: Config is tied to a data file - only applies when the same file is loaded

### Research Findings
- **Curve storage**: `DataModel` holds all series data per file (`m_fileSeries`)
- **Selection state**: `LeftPane::m_selectedSeries` holds currently checked curves in tree
- **Per-canvas selection**: Each `ChartWidget` maintains its own visible series set
- **No existing config mechanism**: No QSettings usage found
- **Style available**: Series style (point shape, size, line style, width) can also be saved

---

## Work Objectives

### Core Objective
Implement save/load functionality for curve visibility configurations.

### Concrete Deliverables
1. **Save Config Action** - Menu item to save current curve selection to JSON file
2. **Load Config Action** - Menu item to load curve config from JSON file  
3. **File Association Logic** - Config stores source data filename; load only applies when matching file is open

### Definition of Done
- [ ] Save config produces valid JSON file with curve names and associated data filename
- [ ] Load config checks if matching data file is open; shows warning if not
- [ ] Load config restores curve checkboxes in left pane tree
- [ ] Menu items visible in File menu

### Must Have
- Save current curve selection to JSON
- Load curve selection from JSON (manual file picker)
- Associate config with source data file path
- Warn user if trying to load config for a file that's not open

### Must NOT Have
- Auto-load config on file open (user chooses manually)
- Config stored outside user-chosen location (save to any location user picks)

---

## Verification Strategy

### Test Decision
- **Infrastructure exists**: NO (no test framework in this C++ Qt project)
- **Agent-Executed QA**: Manual verification by running the app and testing save/load

### QA Policy
Every task includes manual QA scenarios:
- **Save config**: Open file → select some curves → save config → verify JSON file created with correct content
- **Load config with matching file**: Load file → load config → verify curves are checked
- **Load config with non-matching file**: Load config for different file → verify warning shown

---

## Execution Strategy

### Sequential Tasks (no parallelism needed for this small feature)

---

## TODOs

- [ ] 1. Define CurveConfig JSON structure

  **What to do**:
  - Define `CurveConfig` struct to hold:
    - `sourceFile`: The data file path this config belongs to
    - `seriesNames`: List of curve names to select
    - Optional: `styles`: Per-series styling (point shape, size, line style, width)
  - Create a header file `src/models/CurveConfig.h`

  **Must NOT do**:
  - Don't implement serialization yet (just define the struct)

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Simple struct definition, no complex logic
  - **Skills**: []
  
  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Sequential**: Task 1
  - **Blocks**: Task 2

  **References**:
  - `src/models/SeriesData.h:9-31` - Example of simple struct pattern to follow

  **Acceptance Criteria**:
  - [ ] `CurveConfig.h` created with proper struct definition
  - [ ] Struct has fields: sourceFile (QString), seriesNames (QStringList), styles (QMap)

- [ ] 2. Add save config action to MainWindow

  **What to do**:
  - Add `QAction* m_saveConfigAction` member in MainWindow.h
  - Create action in MainWindow.cpp constructor with text "Save Curve Config"
  - Connect to new slot `onSaveConfigClicked()`
  - Implement `onSaveConfigClicked()`:
    - Get current data file path from DataModel
    - Get selected series from LeftPane (`getSelectedSeries()`)
    - Show save file dialog (JSON filter)
    - Serialize config to JSON and save

  **Must NOT do**:
  - Don't implement load yet

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
    - Reason: Requires understanding MainWindow structure and signal/slot patterns
  - **Skills**: []
  
  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Sequential**: Task 2
  - **Blocks**: Task 3

  **References**:
  - `src/MainWindow.cpp:120-125` - Example of creating menu actions
  - `src/MainWindow.cpp:200-210` - Example of file save dialog usage

  **Acceptance Criteria**:
  - [ ] "Save Curve Config" appears in File menu
  - [ ] Clicking opens save file dialog
  - [ ] Saving creates JSON file with correct structure

- [ ] 3. Add load config action to MainWindow

  **What to do**:
  - Add `QAction* m_loadConfigAction` member in MainWindow.h
  - Create action in MainWindow.cpp with text "Load Curve Config"
  - Connect to new slot `onLoadConfigClicked()`
  - Implement `onLoadConfigClicked()`:
    - Show open file dialog (JSON filter)
    - Read and parse JSON file
    - Check if `sourceFile` matches currently open file
    - If match: call `LeftPane::setSelectedSeries()` with the series names
    - If no match: show warning QMessageBox

  **Must NOT do**:
  - Don't modify chart widget (LeftPane selection will trigger canvas update via existing signals)

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
    - Reason: Similar complexity to Task 2
  - **Skills**: []
  
  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Sequential**: Task 3
  - **Blocks**: Task 4

  **References**:
  - `src/LeftPane.h:37` - `getSelectedSeries()` method
  - `src/LeftPane.h:39` - `setSelectedSeries(QStringList)` method
  - `src/MainWindow.cpp:212-220` - Example of open file dialog

  **Acceptance Criteria**:
  - [ ] "Load Curve Config" appears in File menu
  - [ ] Clicking opens file picker for JSON
  - [ ] Loading config for matching file restores checkboxes
  - [ ] Loading config for non-matching file shows warning

- [ ] 4. Build and manual test

  **What to do**:
  - Build the project
  - Test save: Open test_data.csv → select curves → save config → verify JSON
  - Test load: Load same file → load config → verify curves checked
  - Test mismatch: Load different file → load config for original file → verify warning

  **Must NOT do**:
  - None

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Build and basic manual verification
  - **Skills**: []
  
  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Sequential**: Task 4
  - **Blocks**: None (final)

  **References**:
  - `test_data.csv` - Test data file

  **Acceptance Criteria**:
  - [ ] Project builds successfully
  - [ ] Save config works correctly
  - [ ] Load config with matching file works
  - [ ] Load config with non-matching file shows warning

---

## Final Verification Wave

- [ ] F1. **Manual Feature Test** — `unspecified-high`
  Test the complete workflow:
  1. Open test_data.csv
  2. Select 2-3 curves by checking tree items
  3. Click File → Save Curve Config → save as .json
  4. Close and reopen app
  5. Open test_data.csv
  6. Click File → Load Curve Config → select the .json
  7. Verify curves are now checked in the tree
  8. Try loading config when a different file is open → verify warning
  
  Output: `Save [PASS/FAIL] | Load Match [PASS/FAIL] | Load Mismatch [PASS/FAIL] | VERDICT`

---

## Commit Strategy

- **1**: `feat(config): add curve config save/load` — MainWindow.cpp, MainWindow.h, CurveConfig.h

---

## Success Criteria

### Verification Commands
```bash
# Build
cd build && cmake .. && make -j4
```

### Final Checklist
- [ ] "Save Curve Config" menu item exists and works
- [ ] "Load Curve Config" menu item exists and works
- [ ] Config saves to JSON with correct structure
- [ ] Loading applies config only when source file matches
- [ ] Warning shown when source file doesn't match
