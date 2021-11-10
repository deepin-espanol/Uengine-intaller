#include "opendialog.h"
#include <QLayout>
#include <QFileDialog>
#include <DWidgetUtil>
#include <QDropEvent>
#include <QMimeData>
#include <QMimeDatabase>
#include <QGraphicsDropShadowEffect>
#include <DGuiApplicationHelper>
OpenDialog::OpenDialog(QWidget *parent) : DMainWindow(parent)
{

    setMinimumSize(500,400);
    moveToCenter(this);
    QPushButton *button_open=new QPushButton;
    button_open->setText(tr("Click here to open the APK"));
    button_open->setStyleSheet("color:#0066EC; border:none; background:none");
    button_open->setCursor(Qt::PointingHandCursor);
    w=new QWidget;
    drop=new QWidget;
    drop->setStyleSheet("border:2px dashed rgba(128,128,128,0.3); border-radius:8px");

    QLabel* label_icon=new QLabel();
    label_icon->setPixmap(QIcon::fromTheme("application-vnd.android.package-archive").pixmap(128,128));
    setWindowIcon(QIcon::fromTheme("application-vnd.android.package-archive"));
    label_icon->setAlignment(Qt::AlignCenter);
    label_icon->setStyleSheet("border:none");
    label_icon->setFixedHeight(128);
    QLabel* label_tip=new QLabel(tr("You can drag the package here\nor"));
    label_tip->setAlignment(Qt::AlignCenter);
    label_tip->setStyleSheet("border:none");
    QVBoxLayout *layout_drop=new QVBoxLayout;
    layout_drop->addStretch();
    layout_drop->addWidget(label_icon);
    layout_drop->addWidget(label_tip);
    layout_drop->addWidget(button_open);
    layout_drop->addStretch();
    drop->setLayout(layout_drop);
    setCentralWidget(w);
    QVBoxLayout *layout=new QVBoxLayout;
    layout->addWidget(drop);
    layout->setContentsMargins(20,20,20,20);
    layout->setSpacing(18);
    w->setLayout(layout);


    setAcceptDrops(true);

    mainwindow=new MainWindow;

    connect(button_open,&QPushButton::clicked,[=](){
        QString path=QFileDialog::getOpenFileName(this,tr("Open"),QDir::homePath(),tr("Android application package (*.apk)"));
        if(!path.isEmpty()){
            open(path);
        }
    });
    //    mainwindow->show();
}

void OpenDialog::open(QString path)
{
    hide();
    mainwindow->setLoading(tr("Searching icon"));
    mainwindow->open(path);
    mainwindow->show();

}

void OpenDialog::dragEnterEvent(QDragEnterEvent *event)
{
    QString path=event->mimeData()->text();
    QMimeDatabase db;
    QString type = db.mimeTypeForFile(path).name();
    if(type=="application/vnd.android.package-archive"){
        QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
        QColor c = DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
        drop->setStyleSheet("border:2px dashed "+c.name()+"; border-radius:8px");
        shadow_effect->setOffset(0, 5);
        shadow_effect->setColor(c.name());
        shadow_effect->setBlurRadius(100);
        drop->setGraphicsEffect(shadow_effect);
        event->acceptProposedAction();
    }
}

void OpenDialog::dropEvent(QDropEvent *event)
{
    open(QUrl(event->mimeData()->text()).path());
}

void OpenDialog::dragLeaveEvent(QDragLeaveEvent *event)
{
    drop->setGraphicsEffect(nullptr);

    drop->setStyleSheet("border:2px dashed rgba(128,128,128,0.3); border-radius:8px");
}
