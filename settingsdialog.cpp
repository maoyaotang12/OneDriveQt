#include "settingsdialog.h"
#include "logindialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QLabel>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setFixedSize(500, 300);
    QVBoxLayout *lay = new QVBoxLayout(this);

    QHBoxLayout *dLay = new QHBoxLayout;
    dirEdit = new QLineEdit;
    QPushButton *btn = new QPushButton("浏览");
    dLay->addWidget(new QLabel("同步目录:"));
    dLay->addWidget(dirEdit);
    dLay->addWidget(btn);
    lay->addLayout(dLay);

    minCheck = new QCheckBox("关闭最小化到托盘");
    lay->addWidget(minCheck);

    QPushButton *loginBtn = new QPushButton("登录账号");
    QPushButton *logoutBtn = new QPushButton("注销账号");
    lay->addWidget(loginBtn);
    lay->addWidget(logoutBtn);

    QPushButton *ok = new QPushButton("确定");
    QPushButton *cancel = new QPushButton("取消");
    QHBoxLayout *btnLay = new QHBoxLayout;
    btnLay->addWidget(ok);
    btnLay->addWidget(cancel);
    lay->addLayout(btnLay);

    connect(btn, &QPushButton::clicked, this, &SettingsDialog::browse);
    connect(loginBtn, &QPushButton::clicked, this, &SettingsDialog::login);
    connect(logoutBtn, &QPushButton::clicked, this, &SettingsDialog::logout);
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
}

void SettingsDialog::browse() {
    QString d = QFileDialog::getExistingDirectory(this);
    if (!d.isEmpty()) dirEdit->setText(d);
}

void SettingsDialog::login() { LoginDialog d; d.exec(); }
void SettingsDialog::logout() { QProcess::execute("onedrive --logout"); QMessageBox::information(this, "", "已注销"); }

QString SettingsDialog::syncDir() const { return dirEdit->text(); }
bool SettingsDialog::minTray() const { return minCheck->isChecked(); }
void SettingsDialog::setSyncDir(const QString &s) { dirEdit->setText(s); }
void SettingsDialog::setMinTray(bool b) { minCheck->setChecked(b); }
