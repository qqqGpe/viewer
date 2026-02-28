#include "DataModel.h"
#include "DataParser.h"
#include <QDebug>

namespace Viewer {

DataModel::DataModel(QObject* parent)
    : QObject(parent) {
}

void DataModel::loadCSV(const QString& filePath, QString& error) {
    clear();

    QList<SeriesData> newSeries = DataParser::parseCSV(filePath, error);

    if (!error.isEmpty()) {
        emit dataError(error);
        return;
    }

    m_fileSeries[filePath] = newSeries;
    m_currentFilePath = filePath;
    emit dataLoaded(filePath);
}

void DataModel::loadBinary(const QString& filePath, QString& error) {
    clear();

    QList<SeriesData> newSeries = DataParser::parseBinary(filePath, error);

    if (!error.isEmpty()) {
        emit dataError(error);
        return;
    }

    m_fileSeries[filePath] = newSeries;
    m_currentFilePath = filePath;
    emit dataLoaded(filePath);
}

void DataModel::addCSV(const QString& filePath, QString& error) {
    if (m_fileSeries.contains(filePath)) {
        error = QString("File already loaded: %1").arg(filePath);
        emit dataError(error);
        return;
    }

    QList<SeriesData> newSeries = DataParser::parseCSV(filePath, error);

    if (!error.isEmpty()) {
        emit dataError(error);
        return;
    }

    m_fileSeries[filePath] = newSeries;
    m_currentFilePath = filePath;
    emit fileAdded(filePath);
}

void DataModel::addBinary(const QString& filePath, QString& error) {
    if (m_fileSeries.contains(filePath)) {
        error = QString("File already loaded: %1").arg(filePath);
        emit dataError(error);
        return;
    }

    QList<SeriesData> newSeries = DataParser::parseBinary(filePath, error);

    if (!error.isEmpty()) {
        emit dataError(error);
        return;
    }

    m_fileSeries[filePath] = newSeries;
    m_currentFilePath = filePath;
    emit fileAdded(filePath);
}

void DataModel::closeFile(const QString& filePath) {
    if (m_fileSeries.remove(filePath) > 0) {
        if (m_currentFilePath == filePath) {
            m_currentFilePath = m_fileSeries.isEmpty() ? QString() : m_fileSeries.firstKey();
        }
        emit fileRemoved(filePath);
    }
}

void DataModel::clear() {
    m_fileSeries.clear();
    m_currentFilePath.clear();
    emit dataCleared();
}

QList<QString> DataModel::getFileNames() const {
    return m_fileSeries.keys();
}

QList<QString> DataModel::getCategories() const {
    QSet<QString> categorySet;

    for (const auto& seriesList : m_fileSeries) {
        for (const auto& series : seriesList) {
            categorySet.insert(series.category);
        }
    }

    return categorySet.values();
}

QList<QString> DataModel::getCategoriesForFile(const QString& filePath) const {
    QSet<QString> categorySet;

    auto it = m_fileSeries.find(filePath);
    if (it != m_fileSeries.end()) {
        for (const auto& series : it.value()) {
            categorySet.insert(series.category);
        }
    }

    return categorySet.values();
}

QList<SeriesData> DataModel::getSeriesByCategory(const QString& category) const {
    QList<SeriesData> result;

    for (const auto& seriesList : m_fileSeries) {
        for (const auto& series : seriesList) {
            if (series.category == category) {
                result.append(series);
            }
        }
    }

    return result;
}

QList<SeriesData> DataModel::getSeriesByFileAndCategory(const QString& filePath, const QString& category) const {
    QList<SeriesData> result;

    auto it = m_fileSeries.find(filePath);
    if (it != m_fileSeries.end()) {
        for (const auto& series : it.value()) {
            if (series.category == category) {
                result.append(series);
            }
        }
    }

    return result;
}

QList<SeriesData> DataModel::getAllSeries() const {
    QList<SeriesData> result;

    for (const auto& seriesList : m_fileSeries) {
        result.append(seriesList);
    }

    return result;
}

QList<SeriesData> DataModel::getSeriesForFile(const QString& filePath) const {
    return m_fileSeries.value(filePath);
}

int DataModel::getSeriesCount() const {
    int count = 0;
    for (const auto& seriesList : m_fileSeries) {
        count += seriesList.size();
    }
    return count;
}

int DataModel::getFileCount() const {
    return m_fileSeries.size();
}

SeriesData DataModel::getSeriesByName(const QString& name) const {
    for (const auto& seriesList : m_fileSeries) {
        for (const auto& series : seriesList) {
            if (series.name == name) {
                return series;
            }
        }
    }
    return SeriesData();
}

SeriesData DataModel::getSeriesByFileAndName(const QString& filePath, const QString& name) const {
    auto it = m_fileSeries.find(filePath);
    if (it != m_fileSeries.end()) {
        for (const auto& series : it.value()) {
            if (series.name == name) {
                return series;
            }
        }
    }
    return SeriesData();
}

} // namespace Viewer
