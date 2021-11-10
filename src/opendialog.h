#ifndef OPENDIALOG_H
#define OPENDIALOG_H

#include <QObject>
#include <QWidget>
#include <DSuggestButton>
#include <DMainWindow>
#include "mainwindow.h"
DWIDGET_USE_NAMESPACE
class OpenDialog : public DMainWindow
{
    Q_OBJECT
public:
    explicit OpenDialog(QWidget *parent = nullptr);
    QWidget *drop;
    QWidget *w;
    MainWindow *mainwindow;
    void open(QString path);

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);

signals:
public slots:
};

#endif // OPENDIALOG_H
