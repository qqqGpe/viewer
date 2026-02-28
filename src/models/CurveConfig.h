#ifndef CURVECONFIG_H
#define CURVECONFIG_H

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>

namespace Viewer {

struct CurveConfig {
    QString name;
    QStringList seriesNames;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["name"] = name;
        obj["seriesNames"] = QJsonArray::fromStringList(seriesNames);
        return obj;
    }

    static CurveConfig fromJson(const QJsonObject& obj) {
        CurveConfig cfg;
        cfg.name = obj["name"].toString();
        QJsonArray arr = obj["seriesNames"].toArray();
        for (const QJsonValue& v : arr) {
            cfg.seriesNames.append(v.toString());
        }
        return cfg;
    }
};

} // namespace Viewer

#endif
