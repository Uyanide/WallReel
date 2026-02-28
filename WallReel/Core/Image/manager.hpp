#ifndef WALLREEL_IMAGEMANAGER_HPP
#define WALLREEL_IMAGEMANAGER_HPP

#include <QAbstractListModel>
#include <QDir>
#include <QFutureWatcher>
#include <QTimer>
#include <atomic>

#include "Cache/manager.hpp"
#include "data.hpp"
#include "model.hpp"

namespace WallReel::Core::Image {

class Manager : public QObject {
    Q_OBJECT

  public:
    // Constructor / Destructor

    Manager(
        Cache::Manager& cacheMgr,
        const QSize& thumbnailSize,
        QObject* parent = nullptr);

    ~Manager();

    Image::ProxyModel* model() const { return m_proxyModel; }

    bool isLoading() const { return m_isLoading; }

    int processedCount() const { return m_processedCount.load(std::memory_order_relaxed); }

    int totalCount() const { return m_watcher.progressMaximum(); }

    void setSortType(Config::SortType type) { m_proxyModel->setSortType(type); }

    void setSortDescending(bool descending) { m_proxyModel->setSortDescending(descending); }

    void setSearchText(const QString& text) { m_proxyModel->setSearchText(text); }

    Config::SortType sortType() const { return m_proxyModel->getSortType(); }

    bool sortDescending() const { return m_proxyModel->isSortDescending(); }

    QString searchText() const { return m_proxyModel->getSearchText(); }

    void loadAndProcess(const QStringList& paths);

    void stop();

    Image::Data* imageAt(const QString& id) {
        if (m_dataMap.contains(id)) {
            return m_dataMap[id];
        }
        return nullptr;
    }

  private:
    void _clearData();

  signals:
    // Properties
    void isLoadingChanged();
    void processedCountChanged();
    void totalCountChanged();

  private slots:
    void _onProgressValueChanged(int value);
    void _onProcessingFinished();

  private:
    Model* m_dataModel;
    ProxyModel* m_proxyModel;
    QHash<QString, Data*> m_dataMap;

    Cache::Manager& m_cacheMgr;
    QSize m_thumbnailSize;

    QFutureWatcher<Data*> m_watcher;
    bool m_isLoading = false;

    std::atomic<int> m_processedCount{0};
    QTimer m_progressUpdateTimer;
    static constexpr int s_ProgressUpdateIntervalMs = 30;
};

}  // namespace WallReel::Core::Image

#endif  // WALLREEL_IMAGEMANAGER_HPP
