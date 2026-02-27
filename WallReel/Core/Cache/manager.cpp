#include "manager.hpp"

#include <QCryptographicHash>
#include <QFile>
#include <QImage>
#include <QMutexLocker>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>

#include "logger.hpp"

WALLREEL_DECLARE_SENDER("CacheManager")

using namespace Qt::StringLiterals;

namespace WallReel::Core::Cache {

QString Manager::cacheKey(const QFileInfo& fileInfo, const QSize& imageSize) {
    const QString raw = fileInfo.absoluteFilePath() + QString::number(fileInfo.lastModified().toMSecsSinceEpoch()) + u'x' + QString::number(imageSize.width()) + u'x' + QString::number(imageSize.height());
    return QString::fromLatin1(
        QCryptographicHash::hash(raw.toUtf8(), QCryptographicHash::Sha256).toHex());
}

Manager::Manager(const QDir& cacheDir)
    : m_cacheDir(cacheDir), m_dbPath(cacheDir.filePath(u"cache.db"_s)), m_connectionPrefix(u"WallReelCache:"_s + QString::fromLatin1(QCryptographicHash::hash(m_dbPath.toUtf8(), QCryptographicHash::Md5).toHex())) {
    WR_DEBUG(u"Initializing cache db: %1"_s.arg(m_dbPath));
    // Open a connection on the constructing thread so the schema is
    // guaranteed to exist before any worker thread first calls _db().
    _db();
}

Manager::~Manager() {
    QSet<QString> names;
    {
        QMutexLocker lock(&m_connectionsMutex);
        names = std::move(m_connectionNames);
    }
    WR_DEBUG(u"Closing %1 cache db connection(s)"_s.arg(names.size()));
    for (const QString& connName : std::as_const(names)) {
        {
            // Scope: release the QSqlDatabase copy before removeDatabase()
            QSqlDatabase db = QSqlDatabase::database(connName, false);
            if (db.isOpen())
                db.close();
        }
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
        if (selectQuery.exec(QStringLiteral("SELECT file_name FROM image_cache"))) {
            while (selectQuery.next()) {
                QFile::remove(m_cacheDir.filePath(selectQuery.value(0).toString()));
                ++removed;
            }
        }
        QSqlQuery(db).exec(QStringLiteral("DELETE FROM image_cache"));
        WR_INFO(u"Cleared %1 image cache file(s)"_s.arg(removed));
    }

    if ((type & Type::Color) != Type::None) {
        QSqlQuery(db).exec(QStringLiteral("DELETE FROM color_cache"));
        WR_INFO(u"Cleared color cache"_s);
    }
}

QColor Manager::getColor(const QString& key, const std::function<QColor()>& computeFunc) {
    QSqlDatabase db = _db();
    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare(QStringLiteral(
            "SELECT r, g, b, a FROM color_cache WHERE key = :key"));
        query.bindValue(u":key"_s, key);

        if (query.exec() && query.next()) {
            WR_DEBUG(u"Color cache hit [%1]"_s.arg(key));
            return QColor(
                query.value(0).toInt(),
                query.value(1).toInt(),
                query.value(2).toInt(),
                query.value(3).toInt());
        }
    }

    WR_DEBUG(u"Color cache miss [%1], computing"_s.arg(key));
    const QColor color = computeFunc();

    if (!color.isValid()) {
        WR_WARN(u"ComputeFunc returned invalid color for key [%1]"_s.arg(key));
        return color;
    }

    if (db.isOpen()) {
        QSqlQuery insertQuery(db);
        insertQuery.prepare(QStringLiteral(
            "INSERT OR REPLACE INTO color_cache (key, r, g, b, a) "
            "VALUES (:key, :r, :g, :b, :a)"));
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
        query.prepare(QStringLiteral(
            "SELECT file_name FROM image_cache WHERE key = :key"));
        query.bindValue(u":key"_s, key);

        if (query.exec() && query.next()) {
            const QFileInfo cached(m_cacheDir.filePath(query.value(0).toString()));
            if (cached.exists()) {
                WR_DEBUG(u"Image cache hit [%1] -> %2"_s
                             .arg(key, cached.absoluteFilePath()));
                return cached;
            }

            // File was deleted externally — evict the stale DB record.
            WR_WARN(u"Image cache stale, file missing [%1], evicting"_s.arg(key));
            QSqlQuery evict(db);
            evict.prepare(QStringLiteral("DELETE FROM image_cache WHERE key = :key"));
            evict.bindValue(u":key"_s, key);
            evict.exec();
        }
    }

    WR_DEBUG(u"Image cache miss [%1], computing"_s.arg(key));
    const QImage image = computeFunc();
    if (image.isNull()) {
        WR_WARN(u"ComputeFunc returned null image for key [%1]"_s.arg(key));
        return QFileInfo{};
    }

    const QString fileName = key + u".png"_s;
    const QString filePath = m_cacheDir.filePath(fileName);

    if (!image.save(filePath, "PNG")) {
        WR_WARN(u"Failed to save image to %1"_s.arg(filePath));
        return QFileInfo{};
    }
    WR_DEBUG(u"Image saved to %1"_s.arg(filePath));

    if (db.isOpen()) {
        QSqlQuery insertQuery(db);
        insertQuery.prepare(QStringLiteral(
            "INSERT OR REPLACE INTO image_cache (key, file_name) "
            "VALUES (:key, :file_name)"));
        insertQuery.bindValue(u":key"_s, key);
        insertQuery.bindValue(u":file_name"_s, fileName);
        if (!insertQuery.exec())
            WR_WARN(u"Failed to record image in db [%1]: %2"_s
                        .arg(key, insertQuery.lastError().text()));
    }

    return QFileInfo(filePath);
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
    q.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS color_cache ("
        "  key  TEXT    PRIMARY KEY NOT NULL,"
        "  r    INTEGER NOT NULL,"
        "  g    INTEGER NOT NULL,"
        "  b    INTEGER NOT NULL,"
        "  a    INTEGER NOT NULL"
        ")"));
    q.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS image_cache ("
        "  key       TEXT PRIMARY KEY NOT NULL,"
        "  file_name TEXT NOT NULL"
        ")"));
}

}  // namespace WallReel::Core::Cache
