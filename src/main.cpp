#include "mainwindow.h"
#include <DApplication>
#include "src/opendialog.h"
#include <QTranslator>
#include <DAboutDialog>
DWIDGET_USE_NAMESPACE
int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    DAboutDialog dialog;

         a.setAttribute(Qt::AA_UseHighDpiPixmaps);
         a.setProductName("APK installer");
         a.setApplicationName("APK installer");
         a.setAboutDialog(&dialog);

         //Title
         dialog.setWindowTitle("APK installer");
         //descritcion
         dialog.setProductName("<span>apk installer for Deepin/UOS</span>");
         //icons
         dialog.setProductIcon(QIcon::fromTheme("application-vnd.android.package-archive"));
         dialog.setCompanyLogo(QPixmap(":/src/image/deepines.png"));
         //about
         dialog.setDescription(
             "<span style=' font-size:8pt; font-weight:600;'>Deepin en Español </span>"
             "<a href='https://deepinenespañol.org'>https://deepinenespañol.org</a><br/>"
             "<span style=' font-size:8pt; font-weight:600;'>Deepin Latin Code - developers</span>");
         dialog.setVersion(DApplication::buildVersion("Version 1.0"));
         dialog.setWebsiteName("deepinenespañol.org");
         dialog.setWebsiteLink("https://deepinenespañol.org");

    QTranslator qtTranslator;
    qtTranslator.load(a.applicationDirPath()+"/translations/apktoDeb." + QLocale::system().name());
    a.installTranslator(&qtTranslator);
    OpenDialog dlg;

    if(argc==2){
        QString path(argv[1]);
        dlg.open(path);
    }else {
        dlg.show();
    }



    return a.exec();
}
