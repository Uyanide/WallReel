#ifndef WALLREEL_CACHE_MANAGER_HPP
#define WALLREEL_CACHE_MANAGER_HPP

#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QSet>
#include <QtSql>

#include "types.hpp"

namespace WallReel::Core::Cache {

class Manager {
  public:
    static QString cacheKey(const QFileInfo& fileInfo, const QSize& imageSize);

    Manager(const QDir& cacheDir);

    ~Manager();

    void clearCache(Type type = Type::Image | Type::Color);

    QColor getColor(const QString& key, const std::function<QColor()>& computeFunc = nullptr);

    QFileInfo getImage(const QString& key, const std::function<QImage()>& computeFunc = nullptr);

    QString getSetting(SettingsType key, const std::function<QString()>& computeFunc = nullptr);

    void storeSetting(SettingsType key, const QString& value);

  private:
    QDir m_cacheDir;
    QString m_dbPath;
    QString m_connectionPrefix;

    mutable QMutex m_connectionsMutex;
    mutable QSet<QString> m_connectionNames;

    QSqlDatabase _db() const;
    void _setupTables(QSqlDatabase& db) const;
};

}  // namespace WallReel::Core::Cache

#endif  // WALLREEL_CACHE_MANAGER_HPP
