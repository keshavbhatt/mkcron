#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebEngineScript>
#include <QWebEngineProfile>
#include <QWebEngineView>

#include <QFile>
#include <QSettings>

#include "manager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void urlChanged(QUrl url);

public:
    explicit MainWindow(QWidget *parent = nullptr, QString serverBase="");
    ~MainWindow();

public slots:
    void loadUrl(QString urlStr);
protected slots:
    void closeEvent(QCloseEvent *event);


private slots:
    void setStyle(QString fname);

private:
    Ui::MainWindow *ui;
    QString home;
    Manager * managerWidget = nullptr;
    QSettings settings;
    QString humanReadable = "";
    QString definition = "";
};

#endif // MAINWINDOW_H
