#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    QString syncDir() const;
    bool minTray() const;
    void setSyncDir(const QString &s);
    void setMinTray(bool b);

private slots:
    void browse();
    void login();
    void logout();

private:
    QLineEdit *dirEdit;
    QCheckBox *minCheck;
};

#endif
