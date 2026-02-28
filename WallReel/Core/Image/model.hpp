#ifndef WALLREEL_IMAGEMODEL_HPP
#define WALLREEL_IMAGEMODEL_HPP

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include "Config/data.hpp"
#include "data.hpp"

namespace WallReel::Core::Image {

class Model : public QAbstractListModel {
    Q_OBJECT

  public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        UrlRole,
        PathRole,
        NameRole,
        SizeRole,
        DateRole,
        DomColorRole,
    };

    QHash<int, QByteArray> roleNames() const override {
        return {
            {IdRole, "imgId"},
            {UrlRole, "imgUrl"},  // file:///...
            {PathRole, "imgPath"},
            {NameRole, "imgName"},
            {SizeRole, "imgSize"},
            {DateRole, "imgDate"},
            {DomColorRole, "imgDomColor"},
        };
    }

    explicit Model(QObject* parent = nullptr);

    ~Model();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    QVariant dataAt(int index, const QString& roleName) const;

    void clearData();

    void insertData(const QList<Data*>& newData);

  private:
    QList<Data*> m_data;
};

class ProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

  public:
    explicit ProxyModel(QObject* parent = nullptr);

    void setSearchText(const QString& text);

    QString getSearchText() const { return m_searchText; }

    void setSortType(Config::SortType type);

    Config::SortType getSortType() const { return m_sortType; }

    void setSortDescending(bool descending);

    bool isSortDescending() const { return m_sortDescending; }

  protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

    bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;

  private:
    QString m_searchText;
    Config::SortType m_sortType;
    bool m_sortDescending;
};

}  // namespace WallReel::Core::Image

#endif  // WALLREEL_IMAGEMODEL_HPP
