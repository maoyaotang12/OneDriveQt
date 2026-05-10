#include "mainwindow.h"
#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
{
    setWindowTitle("OneDrive");
    setMinimumSize(700, 500);

    cfg = new QSettings(QDir::homePath() + "/.config/onedrive-qt.ini", QSettings::IniFormat, this);
    proc = new QProcess(this);

    tray = new QSystemTrayIcon(this);
    tray->setToolTip("OneDrive");

    trayMenu = new QMenu(this);
    trayMenu->addAction("显示主窗口", this, &MainWindow::showWindow);
    trayMenu->addSeparator();
    trayMenu->addAction("开始同步", this, &MainWindow::startSync);
    trayMenu->addAction("停止同步", this, &MainWindow::stopSync);
    trayMenu->addAction("手动同步", this, &MainWindow::syncOnce);
    trayMenu->addSeparator();
    trayMenu->addAction("打开文件夹", this, &MainWindow::openDir);
    trayMenu->addAction("设置", this, &MainWindow::openSettings);
    trayMenu->addAction("开机自启", this, [this]() {
        setAutoStart(true);
    });
    trayMenu->addAction("关闭自启", this, [this]() {
        setAutoStart(false);
    });
    trayMenu->addSeparator();
    trayMenu->addAction("关于", this, &MainWindow::showAbout);
    trayMenu->addSeparator();
    trayMenu->addAction("退出", this, &MainWindow::exitApp);

    tray->setContextMenu(trayMenu);
    tray->show();

    QWidget *c = new QWidget(this);
    setCentralWidget(c);
    QVBoxLayout *lay = new QVBoxLayout(c);

    statusLabel = new QLabel("状态：未同步");
    lay->addWidget(statusLabel);

    QHBoxLayout *btnLay = new QHBoxLayout;
    QPushButton *b1 = new QPushButton("开始同步");
    QPushButton *b2 = new QPushButton("停止同步");
    QPushButton *b3 = new QPushButton("手动同步");
    QPushButton *bImport = new QPushButton("导入账号配置");
    QPushButton *b4 = new QPushButton("设置");
    btnLay->addWidget(b1);
    btnLay->addWidget(b2);
    btnLay->addWidget(b3);
    btnLay->addWidget(bImport);
    btnLay->addWidget(b4);
    lay->addLayout(btnLay);

    logView = new QTextEdit;
    logView->setReadOnly(true);
    lay->addWidget(logView);

    connect(b1, &QPushButton::clicked, this, &MainWindow::startSync);
    connect(b2, &QPushButton::clicked, this, &MainWindow::stopSync);
    connect(b3, &QPushButton::clicked, this, &MainWindow::syncOnce);
    connect(bImport, &QPushButton::clicked, this, &MainWindow::selectAndImportOneDriveAccount);
    connect(b4, &QPushButton::clicked, this, &MainWindow::openSettings);

    connect(proc, &QProcess::readyReadStandardOutput, this, &MainWindow::readOutput);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::onSyncDone);

    minToTray = cfg->value("MinToTray", true).toBool();
    loadOfficialOneDriveConfig();

    trayStatus = TrayIdle;
    updateTrayIcon();
    tray->show();

    connect(tray, &QSystemTrayIcon::activated, this, &MainWindow::onTrayClicked);
    startSync();
}

void MainWindow::loadOfficialOneDriveConfig()
{
    QString base = QDir::homePath() + "/.config/onedrive/accounts";
    QDir baseDir(base);
    QStringList users = baseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    if (users.isEmpty()) {
        logView->append("❌ 未找到任何 OneDrive 用户配置");
        return;
    }

    QString user = users.first();
    QString path = base + "/" + user + "/config";

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        logView->append("❌ 无法读取用户配置：" + path);
        return;
    }

    QString content = f.readAll();
    f.close();

    logView->append("✅ 已找到用户：" + user);
    logView->append("✅ 已加载用户配置");

    QRegularExpression re("sync_dir = \"([^\"]+)\"");
    auto match = re.match(content);
    if (match.hasMatch()) {
        QString dir = match.captured(1);
        cfg->setValue("SyncDir", dir);
        logView->append("📂 同步目录：" + dir);
    }
}

MainWindow::~MainWindow()
{
    if (proc->isOpen()) proc->terminate();
}

void MainWindow::showWindow() { show(); raise(); }
void MainWindow::exitApp() { if (proc) proc->terminate(); qApp->quit(); }

void MainWindow::startSync()
{
    QString dir = cfg->value("SyncDir").toString();
    proc->setWorkingDirectory(dir);
    proc->start("onedrive", {"--monitor"});

    trayStatus = TraySyncing;
    updateTrayIcon();
    statusLabel->setText("状态：同步中…");
}

void MainWindow::stopSync()
{
    proc->terminate();
    statusLabel->setText("状态：已停止");
    tray->setToolTip("OneDrive 已停止");
    trayStatus = TrayStopped;
    updateTrayIcon();
}

void MainWindow::syncOnce()
{
    QString dir = cfg->value("SyncDir").toString();
    proc->setWorkingDirectory(dir);
    proc->start("onedrive", {"--sync"});
    statusLabel->setText("状态：单次同步");
    trayStatus = TraySyncing;
    updateTrayIcon();
}

void MainWindow::openDir()
{
    QString d = cfg->value("SyncDir").toString();
    QDesktopServices::openUrl(QUrl::fromLocalFile(d));
}

void MainWindow::openSettings()
{
    SettingsDialog s;

    s.setSyncDir(cfg->value("SyncDir", QDir::homePath() + "/OneDrive").toString());
    s.setMinTray(cfg->value("MinToTray", true).toBool());

    if (s.exec() == QDialog::Accepted) {
        cfg->setValue("SyncDir", s.syncDir());
        cfg->setValue("MinToTray", s.minTray());
        minToTray = s.minTray();
        cfg->sync();
    }
}

void MainWindow::readOutput()
{
    QByteArray data = proc->readAllStandardOutput();
    QString txt = QString::fromUtf8(data).trimmed();
    if (txt.isEmpty()) return;

    txt.replace("Using IPv4 and IPv6 (if configured) for all network operations", "使用 IPv4 和 IPv6（如已配置）进行所有网络操作");
    txt.replace("Attempting to contact the Microsoft OneDrive Service", "正在连接 Microsoft OneDrive 服务");
    txt.replace("Successfully reached the Microsoft OneDrive Service", "✅ 成功连接 Microsoft OneDrive 服务");
    txt.replace("Configuring Global Azure AD Endpoints", "正在配置全球 Azure AD 服务端点");
    txt.replace("Attempting to enable WebSocket support to monitor Microsoft Graph API changes in near real-time.", "正在启用 WebSocket 实时监听云端文件变更...");
    txt.replace("Enabled WebSocket support to monitor Microsoft Graph API changes in near real-time.", "✅ 已启用 WebSocket 实时监听");
    txt.replace("OneDrive synchronisation interval (seconds):", "同步间隔时间（秒）：");
    txt.replace("Initialising filesystem inotify monitoring ...", "正在初始化文件系统监控...");
    txt.replace("Performing initial synchronisation to ensure consistent local state ...", "正在执行首次同步，确保本地状态一致...");
    txt.replace("Starting a sync with Microsoft OneDrive", "🚀 开始同步");
    txt.replace("Syncing changes from Microsoft OneDrive ...", "正在同步云端变更...");
    txt.replace("Sync with Microsoft OneDrive is complete", "🏁 同步完成");
    txt.replace("New directories to create on Microsoft OneDrive:", "📁 需新建云端目录：");
    txt.replace("Successfully created the remote directory", "✅ 成功创建目录");
    txt.replace("on Microsoft OneDrive", "");

    txt.replace("Fetching items from the OneDrive API for Drive ID:", "正在获取网盘文件列表，网盘ID：");
    txt.replace("applicable JSON items received from Microsoft OneDrive .", "个文件");
    txt.replace("No changes or items that can be applied were discovered while processing the data received from Microsoft OneDrive", "ℹ️ 未发现任何文件变更，无需同步");
    txt.replace("Performing a database consistency and integrity check on locally stored data .", "正在校验本地同步数据库...");
    txt.replace("Scanning the local file system '~/OneDrive' for new data to upload .", "🔍 扫描本地待上传文件...");
    txt.replace("Performing a last examination of the most recent online data within Microsoft OneDrive to complete the reconciliation process", "正在最终核对云端文件状态...");

    txt.replace("The operating system sent a deletion notification. Trying to delete this item as requested:", "检测到删除，正在同步云端删除...");
    txt.replace("Deleting item from Microsoft OneDrive:", "🗑️ 删除云端文件：");

    txt.replace("Received", "📡 收到");
    txt.replace("signal(s) from WebSocket handler", "条实时通知");

    txt.replace("Attempting to perform a database vacuum to optimise database", "正在优化数据库...");
    txt.replace("Database vacuum is complete", "✅ 数据库优化完成");

    txt.replace("DEPRECIATION WARNING: --synchronize has been deprecated in favour of --sync or -s", "");
    txt.replace("DEPRECIATION WARNING: Deprecated commands will be removed in a future release.", "");

    logView->append(txt);

    if (txt.contains("开始同步") || txt.contains("正在同步") || txt.contains("同步中") ||
        txt.contains("正在处理") || txt.contains("扫描") || txt.contains("上传") || txt.contains("下载"))
    {
        trayStatus = TraySyncing;
        statusLabel->setText("状态：同步中…");
    }
    else if (txt.contains("同步完成"))
    {
        trayStatus = TraySyncDone;
        statusLabel->setText("状态：同步完成 ✅");
        QTimer::singleShot(2000, this, [this]() {
            trayStatus = TrayIdle;
            updateTrayIcon();
        });
    }
    updateTrayIcon();
}

void MainWindow::selectAndImportOneDriveAccount()
{
    QString basePath = QDir::homePath() + "/.config/onedrive/accounts";
    QDir dir(basePath);
    QStringList accountDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (accountDirs.isEmpty()) {
        logView->append("❌ 未找到任何 OneDrive 账号配置");
        return;
    }

    bool ok;
    QString selectedAcc = QInputDialog::getItem(this, "选择 OneDrive 账号", "请选择要导入的用户账号：", accountDirs, 0, false, &ok);
    if (!ok || selectedAcc.isEmpty()) return;

    QString configPath = basePath + "/" + selectedAcc + "/config";
    QFile f(configPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        logView->append("❌ 无法读取配置文件：" + configPath);
        return;
    }

    QString content = f.readAll();
    f.close();
    logView->append("========================================");
    logView->append("✅ 已选择账号：" + selectedAcc);

    QRegularExpression reSyncDir("sync_dir\\s*=\\s*\"([^\"]+)\"");
    auto match = reSyncDir.match(content);
    if (match.hasMatch()) {
        QString syncDir = match.captured(1);
        cfg->setValue("SyncDir", syncDir);
        logView->append("📂 同步目录已导入：" + syncDir);
    }
    logView->append("========================================");
}

void MainWindow::onTrayClicked(QSystemTrayIcon::ActivationReason r)
{
    if (r == QSystemTrayIcon::Trigger || r == QSystemTrayIcon::DoubleClick) {
        if (!isVisible()) { showNormal(); raise(); activateWindow(); }
        else { hide(); }
    }
}

void MainWindow::updateTrayIcon()
{
    QIcon icon;
    switch (trayStatus) {
        case TraySyncing:   icon = QIcon(":/icon_syncing.svg"); break;
        case TraySyncDone:  icon = QIcon(":/icon_done.svg");    break;
        case TrayIdle:      icon = QIcon(":/icon_idle.svg");    break;
        case TrayStopped:   icon = QIcon(":/icon_stopped.svg"); break;
        case TrayError:     icon = QIcon(":/icon_error.svg");   break;
        default:            icon = QIcon(":/icon_idle.svg");
    }
    tray->setIcon(icon);
}

void MainWindow::onSyncDone(int code)
{
    if (code == 0) {
        trayStatus = TrayIdle;
        statusLabel->setText("状态：同步完成 ✅");
    } else {
        trayStatus = TrayError;
        statusLabel->setText("状态：同步失败 ❌");
    }
    updateTrayIcon();
}

void MainWindow::setAutoStart(bool enable)
{
    QString autoStartPath = QDir::homePath() + "/.config/autostart/OneDriveQtTray.desktop";
    if (enable) {
        QDir dir;
        dir.mkpath(QDir::homePath() + "/.config/autostart");
        QString content = "[Desktop Entry]\nName=OneDrive Qt Tray\nExec=" + QApplication::applicationFilePath() + "\nIcon=cloud\nType=Application\nX-GNOME-Autostart-enabled=true\nComment=OneDrive 同步客户端\n";
        QFile file(autoStartPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(content.toUtf8());
            file.close();
            logView->append("✅ 已设置开机自启");
        }
    } else {
        QFile::remove(autoStartPath);
        logView->append("❌ 已关闭开机自启");
    }
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "关于", "OneDriveQt 同步工具\n版本：1.0.1\n作者：MaoYaoTang\n功能：OneDrive 实时同步\n支持开机自启、状态图标自动切换");
}

// ======================
// 唯一正确关闭事件
// ======================
void MainWindow::closeEvent(QCloseEvent *e)
{
    if (minToTray) {
        hide();
        e->ignore();
    } else {
        e->accept();
    }
}
