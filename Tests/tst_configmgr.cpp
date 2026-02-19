#include <QtGui/qcolor.h>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include "Config/manager.hpp"

using namespace WallReel::Core;

class TestConfigMgr : public QObject {
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void testDefaults();
    void testFullConfigParsing();
    void testInvalidConfigValues();
    void testWallpaperScanRecursive();
    void testWallpaperScanNonRecursive();
    void testWallpaperExcludes();
    void testExplicitPaths();
    void testImageExtensions();
    void testSortTypes();

  private:
    QTemporaryDir m_tempDir;
    QString m_configPath;
    QString m_wallpaperRoot;

    void writeConfig(const QJsonObject& json);
    void createDummyFile(const QString& relPath);
};

void TestConfigMgr::initTestCase() {
    QVERIFY(m_tempDir.isValid());
    m_configPath    = m_tempDir.path() + "/config.json";
    m_wallpaperRoot = m_tempDir.path() + "/wallpapers";
    QDir().mkpath(m_wallpaperRoot);
}

void TestConfigMgr::cleanupTestCase() {
}

void TestConfigMgr::writeConfig(const QJsonObject& json) {
    QFile configFile(m_configPath);
    QVERIFY(configFile.open(QIODevice::WriteOnly));
    configFile.write(QJsonDocument(json).toJson());
    configFile.close();
}

void TestConfigMgr::createDummyFile(const QString& relPath) {
    QString absPath = m_wallpaperRoot + "/" + relPath;
    QFileInfo fi(absPath);
    QDir().mkpath(fi.absolutePath());
    QFile file(absPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("foobar");
    file.close();
}

void TestConfigMgr::testDefaults() {
    // Empty config file
    writeConfig(QJsonObject());

    Config::Manager config(m_tempDir.path(), {}, m_configPath);

    // Check Style Defaults
    QCOMPARE(config.getImageWidth(), 320);
    QCOMPARE(config.getImageHeight(), 200);
    QCOMPARE(config.getImageFocusScale(), 1.5);
    QCOMPARE(config.getWindowWidth(), 750);
    QCOMPARE(config.getWindowHeight(), 500);

    // Check Sort Defaults
    QCOMPARE(config.getSortConfig().type, Config::SortType::Name);
    QCOMPARE(config.getSortConfig().reverse, false);

    // Check Action Defaults
    QCOMPARE(config.getActionConfig().previewDebounceTime, 300);
    QCOMPARE(config.getActionConfig().printSelected, false);
    QCOMPARE(config.getActionConfig().printPreview, false);
    QVERIFY(config.getActionConfig().onSelected.isEmpty());
    QVERIFY(config.getActionConfig().onPreview.isEmpty());
    QVERIFY(config.getActionConfig().onRestore.isEmpty());
    QVERIFY(config.getActionConfig().saveStateConfig.isEmpty());
}

void TestConfigMgr::testFullConfigParsing() {
    QJsonObject root;

    // Wallpaper settings
    QJsonObject wallpaperObj;
    QJsonArray dirsArray;
    QJsonObject dir1;
    dir1["path"]      = "/tmp/w1";
    dir1["recursive"] = true;
    dirsArray.append(dir1);
    wallpaperObj["dirs"] = dirsArray;

    QJsonArray pathsArray;
    pathsArray.append("/tmp/p1.jpg");
    wallpaperObj["paths"] = pathsArray;

    QJsonArray excludesArray;
    excludesArray.append(".*bad.*");
    wallpaperObj["excludes"] = excludesArray;

    root["wallpaper"] = wallpaperObj;

    // Palette
    QJsonArray palettesArray;
    QJsonObject palette1;
    palette1["name"] = "Default";
    QJsonArray colorsArray;
    QJsonObject color1;
    color1["name"]  = "Red";
    color1["value"] = "#FF0000";
    colorsArray.append(color1);
    palette1["colors"] = colorsArray;
    palettesArray.append(palette1);
    root["palettes"] = palettesArray;

    // Action
    QJsonObject actionObj;
    actionObj["printSelected"] = true;
    actionObj["onSelected"]    = "echo {{ path }}";
    root["action"]             = actionObj;

    // Style
    QJsonObject styleObj;
    styleObj["image_width"]  = 100;
    styleObj["image_height"] = 100;
    root["style"]            = styleObj;

    // Sort
    QJsonObject sortObj;
    sortObj["type"]    = "date";
    sortObj["reverse"] = true;
    root["sort"]       = sortObj;

    writeConfig(root);
    Config::Manager config(m_tempDir.path(), {}, m_configPath);

    // Assertions
    QCOMPARE(config.getWallpaperConfig().dirs.size(), 1);
    QCOMPARE(config.getWallpaperConfig().dirs[0].path, "/tmp/w1");
    QCOMPARE(config.getWallpaperConfig().dirs[0].recursive, true);

    QCOMPARE(config.getWallpaperConfig().paths.size(), 1);
    QCOMPARE(config.getWallpaperConfig().paths[0], "/tmp/p1.jpg");

    QCOMPARE(config.getWallpaperConfig().excludes.size(), 1);
    QCOMPARE(config.getWallpaperConfig().excludes[0].pattern(), ".*bad.*");

    QCOMPARE(config.getPaletteConfig().palettes.size(), 1);
    QCOMPARE(config.getPaletteConfig().palettes[0].name, "Default");
    QCOMPARE(config.getPaletteConfig().palettes[0].colors.size(), 1);
    QCOMPARE(config.getPaletteConfig().palettes[0].colors[0].name, "Red");
    QCOMPARE(config.getPaletteConfig().palettes[0].colors[0].value.name().toLower(), "#ff0000");

    QCOMPARE(config.getActionConfig().printSelected, true);
    QCOMPARE(config.getActionConfig().onSelected, "echo {{ path }}");

    QCOMPARE(config.getImageWidth(), 100);
    QCOMPARE(config.getImageHeight(), 100);

    QCOMPARE(config.getSortConfig().type, Config::SortType::Date);
    QCOMPARE(config.getSortConfig().reverse, true);
}

void TestConfigMgr::testInvalidConfigValues() {
    QJsonObject root;
    QJsonObject styleObj;
    styleObj["image_width"] = "not a number";  // Should be ignored
    root["style"]           = styleObj;

    QJsonObject sortObj;
    sortObj["type"] = "invalid_type";
    root["sort"]    = sortObj;

    writeConfig(root);
    Config::Manager config(m_tempDir.path(), {}, m_configPath);

    // Should retain defaults
    QCOMPARE(config.getImageWidth(), 320);
    QCOMPARE(config.getSortConfig().type, Config::SortType::Name);
}

void TestConfigMgr::testWallpaperScanRecursive() {
    // Setup files
    createDummyFile("rec/root.jpg");
    createDummyFile("rec/sub/deep.png");  // should be found
    createDummyFile("rec/ignore.txt");    // should be ignored

    QJsonObject root;
    QJsonObject wallpaperObj;
    QJsonArray dirsArray;
    QJsonObject dirConfig;
    dirConfig["path"]      = m_wallpaperRoot + "/rec";
    dirConfig["recursive"] = true;
    dirsArray.append(dirConfig);
    wallpaperObj["dirs"] = dirsArray;
    root["wallpaper"]    = wallpaperObj;

    writeConfig(root);
    Config::Manager config(m_tempDir.path(), {}, m_configPath);

    QStringList wallpapers = config.getWallpapers();
    QCOMPARE(wallpapers.size(), 2);

    // Sort to verify presence
    wallpapers.sort();
    // Paths are absolute
    QVERIFY(wallpapers[0].endsWith("root.jpg"));
    QVERIFY(wallpapers[1].endsWith("deep.png"));
}

void TestConfigMgr::testWallpaperScanNonRecursive() {
    createDummyFile("nonrec/root.jpg");
    createDummyFile("nonrec/sub/deep.png");

    QJsonObject root;
    QJsonObject wallpaperObj;
    QJsonArray dirsArray;
    QJsonObject dirConfig;
    dirConfig["path"]      = m_wallpaperRoot + "/nonrec";
    dirConfig["recursive"] = false;
    dirsArray.append(dirConfig);
    wallpaperObj["dirs"] = dirsArray;
    root["wallpaper"]    = wallpaperObj;

    writeConfig(root);
    Config::Manager config(m_tempDir.path(), {}, m_configPath);

    QStringList wallpapers = config.getWallpapers();
    QCOMPARE(wallpapers.size(), 1);
    QVERIFY(wallpapers[0].endsWith("root.jpg"));
}

void TestConfigMgr::testWallpaperExcludes() {
    createDummyFile("excl/good.jpg");
    createDummyFile("excl/bad.jpg");

    QJsonObject root;
    QJsonObject wallpaperObj;
    QJsonArray dirsArray;
    QJsonObject dirConfig;
    dirConfig["path"]      = m_wallpaperRoot + "/excl";
    dirConfig["recursive"] = false;
    dirsArray.append(dirConfig);
    wallpaperObj["dirs"] = dirsArray;

    QJsonArray excludes;
    excludes.append(".*bad\\.jpg$");
    wallpaperObj["excludes"] = excludes;

    root["wallpaper"] = wallpaperObj;

    writeConfig(root);
    Config::Manager config(m_tempDir.path(), {}, m_configPath);

    QStringList wallpapers = config.getWallpapers();
    QCOMPARE(wallpapers.size(), 1);
    QVERIFY(wallpapers[0].endsWith("good.jpg"));
}

void TestConfigMgr::testExplicitPaths() {
    createDummyFile("explicit/a.jpg");
    QString absPath = m_wallpaperRoot + "/explicit/a.jpg";

    QJsonObject root;
    QJsonObject wallpaperObj;
    QJsonArray pathsArray;
    pathsArray.append(absPath);
    wallpaperObj["paths"] = pathsArray;
    root["wallpaper"]     = wallpaperObj;

    writeConfig(root);
    Config::Manager config(m_tempDir.path(), {}, m_configPath);

    QStringList wallpapers = config.getWallpapers();
    QCOMPARE(wallpapers.size(), 1);
    QCOMPARE(wallpapers[0], absPath);
}

void TestConfigMgr::testImageExtensions() {
    createDummyFile("ext/image1.jpg");
    createDummyFile("ext/image2.jpeg");
    createDummyFile("ext/image3.png");
    createDummyFile("ext/image4.bmp");
    createDummyFile("ext/text.txt");
    createDummyFile("ext/script.sh");
    createDummyFile("ext/noext");

    QJsonObject root;
    QJsonObject wallpaperObj;
    QJsonArray dirsArray;
    QJsonObject dirConfig;
    dirConfig["path"]      = m_wallpaperRoot + "/ext";
    dirConfig["recursive"] = false;
    dirsArray.append(dirConfig);
    wallpaperObj["dirs"] = dirsArray;
    root["wallpaper"]    = wallpaperObj;

    writeConfig(root);
    Config::Manager config(m_tempDir.path(), {}, m_configPath);

    QStringList wallpapers = config.getWallpapers();

    int imageCount = 0;
    for (const auto& w : wallpapers) {
        if (w.endsWith(".txt") || w.endsWith(".sh") || w.endsWith("noext")) {
            QFAIL(qPrintable("Found non-image file: " + w));
        }
        imageCount++;
    }
    QVERIFY(imageCount >= 3);
}

void TestConfigMgr::testSortTypes() {
    // 1. None sort
    {
        QJsonObject root;
        QJsonObject sortObj;
        sortObj["type"]    = "none";
        sortObj["reverse"] = false;
        root["sort"]       = sortObj;

        writeConfig(root);
        Config::Manager config(m_tempDir.path(), {}, m_configPath);
        QCOMPARE(config.getSortConfig().type, Config::SortType::None);
        QCOMPARE(config.getSortConfig().reverse, false);
    }
    // 2. Name sort (default)
    {
        QJsonObject root;
        QJsonObject sortObj;
        sortObj["type"]    = "name";
        sortObj["reverse"] = true;
        root["sort"]       = sortObj;
        writeConfig(root);
        Config::Manager config(m_tempDir.path(), {}, m_configPath);
        QCOMPARE(config.getSortConfig().type, Config::SortType::Name);
        QCOMPARE(config.getSortConfig().reverse, true);
    }
    // 3. Size sort
    {
        QJsonObject root;
        QJsonObject sortObj;
        sortObj["type"] = "size";
        root["sort"]    = sortObj;
        writeConfig(root);
        Config::Manager config(m_tempDir.path(), {}, m_configPath);
        QCOMPARE(config.getSortConfig().type, Config::SortType::Size);
    }
    // 4. Date sort
    {
        QJsonObject root;
        QJsonObject sortObj;
        sortObj["type"] = "date";
        root["sort"]    = sortObj;
        writeConfig(root);
        Config::Manager config(m_tempDir.path(), {}, m_configPath);
        QCOMPARE(config.getSortConfig().type, Config::SortType::Date);
    }
    // 5. Invalid sort -> fallback to default (Name)
    {
        QJsonObject root;
        QJsonObject sortObj;
        sortObj["type"] = "invalid_blah";
        root["sort"]    = sortObj;
        writeConfig(root);
        Config::Manager config(m_tempDir.path(), {}, m_configPath);
        // Default initialized in Config constructor is Name
        // But warning is logged
        QCOMPARE(config.getSortConfig().type, Config::SortType::Name);
    }
    // 6. Case insensitivity for type string
    {
        QJsonObject root;
        QJsonObject sortObj;
        sortObj["type"] = "DaTe";
        root["sort"]    = sortObj;
        writeConfig(root);
        Config::Manager config(m_tempDir.path(), {}, m_configPath);
        QCOMPARE(config.getSortConfig().type, Config::SortType::Date);
    }
}

QTEST_MAIN(TestConfigMgr)
#include "tst_configmgr.moc"
