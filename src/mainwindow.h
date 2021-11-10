#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include "src/apk.h"
#include <QLabel>
#include <QPushButton>
#include <DSpinner>
DWIDGET_USE_NAMESPACE
class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void open(QString path);
    void setLoading(QString message="");
    void loadFinish(QString message="");




private:
    void initUI();
    QLabel *label_appIcon;
    QLabel *label_appName;
    QLabel *label_appVersion;
    QPushButton *button_install;
    QPushButton *button_todeb;
    DSpinner *spinner;
    QLabel *label_tip;
    Apk *apk;
};

#endif // MAINWINDOW_H
