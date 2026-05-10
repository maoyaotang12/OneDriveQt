#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QProcess>
#include <QTimer>

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);

private slots:
    void openOfficialLoginPage();
    void submitCode();

private:
    QTextEdit *log;
    QProcess *proc;
};

#endif
