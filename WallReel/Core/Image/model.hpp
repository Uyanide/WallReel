#ifndef WALLREEL_IMAGEMODEL_HPP
#define WALLREEL_IMAGEMODEL_HPP

#include <QAbstractListModel>
#include <QDir>
#include <QFutureWatcher>
#include <QTimer>
#include <atomic>

#include "Cache/manager.hpp"
#include "Config/data.hpp"
#include "data.hpp"

// Development note
/*
What "Proxy index" is:

There are currently three layers of indices in the Model:
1. Actual index: The index of the image in the original data list (m_data), the order is not
                 guaranteed and can be considered random.
2. Sorted index: The index of the image after sorting, which is stored in m_sortIndices based on
                 different sort types. m_sortIndices are precomputed and does not change unless
                 m_data changes. In practice, the choise of which mapping from m_sortIndices to use
                 is determined by m_currentSortType.
3. Filtered index: The final mapping from the index exposed to the QML view to the actual data index,
                   which is stored in m_filteredIndices. m_filteredIndices is updated each time when
                   the sort type / sort order / search text changes, and only informs the layout to
                   update when its content actually changes.

Therefore, when acquiring data, the "proxied" index must first be converted to the "actual" index
by looking up m_filteredIndices, and then the actual data can be accessed from m_data.

*/

namespace WallReel::Core::Image {

/**
 * @brief An unrefactored (view)model class that manages and provides the image list and properties of the focused image.
 *
 */
class Model : public QAbstractListModel {
    Q_OBJECT

    // Controls which of the main screen and the loading screen should be shown
    // and triggers callbacks when loading finished
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    // Indicates the progress of loading, used to update the progress bar in the loading screen
    // Not neccessarily updated on every image loaded, but should be updated frequently enough to make the progress bar smooth
    Q_PROPERTY(int processedCount READ processedCount NOTIFY progressChanged)
    // Total count of images to be loaded, used to calculate the progress percentage
    Q_PROPERTY(int totalCount READ totalCount NOTIFY totalCountChanged)
    // Sorting related properties
    // How this works:
    // 1. User interact with QML control components
    // 2. QML calls the setter of the corresponding property in the Model
    // 3. Model changes its internal state and update the sort indices accordingly
    // 4. Model emits signal and possibly update state on QML side (for stateless controls)
    // 5. ... Continue on further updates (search filter / focused image properties / etc)
    Q_PROPERTY(QString currentSortType READ currentSortType WRITE setCurrentSortType NOTIFY currentSortTypeChanged)
    Q_PROPERTY(bool currentSortReverse READ currentSortReverse WRITE setCurrentSortReverse NOTIFY currentSortReverseChanged)
    // Focused image related properties, updated when focused image changed
    Q_PROPERTY(QString focusedName READ focusedName NOTIFY focusedNameChanged)

  public:
    // Types

    enum Roles {
        IdRole = Qt::UserRole + 1,
        UrlRole,
        PathRole,
        NameRole
    };

    QHash<int, QByteArray> roleNames() const override {
        return {
            {IdRole, "imgId"},
            {UrlRole, "imgUrl"},  // file:///...
            {PathRole, "imgPath"},
            {NameRole, "imgName"},
        };
    }

    // Constructor / Destructor

    Model(
        const Config::SortConfigItems& sortConfig,
        Cache::Manager& cacheMgr,
        const QSize& thumbnailSize,
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

    Data* imageAt(int index);

    Data* focusedImage();

    Q_INVOKABLE QVariant dataAt(int index, const QString& roleName) const;

    void loadAndProcess(const QStringList& paths);

    Q_INVOKABLE void focusOnIndex(int index);

    Q_INVOKABLE void stop();

  private:
    int _convertProxyIndex(int proxyIndex) const;
    void _clearData();
    // Update the corresponding mapping in m_sortIndices based on the current m_data and the given sort type
    void _updateSortIndices(Config::SortType type);
    // Reobtain the properties of the focused image and emit corresponding signals
    void _updateFocusedProperties();
    // Update m_filteredIndices, only calls layoutAboutToBeChanged and layoutChanged when the filtered result
    // actually changes and informView is true
    void _applySearchFilter(bool informView = true);

  signals:
    // Properties
    void isLoadingChanged();
    void progressChanged();
    void totalCountChanged();
    void currentSortTypeChanged();     // -> _onSortMethodChanged
    void currentSortReverseChanged();  // -> _onSortMethodChanged
    void focusedNameChanged();
    // emitted after search text changed and the filter is applied
    void searchTextChanged();  // -> _onSearchTextChanged
    // emiited when the focued image (is believed to be) changed
    void focusedImageChanged();

  private slots:
    void _onProgressValueChanged(int value);
    void _onProcessingFinished();
    void _onSortMethodChanged();
    void _onSearchTextChanged();

  private:
    const Config::SortConfigItems& m_sortConfig;
    Cache::Manager& m_cacheMgr;
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
