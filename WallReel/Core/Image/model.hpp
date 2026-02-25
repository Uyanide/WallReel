#ifndef WALLREEL_IMAGEMODEL_HPP
#define WALLREEL_IMAGEMODEL_HPP

#include <qcontainerfwd.h>

#include <QAbstractListModel>
#include <QFutureWatcher>
#include <QTimer>
#include <atomic>

#include "Config/data.hpp"
#include "provider.hpp"

namespace WallReel::Core::Image {

class Model : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(int processedCount READ processedCount NOTIFY progressChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY totalCountChanged)
    Q_PROPERTY(QString currentSortType READ currentSortType WRITE setCurrentSortType NOTIFY currentSortTypeChanged)
    Q_PROPERTY(bool currentSortReverse READ currentSortReverse WRITE setCurrentSortReverse NOTIFY currentSortReverseChanged)
    Q_PROPERTY(QString focusedName READ focusedName NOTIFY focusedNameChanged)

  public:
    // Types

    enum Roles {
        IdRole = Qt::UserRole + 1,
        PathRole,
        NameRole
    };

    QHash<int, QByteArray> roleNames() const override {
        return {
            {IdRole, "imgId"},
            {PathRole, "imgPath"},
            {NameRole, "imgName"},
        };
    }

    // Constructor / Destructor

    Model(
        Provider& provider,
        const Config::SortConfigItems& sortConfig,
        QSize thumbnailSize,
        QObject* parent = nullptr);

    ~Model();

    // QAbstractListModel

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // Properties

    bool isLoading() const { return m_isLoading; }

    int processedCount() const { return m_processedCount.load(std::memory_order_relaxed); }

    int totalCount() const { return m_watcher.progressMaximum(); }

    QString currentSortType() const;

    void setCurrentSortType(const QString& type);

    bool currentSortReverse() const { return m_currentSortReverse; }

    void setCurrentSortReverse(bool reverse);

    QString focusedName() const { return m_focusedName; }

    // Methods

    Q_INVOKABLE void setSearchText(const QString& text);

    const Data* getDataPtrAt(int index) const;

    Q_INVOKABLE QVariant dataAt(int index, const QString& roleName) const;

    void loadAndProcess(const QStringList& paths);

    int fromProxyIndex(int proxyIndex) const;

    Q_INVOKABLE void focusOnIndex(int index);

    Q_INVOKABLE void stop();

  private:
    void _clearData();
    void _updateSortIndices(Config::SortType type);
    void _updateFocusedName();
    void _applySearchFilter();

  signals:
    void isLoadingChanged();
    void progressChanged();
    void totalCountChanged();
    void currentSortTypeChanged();
    void currentSortReverseChanged();
    void focusedNameChanged();
    void searchTextChanged();

  private slots:
    void _onProgressValueChanged(int value);
    void _onProcessingFinished();
    void _onSortMethodChanged();
    void _onSearchTextChanged();

  private:
    Provider& m_provider;
    const Config::SortConfigItems& m_sortConfig;
    QSize m_thumbnailSize;

    QList<Data*> m_data;

    QList<QList<int>> m_sortIndices;
    Config::SortType m_currentSortType;
    bool m_currentSortReverse;

    QString m_focusedName{};

    QList<int> m_filteredIndices;
    QString m_searchText{};
    // QTimer m_searchDebounceTimer;
    // static constexpr int s_SearchDebounceIntervalMs = 300;

    QFutureWatcher<Data*> m_watcher;
    bool m_isLoading = false;

    int m_focusedIndex = -1;

    std::atomic<int> m_processedCount{0};
    QTimer m_progressUpdateTimer;
    static constexpr int s_ProgressUpdateIntervalMs = 30;
};

}  // namespace WallReel::Core::Image

#endif  // WALLREEL_IMAGEMODEL_HPP
