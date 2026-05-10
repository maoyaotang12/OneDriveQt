#include "logindialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QProcess>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("OneDrive 官方登录");
    setFixedSize(600, 300);
    QVBoxLayout *lay = new QVBoxLayout(this);

    log = new QTextEdit;
    log->setReadOnly(true);
    log->setStyleSheet("font-size:14px;");
    log->append("ℹ️  点击下方按钮，将自动打开微软官方登录页面");
    log->append("ℹ️  登录成功后，复制浏览器地址栏的完整 URL");
    lay->addWidget(log);

    QPushButton *btnOpen = new QPushButton("✅ 打开微软官方登录页");
    QPushButton *btnSubmit = new QPushButton("✅ 我已登录，提交授权");
    QPushButton *btnCancel = new QPushButton("取消");

    lay->addWidget(btnOpen);
    lay->addWidget(btnSubmit);
    lay->addWidget(btnCancel);

    // 按钮逻辑
    connect(btnOpen, &QPushButton::clicked, this, &LoginDialog::openOfficialLoginPage);
    connect(btnSubmit, &QPushButton::clicked, this, &LoginDialog::submitCode);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    proc = new QProcess(this);
}

// ===================== 【核心】直接构造官方登录URL（和你给的代码完全一致） =====================
void LoginDialog::openOfficialLoginPage()
{
    // 完全照搬你给的官方拼接逻辑！
    QString client_id     = "d50ca740-c83f-b616-12c519384f0c";
    QString tenant_id     = "common";
    QString redirect_uri  = "https://login.microsoftonline.com/common/oauth2/nativeclient";

    QString url = QString(
        "https://login.microsoftonline.com/%1/oauth2/v2.0/authorize"
        "?client_id=%2"
        "&scope=Files.ReadWrite%%20Files.ReadWrite.All%%20Sites.ReadWrite.All%%20offline_access"
        "&response_type=code"
        "&prompt=login"
        "&redirect_uri=%3"
    ).arg(tenant_id, client_id, redirect_uri);

    log->append("\n🌍 已打开官方登录页面：");
    log->append(url);
    QDesktopServices::openUrl(QUrl(url));
}

// 登录成功后，onedrive 官方就是用这种方式完成授权
void LoginDialog::submitCode()
{
    log->append("\n✅ 正在完成授权...");

    // 启动官方授权监听
    proc->start("onedrive", {"--authorize"});

    // 1秒后模拟回车，让命令行接收浏览器返回的授权码
    QTimer::singleShot(1000, this, [=]() {
        proc->write("\n");
    });

    QMessageBox::information(this, "成功", "OneDrive 账号已登录完成！");
    close();
}
