#include <QDate>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include "configmgr.hpp"
#include "imagemodel.hpp"
#include "imageprovider.hpp"

class TestImageModel : public QObject {
    Q_OBJECT

  private slots:
    void initTestCase();
    void testSortName();
    void testSortDate();
    void testSortSize();

  private:
    QTemporaryDir m_tempDir;
    QString m_pathA;
    QString m_pathB;
    QString m_pathC;

    void createTestFiles();
    void waitForModel(ImageModel* model);
};

// clang-format off
// xxd <file> -i
static const unsigned char smallGIF[] = {
    0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x01, 0x00, 0x01, 0x00, 0xf0, 0x00,
    0x00, 0xcd, 0xcf, 0xd2, 0x00, 0x00, 0x00, 0x21, 0xf9, 0x04, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
    0x00, 0x02, 0x02, 0x44, 0x01, 0x00, 0x3b
};
static const unsigned char mediumGIF[] = {
  0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x02, 0x00, 0x02, 0x00, 0xf1, 0x00,
  0x00, 0xb0, 0xb8, 0xc0, 0xb7, 0xbc, 0xc2, 0xd8, 0xdb, 0xda, 0xe2, 0xdd,
  0xdb, 0x21, 0xf9, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00,
  0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x02, 0x03, 0xd4, 0x10, 0x05,
  0x00, 0x3b
};
static const unsigned char largeGIF[] = {
  0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x03, 0x00, 0x03, 0x00, 0xf3, 0x00,
  0x00, 0x80, 0x8b, 0x9c, 0xa9, 0xad, 0xac, 0xcf, 0xd5, 0xd6, 0xc9, 0xd2,
  0xdc, 0xde, 0xd7, 0xd8, 0xdf, 0xdf, 0xdf, 0xd3, 0xda, 0xe0, 0xe9, 0xea,
  0xeb, 0xf8, 0xf0, 0xee, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x21, 0xf9, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00,
  0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x04, 0x07, 0xf0, 0x14, 0x24,
  0x02, 0x19, 0xc0, 0x44, 0x00, 0x3b
};
// clang-format on

void TestImageModel::initTestCase() {
    createTestFiles();
}

void TestImageModel::createTestFiles() {
    QVERIFY(m_tempDir.isValid());

    // Create files with specific names, sizes, dates
    // a.gif: medium size, medium date
    // c.gif: small size, old date
    // b.gif: big size, new date
    // Note: Names are a.gif, b.gif, c.gif for name sort.

    m_pathA = m_tempDir.path() + "/a.gif";
    m_pathB = m_tempDir.path() + "/b.gif";
    m_pathC = m_tempDir.path() + "/c.gif";

    {
        QFile f(m_pathA);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(reinterpret_cast<const char*>(mediumGIF), sizeof(mediumGIF));
        f.close();
    }

    {
        QFile f(m_pathB);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(reinterpret_cast<const char*>(largeGIF), sizeof(largeGIF));
        f.close();
    }

    {
        QFile f(m_pathC);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(reinterpret_cast<const char*>(smallGIF), sizeof(smallGIF));
        f.close();
    }

    // Set times

    QDateTime now     = QDateTime::currentDateTime();
    QDateTime timeOld = now.addDays(-10);
    QDateTime timeMid = now.addDays(-5);
    QDateTime timeNew = now;

    {
        QFile f(m_pathC);
        QVERIFY(f.open(QIODevice::ReadWrite));
        QVERIFY(f.setFileTime(timeOld, QFileDevice::FileModificationTime));
    }
    {
        QFile f(m_pathA);
        QVERIFY(f.open(QIODevice::ReadWrite));
        QVERIFY(f.setFileTime(timeMid, QFileDevice::FileModificationTime));
    }
    {
        QFile f(m_pathB);
        QVERIFY(f.open(QIODevice::ReadWrite));
        QVERIFY(f.setFileTime(timeNew, QFileDevice::FileModificationTime));
    }
}

void TestImageModel::waitForModel(ImageModel* model) {
    if (!model->isLoading()) {
        return;
    }
    QSignalSpy spy(model, &ImageModel::isLoadingChanged);
    while (model->isLoading()) {
        if (!spy.wait(5000)) {
            qWarning() << "Timeout waiting for model to load";
            break;
        }
    }
}

void TestImageModel::testSortName() {
    Config::SortConfigItems sortConfig;
    sortConfig.type    = Config::SortType::Name;
    sortConfig.reverse = false;

    ImageProvider provider;
    ImageModel model(provider, sortConfig, QSize(100, 100));

    QStringList paths = {m_pathB, m_pathA, m_pathC};  // Unordered input
    model.loadAndProcess(paths);
    waitForModel(&model);

    QCOMPARE(model.rowCount(), 3);

    // Expected: a.gif, b.gif, c.gif
    QCOMPARE(model.data(model.index(0), ImageModel::NameRole).toString(), "a.gif");
    QCOMPARE(model.data(model.index(1), ImageModel::NameRole).toString(), "b.gif");
    QCOMPARE(model.data(model.index(2), ImageModel::NameRole).toString(), "c.gif");

    // Reverse
    sortConfig.reverse = true;
    model.sortUpdate();

    QCOMPARE(model.rowCount(), 3);

    // Expected: c.gif, b.gif, a.gif
    QCOMPARE(model.data(model.index(0), ImageModel::NameRole).toString(), "c.gif");
    QCOMPARE(model.data(model.index(1), ImageModel::NameRole).toString(), "b.gif");
    QCOMPARE(model.data(model.index(2), ImageModel::NameRole).toString(), "a.gif");
}

void TestImageModel::testSortDate() {
    Config::SortConfigItems sortConfig;
    sortConfig.type    = Config::SortType::Date;
    sortConfig.reverse = false;

    ImageProvider provider;
    ImageModel model(provider, sortConfig, QSize(100, 100));

    QStringList paths = {m_pathA, m_pathC, m_pathB};
    model.loadAndProcess(paths);
    waitForModel(&model);

    QCOMPARE(model.rowCount(), 3);

    // Expected: c (old), a (mid), b (new)
    QCOMPARE(model.data(model.index(0), ImageModel::NameRole).toString(), "c.gif");
    QCOMPARE(model.data(model.index(1), ImageModel::NameRole).toString(), "a.gif");
    QCOMPARE(model.data(model.index(2), ImageModel::NameRole).toString(), "b.gif");

    // Reverse (Newest first)
    sortConfig.reverse = true;
    model.sortUpdate();

    QCOMPARE(model.data(model.index(0), ImageModel::NameRole).toString(), "b.gif");
    QCOMPARE(model.data(model.index(1), ImageModel::NameRole).toString(), "a.gif");
    QCOMPARE(model.data(model.index(2), ImageModel::NameRole).toString(), "c.gif");
}

void TestImageModel::testSortSize() {
    Config::SortConfigItems sortConfig;
    sortConfig.type    = Config::SortType::Size;
    sortConfig.reverse = false;

    ImageProvider provider;
    ImageModel model(provider, sortConfig, QSize(100, 100));

    QStringList paths = {m_pathB, m_pathC, m_pathA};
    model.loadAndProcess(paths);
    waitForModel(&model);

    QCOMPARE(model.rowCount(), 3);

    QCOMPARE(model.data(model.index(0), ImageModel::NameRole).toString(), "c.gif");
    QCOMPARE(model.data(model.index(1), ImageModel::NameRole).toString(), "a.gif");
    QCOMPARE(model.data(model.index(2), ImageModel::NameRole).toString(), "b.gif");

    // Reverse
    sortConfig.reverse = true;
    model.sortUpdate();

    QCOMPARE(model.data(model.index(0), ImageModel::NameRole).toString(), "b.gif");
    QCOMPARE(model.data(model.index(1), ImageModel::NameRole).toString(), "a.gif");
    QCOMPARE(model.data(model.index(2), ImageModel::NameRole).toString(), "c.gif");
}

QTEST_MAIN(TestImageModel)
#include "tst_imagemodel.moc"
