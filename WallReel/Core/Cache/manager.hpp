#ifndef WALLREEL_CACHE_MANAGER_HPP
#define WALLREEL_CACHE_MANAGER_HPP

#include <QDir>
#include <QFileInfo>
#include <QFuture>
#include <QMutex>
#include <QSet>
#include <QtSql>

#include "types.hpp"

namespace WallReel::Core::Cache {

class Manager {
  public:
    static QString cacheKey(const QFileInfo& fileInfo, const QSize& imageSize);

    Manager(const QDir& cacheDir, int maxEntries = 1000);

    ~Manager();

    void evictOldEntries();

    void clearCache(Type type = Type::Image | Type::Color);

    QColor getColor(const QString& key, const std::function<QColor()>& computeFunc = nullptr);

    QFileInfo getImage(const QString& key, const std::function<QImage()>& computeFunc = nullptr);

    QString getSetting(SettingsType key, const std::function<QString()>& computeFunc = nullptr);

    void storeSetting(SettingsType key, const QString& value);

  private:
    QDir m_cacheDir;
    int m_maxEntries;
    QString m_dbPath;
    QString m_connectionPrefix;

    mutable QMutex m_connectionsMutex;
    mutable QSet<QString> m_connectionNames;

    mutable QMutex m_hotKeysMutex;
    mutable QSet<QString> m_hotColorKeys;
    mutable QSet<QString> m_hotImageKeys;

    QFuture<void> m_cleanupFuture;

    QSqlDatabase _db() const;
    void _setupTables(QSqlDatabase& db) const;
    void _runCleanup();
};

}  // namespace WallReel::Core::Cache

#endif  // WALLREEL_CACHE_MANAGER_HPP
