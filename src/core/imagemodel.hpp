#ifndef WALLREEL_IMAGEMODEL_HPP
#define WALLREEL_IMAGEMODEL_HPP

#include <QAbstractListModel>
#include <QFutureWatcher>
#include <QTimer>
#include <atomic>

#include "configmgr.hpp"
#include "imageprovider.hpp"

class ImageModel : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(int processedCount READ processedCount NOTIFY progressChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY totalCountChanged)

  public:
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

    ImageModel(
        ImageProvider* provider,
        const Config::SortConfigItems& sortConfig,
        QSize thumbnailSize,
        QObject* parent = nullptr);

    ~ImageModel();

    bool isLoading() const { return m_isLoading; }

    int processedCount() const { return m_processedCount.load(std::memory_order_relaxed); }

    int totalCount() const { return m_watcher.progressMaximum(); }

    void sortUpdate();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void loadAndProcess(const QStringList& paths);

    Q_INVOKABLE void stop();

    Q_INVOKABLE QVariant dataAt(int index, const QString& roleName) const;

  signals:
    void isLoadingChanged();
    void progressChanged();
    void totalCountChanged();
    void imageSelected(const QString& path);

  private slots:
    void onProgressValueChanged(int value);
    void onProcessingFinished();

  private:
    ImageProvider* m_provider;
    const Config::SortConfigItems& m_sortConfig;
    QSize m_thumbnailSize;

    QList<ImageData*> m_data;

    QFutureWatcher<ImageData*> m_watcher;
    bool m_isLoading = false;

    std::atomic<int> m_processedCount{0};
    QTimer m_progressUpdateTimer;
    static constexpr int s_progressUpdateInterval  = 30;
    static constexpr int s_isLoadingUpdateInterval = 50;
};

#endif  // WALLREEL_IMAGEMODEL_HPP
