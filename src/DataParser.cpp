#include "DataParser.h"
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QDataStream>
#include <QDebug>
#include <QFileInfo>

namespace Viewer {

QList<SeriesData> DataParser::parseCSV(const QString& filePath, QString& error) {
    qDebug() << "DataParser::parseCSV:" << filePath;
    QList<SeriesData> seriesList;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = QString("Failed to open file: %1").arg(filePath);
        return seriesList;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");

    // Read first line to check for header
    QString firstLine = in.readLine();

    bool hasHeader = isHeaderLine(firstLine);
    int lineNum = hasHeader ? 2 : 1;

    // If header was present, firstLine contains the header, otherwise firstLine is data
    QString currentLine = firstLine;
    if (!hasHeader && !currentLine.isEmpty()) {
        // Process first data line
    } else if (hasHeader) {
        currentLine = in.readLine();
        lineNum++;
    }

    QHash<QString, SeriesData*> seriesMap;

    while (!currentLine.isEmpty()) {
        QStringList parts = currentLine.split(',', QString::SkipEmptyParts);
        if (parts.size() >= 3) {
            bool timestampOk = false, valueOk = false;
            double timestamp = parts[0].trimmed().toDouble(&timestampOk);
            QString category = parts[1].trimmed();
            double value = parts[2].trimmed().toDouble(&valueOk);

            QString seriesName = QString("%1_%2").arg(category, parts.size() > 3 ? parts[3].trimmed() : "data");

            qDebug() << "    Parsing line:" << currentLine << "->" << seriesName << "timestamp:" << timestamp << "value:" << value;

            if (timestampOk && valueOk && !category.isEmpty()) {
                if (!seriesMap.contains(seriesName)) {
                    SeriesData* series = new SeriesData(seriesName, category);
                    seriesMap[seriesName] = series;
                }
                seriesMap[seriesName]->timestamps.append(timestamp);
                seriesMap[seriesName]->values.append(value);
            }
        }

        currentLine = in.readLine();
        lineNum++;
    }

    // Now copy data from map to list
    for (auto it = seriesMap.begin(); it != seriesMap.end(); ++it) {
        seriesList.append(*it.value());
    }

    // Clean up pointers
    qDeleteAll(seriesMap);

    file.close();

    if (seriesList.isEmpty()) {
        error = QString("No valid data found in file");
    }

    qDebug() << "  Parsed" << seriesList.size() << "series";
    for (const auto& series : seriesList) {
        qDebug() << "    -" << series.name << "(category:" << series.category << ", points:" << series.pointCount() << ")";
    }

    return seriesList;
}

QList<SeriesData> DataParser::parseBinary(const QString& filePath, QString& error) {
    QList<SeriesData> seriesList;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        error = QString("Failed to open file: %1").arg(filePath);
        return seriesList;
    }

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);
    in.setFloatingPointPrecision(QDataStream::DoublePrecision);

    // Read header
    char header[4] = {0};
    in.readRawData(header, 3);

    if (QString::fromLatin1(header, 3) != "LOG") {
        error = QString("Invalid binary file format");
        file.close();
        return seriesList;
    }

    // Read version
    quint32 version;
    in >> version;

    if (version > 1) {
        error = QString("Unsupported file version: %1").arg(version);
        file.close();
        return seriesList;
    }

    // Read series count
    quint32 seriesCount;
    in >> seriesCount;

    for (quint32 i = 0; i < seriesCount; ++i) {
        // Read category name
        quint16 categoryLen;
        in >> categoryLen;

        QByteArray categoryBytes(categoryLen, 0);
        in.readRawData(categoryBytes.data(), categoryLen);
        QString category = QString::fromUtf8(categoryBytes);

        // Read series name
        quint16 nameLen;
        in >> nameLen;

        QByteArray nameBytes(nameLen, 0);
        in.readRawData(nameBytes.data(), nameLen);
        QString seriesName = QString::fromUtf8(nameBytes);

        // Read data points
        quint32 pointCount;
        in >> pointCount;

        SeriesData series(seriesName, category);
        series.timestamps.reserve(pointCount);
        series.values.reserve(pointCount);

        for (quint32 j = 0; j < pointCount; ++j) {
            double timestamp, value;
            in >> timestamp >> value;
            series.timestamps.append(timestamp);
            series.values.append(value);
        }

        seriesList.append(series);
    }

    file.close();

    return seriesList;
}

bool DataParser::exportBinary(const QList<SeriesData>& series, const QString& filePath, QString& error) {
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        error = QString("Failed to create file: %1").arg(filePath);
        return false;
    }

    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);
    out.setFloatingPointPrecision(QDataStream::DoublePrecision);

    // Write header
    out.writeRawData("LOG", 3);

    // Write version
    out << static_cast<quint32>(1);

    // Write series count
    out << static_cast<quint32>(series.size());

    // Write each series
    for (const auto& s : series) {
        // Write category name
        QByteArray categoryBytes = s.category.toUtf8();
        out << static_cast<quint16>(categoryBytes.size());
        out.writeRawData(categoryBytes.constData(), categoryBytes.size());

        // Write series name
        QByteArray nameBytes = s.name.toUtf8();
        out << static_cast<quint16>(nameBytes.size());
        out.writeRawData(nameBytes.constData(), nameBytes.size());

        // Write data points
        out << static_cast<quint32>(s.pointCount());

        for (int i = 0; i < s.pointCount(); ++i) {
            out << s.timestamps[i] << s.values[i];
        }
    }

    file.close();
    return true;
}

bool DataParser::isHeaderLine(const QString& line) {
    if (line.isEmpty()) {
        return false;
    }

    QStringList parts = line.split(',', QString::SkipEmptyParts);

    // Check if the line contains non-numeric values in first column
    if (parts.size() >= 1) {
        bool ok = false;
        parts[0].trimmed().toDouble(&ok);
        if (!ok) {
            return true;
        }
    }

    // Check for common header keywords
    if (line.contains("timestamp", Qt::CaseInsensitive) ||
        line.contains("time", Qt::CaseInsensitive) ||
        line.contains("category", Qt::CaseInsensitive) ||
        line.contains("value", Qt::CaseInsensitive)) {
        return true;
    }

    return false;
}

} // namespace Viewer
