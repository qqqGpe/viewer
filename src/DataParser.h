#ifndef DATAPARSER_H
#define DATAPARSER_H

#include "models/SeriesData.h"
#include <QList>
#include <QString>

namespace Viewer {

class DataParser {
public:
    // Parse CSV format text file
    // Expected format: timestamp,category,value
    // or with header: timestamp,category,value (header will be skipped)
    static QList<SeriesData> parseCSV(const QString& filePath, QString& error);

    // Parse binary log file
    // Binary format:
    // Header: "LOG" (3 bytes)
    // Version: uint32_t (4 bytes)
    // SeriesCount: uint32_t (4 bytes)
    // PerSeries:
    //   CategoryNameLength: uint16_t (2 bytes)
    //   CategoryName: char[] (variable)
    //   SeriesNameLength: uint16_t (2 bytes)
    //   SeriesName: char[] (variable)
    //   DataPointCount: uint32_t (4 bytes)
    //   DataPoints: struct { timestamp: double, value: double }[] (16 bytes each)
    static QList<SeriesData> parseBinary(const QString& filePath, QString& error);

    // Export to binary format
    static bool exportBinary(const QList<SeriesData>& series, const QString& filePath, QString& error);

private:
    static bool isHeaderLine(const QString& line);
};

} // namespace Viewer

#endif // DATAPARSER_H
