#ifndef SERIESDATA_H
#define SERIESDATA_H

#include <QVector>
#include <QString>

namespace Viewer {

struct SeriesData {
    QString name;
    QString category;
    QVector<double> timestamps;
    QVector<double> values;

    SeriesData() = default;
    SeriesData(const QString& name, const QString& category)
        : name(name), category(category) {}

    int pointCount() const {
        return qMin(timestamps.size(), values.size());
    }

    bool isEmpty() const {
        return timestamps.isEmpty() || values.isEmpty();
    }

    void clear() {
        timestamps.clear();
        values.clear();
    }
};

} // namespace Viewer

#endif // SERIESDATA_H
