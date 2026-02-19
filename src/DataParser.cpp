#include "DataParser.h"
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QDataStream>
#include <QDebug>
#include <QFileInfo>
#include <limits>

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

    QString firstLine = in.readLine().trimmed();

    bool isTUMFormat = firstLine.startsWith('#');
    QStringList columnNames;

    if (isTUMFormat) {
        firstLine.remove(0, 1);
        firstLine = firstLine.trimmed();
        columnNames = firstLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        qDebug() << "  TUM format detected, columns:" << columnNames;
    }

    QList<QStringList> allDataLines;
    double minTimestamp = std::numeric_limits<double>::max();

    QString currentLine = isTUMFormat ? in.readLine() : firstLine;
    while (!currentLine.isNull()) {
        currentLine = currentLine.trimmed();
        if (currentLine.isEmpty() || currentLine.startsWith('#')) {
            currentLine = in.readLine();
            continue;
        }

        QStringList parts;
        if (isTUMFormat) {
            parts = currentLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        } else {
            parts = currentLine.split(',', QString::SkipEmptyParts);
        }

        if (parts.size() >= 2) {
            bool ok = false;
            double ts = parts[0].trimmed().toDouble(&ok);
            if (ok && ts < minTimestamp) {
                minTimestamp = ts;
            }
            allDataLines.append(parts);
        }

        currentLine = in.readLine();
    }

    file.close();

    if (allDataLines.isEmpty()) {
        error = QString("No valid data found in file");
        return seriesList;
    }

    if (minTimestamp == std::numeric_limits<double>::max()) {
        minTimestamp = 0;
    }

    qDebug() << "  Min timestamp:" << minTimestamp << ", aligning to 0";

    if (isTUMFormat) {
        int numColumns = allDataLines.first().size();

        QVector<SeriesData*> seriesOrder;
        seriesOrder.resize(numColumns - 1);

        for (int col = 1; col < numColumns; ++col) {
            QString seriesName;
            if (col - 1 < columnNames.size()) {
                seriesName = columnNames[col];
            } else {
                seriesName = QString("col_%1").arg(col);
            }

            seriesOrder[col - 1] = new SeriesData(seriesName, "data");
        }

        for (const auto& parts : allDataLines) {
            bool tsOk = false;
            double timestamp = parts[0].trimmed().toDouble(&tsOk) - minTimestamp;

            for (int col = 1; col < parts.size() && col < numColumns; ++col) {
                bool valOk = false;
                double value = parts[col].trimmed().toDouble(&valOk);

                if (tsOk && valOk && seriesOrder[col - 1]) {
                    seriesOrder[col - 1]->timestamps.append(timestamp);
                    seriesOrder[col - 1]->values.append(value);
                }
            }
        }

        for (auto* series : seriesOrder) {
            if (series) {
                seriesList.append(*series);
                delete series;
            }
        }
    } else {
        QHash<QString, SeriesData*> seriesMap;

        for (const auto& parts : allDataLines) {
            if (parts.size() >= 3) {
                bool timestampOk = false, valueOk = false;
                double timestamp = parts[0].trimmed().toDouble(&timestampOk) - minTimestamp;
                QString category = parts[1].trimmed();
                double value = parts[2].trimmed().toDouble(&valueOk);

                QString seriesName = QString("%1_%2").arg(category, parts.size() > 3 ? parts[3].trimmed() : "data");

                if (timestampOk && valueOk && !category.isEmpty()) {
                    if (!seriesMap.contains(seriesName)) {
                        SeriesData* series = new SeriesData(seriesName, category);
                        seriesMap[seriesName] = series;
                    }
                    seriesMap[seriesName]->timestamps.append(timestamp);
                    seriesMap[seriesName]->values.append(value);
                }
            }
        }

        for (auto it = seriesMap.begin(); it != seriesMap.end(); ++it) {
            seriesList.append(*it.value());
        }
        qDeleteAll(seriesMap);
    }

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
