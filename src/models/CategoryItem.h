#ifndef CATEGORYITEM_H
#define CATEGORYITEM_H

#include <QString>

namespace Viewer {

struct CategoryItem {
    QString name;
    bool expanded = false;

    CategoryItem() = default;
    explicit CategoryItem(const QString& name) : name(name) {}

    bool operator==(const CategoryItem& other) const {
        return name == other.name;
    }
};

} // namespace Viewer

#endif // CATEGORYITEM_H
