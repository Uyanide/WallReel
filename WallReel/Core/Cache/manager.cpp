#include "manager.hpp"

#include <QCryptographicHash>
#include <QFile>
#include <QImage>
#include <QMutexLocker>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <QtConcurrent>

#include "logger.hpp"

WALLREEL_DECLARE_SENDER("CacheManager")

using namespace Qt::StringLiterals;

namespace WallReel::Core::Cache {

static QLatin1StringView settingKey(SettingsType type) {
    switch (type) {
        case SettingsType::LastSelectedPalette: return "last_selected_palette"_L1;
        case SettingsType::LastSortType: return "last_sort_type"_L1;
        case SettingsType::LastSortDescending: return "last_sort_descending"_L1;
    }
    Q_UNREACHABLE();
}

QString Manager::cacheKey(const QFileInfo& fileInfo, const QSize& imageSize) {
    const QString raw = fileInfo.absoluteFilePath() +
                        QString::number(fileInfo.lastModified().toMSecsSinceEpoch()) +
                        u'x' + QString::number(imageSize.width()) +
                        u'x' + QString::number(imageSize.height());
    return QString::fromLatin1(
        QCryptographicHash::hash(raw.toUtf8(), QCryptographicHash::Sha256).toHex());
}

Manager::Manager(const QDir& cacheDir, int maxEntries)
    : m_cacheDir(cacheDir),
      m_maxEntries(maxEntries),
      m_dbPath(cacheDir.filePath(u"cache.db"_s)),
      m_connectionPrefix(u"WallReelCache:"_s +
                         QString::fromLatin1(QCryptographicHash::hash(
                                                 m_dbPath.toUtf8(),
                                                 QCryptographicHash::Md5)
                                                 .toHex())) {
    WR_DEBUG(u"Initializing cache db: %1"_s.arg(m_dbPath));
    // Open a connection on the constructing thread so the schema is
    // guaranteed to exist before any worker thread first calls _db().
    _db();
}

void Manager::evictOldEntries() {
    if (m_maxEntries > 0)
        m_cleanupFuture = QtConcurrent::run([this] { _runCleanup(); });
}

Manager::~Manager() {
    // Wait for the background cleanup to finish before tearing down DB connections.
    if (m_cleanupFuture.isValid() && !m_cleanupFuture.isFinished()) {
        WR_DEBUG(u"Waiting for cache cleanup to finish..."_s);
        m_cleanupFuture.waitForFinished();
    }

    QSet<QString> names;
    {
        QMutexLocker lock(&m_connectionsMutex);
        names = std::move(m_connectionNames);
    }
    WR_DEBUG(u"Closing %1 cache db connection(s)"_s.arg(names.size()));
    for (const QString& connName : std::as_const(names)) {
        QSqlDatabase::removeDatabase(connName);
    }
}

void Manager::clearCache(Type type) {
    QSqlDatabase db = _db();
    if (!db.isOpen())
        return;

    if ((type & Type::Image) != Type::None) {
        int removed = 0;
        QSqlQuery selectQuery(db);
        if (selectQuery.exec(u"SELECT file_name FROM image_cache"_s)) {
            while (selectQuery.next()) {
                QFile::remove(m_cacheDir.filePath(selectQuery.value(0).toString()));
                ++removed;
            }
        }
        QSqlQuery(db).exec(u"DELETE FROM image_cache"_s);
        WR_INFO(u"Cleared %1 image cache file(s)"_s.arg(removed));
    }

    if ((type & Type::Color) != Type::None) {
        QSqlQuery(db).exec(u"DELETE FROM color_cache"_s);
        WR_INFO(u"Cleared color cache"_s);
    }

    if ((type & Type::Settings) != Type::None) {
        QSqlQuery(db).exec(u"DELETE FROM settings_cache"_s);
        WR_INFO(u"Cleared settings cache"_s);
    }
}

QColor Manager::getColor(const QString& key, const std::function<QColor()>& computeFunc) {
    QSqlDatabase db = _db();
    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare(u"SELECT r, g, b, a FROM color_cache WHERE key = :key"_s);
        query.bindValue(u":key"_s, key);

        if (query.exec() && query.next()) {
            WR_DEBUG(u"Color cache hit [%1]"_s.arg(key));
            QColor result(
                query.value(0).toInt(),
                query.value(1).toInt(),
                query.value(2).toInt(),
                query.value(3).toInt());
            {
                QMutexLocker lk(&m_hotKeysMutex);
                m_hotColorKeys.insert(key);
            }
            QSqlQuery touchQuery(db);
            touchQuery.prepare(u"UPDATE color_cache SET last_accessed = CURRENT_TIMESTAMP WHERE key = :key"_s);
            touchQuery.bindValue(u":key"_s, key);
            touchQuery.exec();
            return result;
        }
    }

    WR_DEBUG(u"Color cache miss [%1], computing"_s.arg(key));
    if (!computeFunc) {
        WR_WARN(u"No compute function provided for color cache miss [%1]"_s.arg(key));
        return QColor();
    }

    const QColor color = computeFunc();

    if (!color.isValid()) {
        WR_WARN(u"ComputeFunc returned invalid color for key [%1]"_s.arg(key));
        return color;
    }

    if (db.isOpen()) {
        QSqlQuery insertQuery(db);
        insertQuery.prepare(
            u"INSERT OR REPLACE INTO color_cache (key, r, g, b, a, last_accessed) "
            "VALUES (:key, :r, :g, :b, :a, CURRENT_TIMESTAMP)"_s);
        insertQuery.bindValue(u":key"_s, key);
        insertQuery.bindValue(u":r"_s, color.red());
        insertQuery.bindValue(u":g"_s, color.green());
        insertQuery.bindValue(u":b"_s, color.blue());
        insertQuery.bindValue(u":a"_s, color.alpha());
        if (!insertQuery.exec())
            WR_WARN(u"Failed to cache color [%1]: %2"_s
                        .arg(key, insertQuery.lastError().text()));
        else
            WR_DEBUG(u"Color cached [%1]"_s.arg(key));
    }

    return color;
}

QFileInfo Manager::getImage(const QString& key, const std::function<QImage()>& computeFunc) {
    QSqlDatabase db = _db();
    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare(u"SELECT file_name FROM image_cache WHERE key = :key"_s);
        query.bindValue(u":key"_s, key);

        if (query.exec() && query.next()) {
            const QFileInfo cached(m_cacheDir.filePath(query.value(0).toString()));
            if (cached.exists()) {
                WR_DEBUG(u"Image cache hit [%1] -> %2"_s
                             .arg(key, cached.absoluteFilePath()));
                {
                    QMutexLocker lk(&m_hotKeysMutex);
                    m_hotImageKeys.insert(key);
                }
                QSqlQuery touchQuery(db);
                touchQuery.prepare(u"UPDATE image_cache SET last_accessed = CURRENT_TIMESTAMP WHERE key = :key"_s);
                touchQuery.bindValue(u":key"_s, key);
                touchQuery.exec();
                return cached;
            }

            // File was deleted externally — evict the stale DB record.
            WR_WARN(u"Image cache stale, file missing [%1], evicting"_s.arg(key));
            QSqlQuery evict(db);
            evict.prepare(u"DELETE FROM image_cache WHERE key = :key"_s);
            evict.bindValue(u":key"_s, key);
            evict.exec();
        }
    }

    WR_DEBUG(u"Image cache miss [%1], computing"_s.arg(key));
    if (!computeFunc) {
        WR_WARN(u"No compute function provided for image cache miss [%1]"_s.arg(key));
        return QFileInfo{};
    }

    const QImage image = computeFunc();
    if (image.isNull()) {
        WR_WARN(u"ComputeFunc returned null image for key [%1]"_s.arg(key));
        return QFileInfo{};
    }

    const QString fileName = key + u".jpg"_s;
    const QString filePath = m_cacheDir.filePath(fileName);

    if (!image.save(filePath, "JPEG", 85)) {
        WR_WARN(u"Failed to save image to %1"_s.arg(filePath));
        return QFileInfo{};
    }
    WR_DEBUG(u"Image saved to %1"_s.arg(filePath));

    if (db.isOpen()) {
        QSqlQuery insertQuery(db);
        insertQuery.prepare(
            u"INSERT OR REPLACE INTO image_cache (key, file_name, last_accessed) "
            "VALUES (:key, :file_name, CURRENT_TIMESTAMP)"_s);
        insertQuery.bindValue(u":key"_s, key);
        insertQuery.bindValue(u":file_name"_s, fileName);
        if (!insertQuery.exec())
            WR_WARN(u"Failed to record image in db [%1]: %2"_s
                        .arg(key, insertQuery.lastError().text()));
        else {
            QMutexLocker lock(&m_hotKeysMutex);
            m_hotImageKeys.insert(key);
        }
    }

    return QFileInfo(filePath);
}

QString Manager::getSetting(SettingsType key, const std::function<QString()>& computeFunc) {
    QSqlDatabase db                = _db();
    const QLatin1StringView keyStr = settingKey(key);

    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare(u"SELECT value FROM settings_cache WHERE key = :key"_s);
        query.bindValue(u":key"_s, keyStr);

        if (query.exec() && query.next()) {
            WR_DEBUG(u"Settings cache hit [%1]"_s.arg(keyStr));
            return query.value(0).toString();
        }
    }

    WR_DEBUG(u"Settings cache miss [%1], computing"_s.arg(keyStr));
    if (!computeFunc) {
        WR_WARN(u"No compute function provided for settings cache miss [%1]"_s.arg(keyStr));
        return QString{};
    }

    const QString value = computeFunc();

    if (db.isOpen() && !value.isNull()) {
        QSqlQuery insertQuery(db);
        insertQuery.prepare(
            u"INSERT OR REPLACE INTO settings_cache (key, value) "
            "VALUES (:key, :value)"_s);
        insertQuery.bindValue(u":key"_s, keyStr);
        insertQuery.bindValue(u":value"_s, value);
        if (!insertQuery.exec())
            WR_WARN(u"Failed to cache setting [%1]: %2"_s
                        .arg(keyStr, insertQuery.lastError().text()));
        else
            WR_DEBUG(u"Setting cached [%1]"_s.arg(keyStr));
    }

    return value;
}

void Manager::storeSetting(SettingsType key, const QString& value) {
    QSqlDatabase db                = _db();
    const QLatin1StringView keyStr = settingKey(key);

    if (db.isOpen()) {
        if (value.isNull()) {
            QSqlQuery deleteQuery(db);
            deleteQuery.prepare(u"DELETE FROM settings_cache WHERE key = :key"_s);
            deleteQuery.bindValue(u":key"_s, keyStr);
            if (!deleteQuery.exec())
                WR_WARN(u"Failed to delete setting [%1]: %2"_s
                            .arg(keyStr, deleteQuery.lastError().text()));
            else
                WR_DEBUG(u"Setting deleted [%1]"_s.arg(keyStr));
        } else {
            QSqlQuery insertQuery(db);
            insertQuery.prepare(
                u"INSERT OR REPLACE INTO settings_cache (key, value) "
                "VALUES (:key, :value)"_s);
            insertQuery.bindValue(u":key"_s, keyStr);
            insertQuery.bindValue(u":value"_s, value);
            if (!insertQuery.exec())
                WR_WARN(u"Failed to store setting [%1]: %2"_s
                            .arg(keyStr, insertQuery.lastError().text()));
            else
                WR_DEBUG(u"Setting stored [%1]"_s.arg(keyStr));
        }
    }
}

/// Returns an open QSqlDatabase for the calling thread, creating it on first use.
QSqlDatabase Manager::_db() const {
    // thread_local: one slot per OS thread, initialized on first call in that thread.
    // For QThreadPool workers the thread is reused across tasks, so the connection
    // is opened once per worker thread for the lifetime of the Manager.
    thread_local QHash<QString /*connectionPrefix*/, QString /*connName*/> tlsConns;

    auto it = tlsConns.find(m_connectionPrefix);
    if (it != tlsConns.end()) {
        QSqlDatabase db = QSqlDatabase::database(*it, false);
        if (db.isOpen())
            return db;
        // Reopen if closed externally.
        WR_DEBUG(u"Reopening cache db connection [%1]"_s.arg(*it));
        if (!db.open()) {
            WR_WARN(u"Cannot reopen cache database: %1"_s.arg(db.lastError().text()));
            return QSqlDatabase{};
        }
        QSqlQuery q(db);
        q.exec(u"PRAGMA journal_mode=WAL"_s);
        q.exec(u"PRAGMA synchronous=NORMAL"_s);
        return db;
    }

    // First use of this Manager in this thread.
    const QString connName = m_connectionPrefix + u':' +
                             QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    QSqlDatabase db = QSqlDatabase::addDatabase(u"QSQLITE"_s, connName);
    db.setDatabaseName(m_dbPath);

    if (!db.open()) {
        WR_WARN(u"Cannot open cache database %1: %2"_s
                    .arg(m_dbPath, db.lastError().text()));
        db = QSqlDatabase{};
        QSqlDatabase::removeDatabase(connName);
        return QSqlDatabase{};
    }
    WR_DEBUG(u"Opened cache db connection [%1]"_s.arg(connName));

    tlsConns.insert(m_connectionPrefix, connName);
    {
        QMutexLocker lock(&m_connectionsMutex);
        m_connectionNames.insert(connName);
    }

    QSqlQuery q(db);
    q.exec(u"PRAGMA journal_mode=WAL"_s);
    q.exec(u"PRAGMA synchronous=NORMAL"_s);
    q.exec(u"PRAGMA foreign_keys=ON"_s);
    _setupTables(db);

    return db;
}

void Manager::_setupTables(QSqlDatabase& db) const {
    QSqlQuery q(db);
    q.exec(
        u"CREATE TABLE IF NOT EXISTS color_cache ("
        "  key           TEXT    PRIMARY KEY NOT NULL,"
        "  r             INTEGER NOT NULL,"
        "  g             INTEGER NOT NULL,"
        "  b             INTEGER NOT NULL,"
        "  a             INTEGER NOT NULL,"
        "  last_accessed TEXT"
        ")"_s);
    q.exec(
        u"CREATE TABLE IF NOT EXISTS image_cache ("
        "  key           TEXT PRIMARY KEY NOT NULL,"
        "  file_name     TEXT NOT NULL,"
        "  last_accessed TEXT"
        ")"_s);
    q.exec(
        u"CREATE TABLE IF NOT EXISTS settings_cache ("
        "  key   TEXT PRIMARY KEY NOT NULL,"
        "  value TEXT NOT NULL"
        ");"_s);
    // Migrate existing databases that predate the last_accessed column.
    q.exec(u"ALTER TABLE color_cache ADD COLUMN last_accessed TEXT"_s);
    q.exec(u"ALTER TABLE image_cache ADD COLUMN last_accessed TEXT"_s);
}

void Manager::_runCleanup() {
    WR_DEBUG(u"Cache cleanup started (maxEntries=%1)"_s.arg(m_maxEntries));

    QSqlDatabase db = _db();
    if (!db.isOpen())
        return;

    // Evict image_cache rows whose backing file no longer exists
    {
        QSqlQuery sel(db);
        if (sel.exec(u"SELECT key, file_name FROM image_cache"_s)) {
            struct Stale {
                QString key, fileName;
            };

            QList<Stale> stale;
            while (sel.next()) {
                const QString k    = sel.value(0).toString();
                const QString file = sel.value(1).toString();
                if (!QFileInfo::exists(m_cacheDir.filePath(file)))
                    stale.push_back({k, file});
            }
            int evicted = 0;
            for (const auto& s : std::as_const(stale)) {
                {
                    QMutexLocker lk(&m_hotKeysMutex);
                    if (m_hotImageKeys.contains(s.key))
                        continue;
                }
                QSqlQuery del(db);
                del.prepare(u"DELETE FROM image_cache WHERE key = :key"_s);
                del.bindValue(u":key"_s, s.key);
                if (del.exec())
                    ++evicted;
            }
            if (evicted)
                WR_INFO(u"Cleanup evicted %1 stale image cache row(s)"_s.arg(evicted));
        }
    }

    // Trim image_cache to m_maxEntries (oldest last_accessed first)
    {
        QSqlQuery countQ(db);
        if (countQ.exec(u"SELECT COUNT(*) FROM image_cache"_s) && countQ.next()) {
            int excess = countQ.value(0).toInt() - m_maxEntries;
            if (excess > 0) {
                QSqlQuery sel(db);
                sel.exec(u"SELECT key, file_name FROM image_cache ORDER BY last_accessed ASC"_s);
                QList<QPair<QString, QString>> toDelete;
                while (sel.next() && excess > 0) {
                    const QString k = sel.value(0).toString();
                    QMutexLocker lk(&m_hotKeysMutex);
                    if (!m_hotImageKeys.contains(k)) {
                        toDelete.push_back({k, sel.value(1).toString()});
                        --excess;
                    }
                }
                int removed = 0;
                for (const auto& [k, fileName] : std::as_const(toDelete)) {
                    {
                        QMutexLocker lk(&m_hotKeysMutex);
                        if (m_hotImageKeys.contains(k))
                            continue;
                    }
                    QFile::remove(m_cacheDir.filePath(fileName));
                    QSqlQuery del(db);
                    del.prepare(u"DELETE FROM image_cache WHERE key = :key"_s);
                    del.bindValue(u":key"_s, k);
                    if (del.exec())
                        ++removed;
                }
                if (removed)
                    WR_INFO(u"Cleanup trimmed %1 image cache entry(ies)"_s.arg(removed));
            }
        }
    }

    // Trim color_cache to m_maxEntries (oldest last_accessed first)
    {
        QSqlQuery countQ(db);
        if (countQ.exec(u"SELECT COUNT(*) FROM color_cache"_s) && countQ.next()) {
            int excess = countQ.value(0).toInt() - m_maxEntries;
            if (excess > 0) {
                QSqlQuery sel(db);
                sel.exec(u"SELECT key FROM color_cache ORDER BY last_accessed ASC"_s);
                QStringList toDelete;
                while (sel.next() && excess > 0) {
                    const QString k = sel.value(0).toString();
                    QMutexLocker lk(&m_hotKeysMutex);
                    if (!m_hotColorKeys.contains(k)) {
                        toDelete << k;
                        --excess;
                    }
                }
                int removed = 0;
                for (const QString& k : std::as_const(toDelete)) {
                    {
                        QMutexLocker lk(&m_hotKeysMutex);
                        if (m_hotColorKeys.contains(k))
                            continue;
                    }
                    QSqlQuery del(db);
                    del.prepare(u"DELETE FROM color_cache WHERE key = :key"_s);
                    del.bindValue(u":key"_s, k);
                    if (del.exec())
                        ++removed;
                }
                if (removed)
                    WR_INFO(u"Cleanup trimmed %1 color cache entry(ies)"_s.arg(removed));
            }
        }
    }

    WR_DEBUG(u"Cache cleanup complete"_s);
}

}  // namespace WallReel::Core::Cache
