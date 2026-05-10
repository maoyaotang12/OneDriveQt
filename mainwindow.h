#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QSettings>
#include <QTextEdit>
#include <QLabel>
#include <QCloseEvent>
#include <QApplication>
#include <QTimer>

enum TrayStatus {
    TrayIdle,      // 默认云朵
    TraySyncing,   // 同步中
    TraySyncDone,  // 同步完成 ✅ 新加这个
    TrayStopped,   // 停止
    TrayError      // 错误
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void onTrayClicked(QSystemTrayIcon::ActivationReason);
    void showWindow();
    void exitApp();

    void startSync();
    void stopSync();
    void syncOnce();
    void openDir();
    void openSettings();

    void readOutput();
    void onSyncDone(int code);
    void loadOfficialOneDriveConfig();
    void selectAndImportOneDriveAccount();
    void setAutoStart(bool enable);
    void showAbout();

private:
    void updateTrayByStatus();
    void updateTrayIcon();
    void loadSettings();
    void saveSettings();
    QSystemTrayIcon *tray;
    QMenu *trayMenu;
    QProcess *proc;
    QSettings *cfg;
    QTimer *syncTimer;

    QTextEdit *logView;
    QLabel *statusLabel;

    bool minToTray;
    TrayStatus trayStatus;
};

#endif
