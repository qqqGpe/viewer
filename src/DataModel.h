#ifndef DATAMODEL_H
#define DATAMODEL_H

#include "models/SeriesData.h"
#include <QObject>
#include <QList>
#include <QString>
#include <QMap>

namespace Viewer {

class DataModel : public QObject {
    Q_OBJECT

public:
    explicit DataModel(QObject* parent = nullptr);

    void loadCSV(const QString& filePath, QString& error);
    void loadBinary(const QString& filePath, QString& error);
    void addCSV(const QString& filePath, QString& error);
    void addBinary(const QString& filePath, QString& error);
    void closeFile(const QString& filePath);
    void clear();

    QList<QString> getFileNames() const;
    QList<QString> getCategories() const;
    QList<QString> getCategoriesForFile(const QString& filePath) const;
    QList<SeriesData> getSeriesByCategory(const QString& category) const;
    QList<SeriesData> getSeriesByFileAndCategory(const QString& filePath, const QString& category) const;
    QList<SeriesData> getAllSeries() const;
    QList<SeriesData> getSeriesForFile(const QString& filePath) const;
    int getSeriesCount() const;
    int getFileCount() const;
    SeriesData getSeriesByName(const QString& name) const;
    SeriesData getSeriesByFileAndName(const QString& filePath, const QString& name) const;

signals:
    void dataLoaded(const QString& filePath);
    void fileAdded(const QString& filePath);
    void fileRemoved(const QString& filePath);
    void dataCleared();
    void dataError(const QString& error);

private:
    QMap<QString, QList<SeriesData>> m_fileSeries;
    QString m_currentFilePath;
};

} // namespace Viewer

#endif // DATAMODEL_H
