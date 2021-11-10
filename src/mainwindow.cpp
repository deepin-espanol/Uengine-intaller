#include "mainwindow.h"
#include <DTitlebar>
#include <QLayout>
#include <DSuggestButton>
#include <QFileDialog>
#include <DWidgetUtil>
MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
{
    initUI();
    apk=nullptr;
    connect(button_todeb,&QPushButton::clicked,[=](){
        QString path=QFileDialog::getSaveFileName(this, tr("Save File"),QDir::homePath()+"/"+apk->info().applicationName+"_"+apk->info().versionName+".deb", tr("Debian package (*.deb)"));
        if(!path.isEmpty()){
            apk->toDeb(path);
            setLoading(tr("Exporting deb, please wait..."));
        }
    });
    connect(button_install,&QPushButton::clicked,[=](){
       apk->install();
       setLoading(tr("Installing, please wait..."));
    });


}

MainWindow::~MainWindow()
{

}

void MainWindow::open(QString path)
{
    if(apk!=nullptr){
        delete apk;
    }
    apk=new Apk;
    qDebug()<<"检查环境是否完整"<<apk->clickEnv();
    connect(apk,&Apk::allDone,[=](int exitCode){
        spinner->hide();

        label_tip->show();
        if(exitCode==0){
            label_tip->setText(tr("Successful installation"));
        }else if(exitCode==100){
            label_tip->setText(tr("Export successfully"));
        }else {
            label_tip->setText(tr("Error:")+QString::number(exitCode));
        }

    });
    connect(apk,&Apk::iconOpened,[=](int code){
        qDebug()<<"Icon:"<<code;
        if(code==0){
            loadFinish(tr("Recommend you to install directly"));
            label_appIcon->setPixmap(apk->info().applicationIcon);
            MainWindow::setWindowIcon(apk->info().applicationIcon);
        }else {
            loadFinish(tr(""));
        }
        if(apk->info().component.isEmpty()){
            button_todeb->hide();
            button_install->hide();
            label_tip->show();
            label_tip->setText(tr("This software is not currently supported"));
        }
    });
    apk->open(path);
    label_appName->setText(apk->info().applicationName);
    label_appVersion->setText(tr("Version:")+apk->info().versionName);


}

void MainWindow::setLoading(QString message)
{

    button_todeb->hide();
    button_install->hide();
    spinner->show();
    spinner->start();
    if(message.isEmpty()){
        label_tip->hide();
    }else {
        label_tip->show();
        label_tip->setText(message);
    }


}

void MainWindow::loadFinish(QString message)
{
    button_todeb->show();
    button_install->show();
    spinner->hide();
    spinner->stop();
    if(message.isEmpty()){
        label_tip->hide();
    }else {
        label_tip->show();
        label_tip->setText(message);
    }
}


void MainWindow::initUI()
{
    setFixedSize(420,620);
    moveToCenter(this);
    titlebar()->setFullScreenButtonVisible(false);
    QWidget *w=new QWidget;
    setCentralWidget(w);
    QVBoxLayout* layout=new QVBoxLayout;
    w->setLayout(layout);

    QHBoxLayout* layout_icon=new QHBoxLayout;
    label_appIcon=new QLabel;
    layout_icon->addStretch();
    layout_icon->addWidget(label_appIcon);
    layout_icon->addStretch();
    label_appIcon->setFixedSize(128,128);
    label_appIcon->setScaledContents(true);
    label_appIcon->setPixmap(QIcon::fromTheme("application-vnd.android.package-archive").pixmap(128,128));

    QFont font_name;
    font_name.setPointSize(14);
    label_appName=new QLabel;
    label_appVersion=new QLabel;
    label_appName->setFont(font_name);
    label_appName->setFixedHeight(32);
    label_appVersion->setFixedHeight(32);
    label_appName->setAlignment(Qt::AlignCenter);
    label_appVersion->setAlignment(Qt::AlignCenter);

    QHBoxLayout* layout_btns=new QHBoxLayout;
    button_todeb=new QPushButton(tr("Save As Deb"));
    button_install=new DSuggestButton(tr("Install"));
    button_todeb->setFixedSize(180,36);
    button_install->setFixedSize(180,36);
    layout_btns->addStretch();
    layout_btns->addWidget(button_todeb);
    layout_btns->addWidget(button_install);
    layout_btns->addStretch();
    layout_btns->setSpacing(10);

    //中心部分
    spinner=new DSpinner;
    spinner->setFixedSize(75,75);
    spinner->start();
    QHBoxLayout *layout_spinner=new QHBoxLayout;
    layout_spinner->addStretch();
    layout_spinner->addWidget(spinner);
    layout_spinner->addStretch();
    spinner->hide();

    label_tip=new QLabel;
    label_tip->setAlignment(Qt::AlignCenter);


    layout->addSpacing(32);
    layout->addLayout(layout_icon);
    layout->addWidget(label_appName);
    layout->addWidget(label_appVersion);
    layout->addStretch();
    layout->addLayout(layout_spinner);
    layout->addWidget(label_tip);
    layout->addStretch();
    layout->addLayout(layout_btns);
    layout->addSpacing(18);
}
