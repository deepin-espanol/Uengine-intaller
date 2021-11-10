#include "apk.h"
#include <QProcess>
#include <QDebug>
#include <QTemporaryDir>
#include <DApplication>
#include <QRegExp>
Apk::Apk(QObject *parent) : QObject(parent)
{
    autoinstall=false;
}

Apk::~Apk()
{
}

bool Apk::open(QString path)
{

    m_filePath=path;
    QProcess run;
    run.start("aapt",{"d","badging",path});
    run.waitForFinished();
    if(run.exitCode()!=0)
        return false;
    QString out=run.readAllStandardOutput();
    m_apkInfo.packageName = getCenter(out,"package: name='","'");
    m_apkInfo.versionCode = getCenter(out,"versionCode='","'");
    m_apkInfo.versionName = getCenter(out,"versionName='","'").split(" ")[0];
    m_apkInfo.applicationName = getCenter(out,"application-label:'","'");
    m_apkInfo.component = getCenter(out,"launchable-activity: name='","'");
    QString iconPath= getCenter(out,"application-icon-480:'","'");
    QTemporaryDir *tmpdir=new QTemporaryDir;
    if(tmpdir->isValid()){
        connect(this,&Apk::iconOpened,[=](){tmpdir->remove();delete tmpdir;});
        qDebug()<<"TMP:"<<tmpdir->path();
        QProcess unzip;
        unzip.start("unzip",{path,iconPath,"-d",tmpdir->path()});
        unzip.waitForFinished();
        if(!m_apkInfo.applicationIcon.load(tmpdir->path()+"/"+iconPath) || m_apkInfo.component.isEmpty()){
            qDebug()<<"直接读取图标失败";
            QProcess *run2=new QProcess;
            run2->start("apktool",{"d","-o",tmpdir->path()+"/apk_res","-s",m_filePath});
            connect(run2, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                 [=](int exitCode, QProcess::ExitStatus exitStatus){
                if(exitCode==0){



                    QString mainxml;
                    QFile xml(tmpdir->path()+"/apk_res/AndroidManifest.xml");
                    xml.open(QFile::ReadOnly);
                    mainxml=xml.readAll();

                    if(m_apkInfo.component.isEmpty()){
                        m_apkInfo.component=getCenter(mainxml,"android:name=\"",".MainActivity\"",true);
                        m_apkInfo.component.append(".MainActivity");
                        qDebug()<<m_apkInfo.component;
                    }

                    QString name = getCenter(mainxml,"android:icon=\"@mipmap/","\"");
                    qDebug()<<"找到图标："+name;
                    //获取mipmap列表
                    QDir cfgPathDir(tmpdir->path()+"/apk_res/res");
                    QStringList filters;
                    filters << QString("mipmap*");
                    cfgPathDir.setFilter(QDir::Dirs | QDir::NoSymLinks); //设置类型过滤器，只为文件格式
                    cfgPathDir.setNameFilters(filters);                   //设置文件名称过滤器，只为filters格式
                    int dirCount = int(cfgPathDir.count());
                    if(dirCount<=0){
                        emit iconOpened(-1);//没发现任何mipmap开头的目录
                    }
                    bool isFinded=false;
                    for (int i=dirCount-1;i>=0;i--) {
                        qDebug()<<cfgPathDir[i];
                        QDir dirMipmap(tmpdir->path()+"/apk_res/res/"+cfgPathDir[i]);
                        QStringList filters2;
                        filters2 << name+"*";
                        dirMipmap.setFilter(QDir::Files);
                        dirMipmap.setNameFilters(filters2);
                        int iconCount = int(dirMipmap.count());
                        if(iconCount > 0){
                            if(m_apkInfo.applicationIcon.load(tmpdir->path()+"/apk_res/res/"+cfgPathDir[i]+"/"+dirMipmap[0])){
                                emit iconOpened(0);
                                isFinded=true;
                                break;
                            }else {
                                continue;
                            }

                        }else {
                            continue;
                        }
                    }
                    if(!isFinded){
                        emit iconOpened(1);
                    }

                }else {
                    emit iconOpened(exitCode);
                }
            });
        }else {
            qDebug()<<"直接读取图标成功";
            emit iconOpened(0);
        }
    }else {

        emit iconOpened(6);
        return false;
    }

    qDebug()<<"---------App---------"
           <<"\nPackage name:"<<m_apkInfo.packageName
          <<"\nApplication name:"<<m_apkInfo.applicationName
         <<"\nVersion:"<<m_apkInfo.versionName<<"("<<m_apkInfo.versionCode<<")"
        <<"\nLaunchable activity:"<<m_apkInfo.component
       <<"\n---------------------";
    return true;


}
//检查依赖是否已經安装
bool Apk::clickEnv()
{
    QProcess *run=new QProcess;
    run->start("aapt",{"v"});
    run->waitForFinished();
    if(run->exitCode()!=0){
        delete run;
        return false;
    }
    run->start("unzip",{"-v"});
    run->waitForFinished();
    if(run->exitCode()!=0){
        delete run;
        return false;
    }
    run->start("fakeroot",{"-v"});
    run->waitForFinished();
    if(run->exitCode()!=0){
        delete run;
        return false;
    }
    delete run;
    return true;
}

ApkInfo Apk::info()
{
    return m_apkInfo;
}

bool Apk::toDeb(QString path)
{
    QTemporaryDir* tmpdir=new QTemporaryDir;
    if(tmpdir->isValid()){
        QDir debdir(tmpdir->path());
        debdir.mkdir("deb");
        debdir.setPath(debdir.path()+"/deb");
        //        copyPath(QApplication::applicationDirPath()+"/template",tmpdir->path()+"/deb");
        debdir.mkdir("DEBIAN");
        debdir.mkpath("usr/share/applications");
        debdir.mkpath("usr/share/uengine/apk");
        debdir.mkpath("usr/share/uengine/icons");
        QFile t(":/pkg/template/DEBIAN/control");
        t.open(QFile::ReadOnly);
        QString s_control=t.readAll();
        t.close();
        QString tpkgname=m_apkInfo.packageName;
        tpkgname= tpkgname.replace(QRegExp("[^0-9a-zA-Z.+\\-]"),".");
        s_control.replace("<<packageName>>","uengine-dc."+tpkgname);
        s_control.replace("<<version>>",m_apkInfo.versionName);
        QFile f_control(debdir.path()+"/DEBIAN/control");
        qDebug()<<f_control.open(QFile::WriteOnly);
        f_control.write(s_control.toUtf8());
        f_control.close();

        QFile t1(":/pkg/template/DEBIAN/postinst");
        t1.open(QFile::ReadOnly);
        QString s_postinst=t1.readAll();
        t1.close();
        QFileInfo apkfileinfo(m_filePath);
        s_postinst.replace("<<apkFileName>>",m_apkInfo.packageName+".apk");
        s_postinst.replace("<<iconFile>>","/usr/share/uengine/icons/"+m_apkInfo.packageName+".png");
        s_postinst.replace("<<desktopFile>>","/usr/share/applications/uengine."+m_apkInfo.packageName+".desktop");
        QFile f_postinst(debdir.path()+"/DEBIAN/postinst");
        f_postinst.open(QFile::WriteOnly);
        f_postinst.write(s_postinst.toUtf8());
        f_postinst.close();



        QFile t2(":/pkg/template/DEBIAN/prerm");
        t2.open(QFile::ReadOnly);
        QString s_prerm=t2.readAll();
        t2.close();
        s_prerm.replace("<<packageName>>",m_apkInfo.packageName);
        QFile f_prerm(debdir.path()+"/DEBIAN/prerm");
        f_prerm.open(QFile::WriteOnly);
        f_prerm.write(s_prerm.toUtf8());
        f_prerm.close();
        QProcess chmod;
        chmod.start("chmod",{"+x",debdir.path()+"/DEBIAN/postinst"});
        chmod.waitForFinished();
        chmod.start("chmod",{"+x",debdir.path()+"/DEBIAN/prerm"});
        chmod.waitForFinished();


        QFile f_apkfile(m_filePath);
        f_apkfile.copy(debdir.path()+"/usr/share/uengine/apk/"+m_apkInfo.packageName+".apk");
        f_apkfile.close();

        QFile f_icon(debdir.path()+"/usr/share/uengine/icons/"+m_apkInfo.packageName+".png");
        f_icon.open(QFile::ReadOnly);
        m_apkInfo.applicationIcon.save(&f_icon,"PNG");
        f_icon.close();

        QFile t3(":/pkg/template/usr/share/applications/uengine.name.desktop");
        t3.open(QFile::ReadOnly);
        QString s_desktop=t3.readAll();
        t3.close();
        s_desktop.replace("<<packageName>>",m_apkInfo.packageName);
        s_desktop.replace("<<component>>",m_apkInfo.component);
        s_desktop.replace("<<icon>>","/usr/share/uengine/icons/"+m_apkInfo.packageName+".png");
        s_desktop.replace("<<appName>>",m_apkInfo.applicationName);
        QFile f_desktop(debdir.path()+"/usr/share/applications/uengine."+m_apkInfo.packageName+".desktop");
        f_desktop.open(QFile::WriteOnly);
        f_desktop.write(s_desktop.toUtf8());
        f_desktop.close();
        QProcess *dpkg=new QProcess;
        dpkg->start("fakeroot",{"dpkg","-b",debdir.path(),tmpdir->path()+"/uengine."+m_apkInfo.packageName+"_"+m_apkInfo.versionName+".deb"});
        connect(this,&Apk::allDone,[=](){tmpdir->remove();delete tmpdir;});
        connect(dpkg, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
             [=](int exitCode, QProcess::ExitStatus exitStatus){
            if(exitCode==0){
                if(path!=""){
                    QFile deb(tmpdir->path()+"/uengine."+m_apkInfo.packageName+"_"+m_apkInfo.versionName+".deb");
                    deb.copy(path);
                    deb.close();
                }
                dpkg->deleteLater();
                if(autoinstall){
                    QProcess *install=new QProcess;
                    install->start("pkexec",{"apt","install","-y",tmpdir->path()+"/uengine."+m_apkInfo.packageName+"_"+m_apkInfo.versionName+".deb"});
                    connect(install, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                         [=](int exitCode, QProcess::ExitStatus exitStatus){
                        emit allDone(exitCode);  //打包成功且安装的结果
                    });
                }else {
                    emit allDone(100); //打包成功且无需自动安装
                }
            }else{
                emit allDone(exitCode+100);  //打包失败结果
            }
            autoinstall=false;
        });
    }else {
        return false;
    }
}

bool Apk::install()
{
    autoinstall=true;
    toDeb();
}

bool copyPath(QString src, QString dst)
{

    QDir dir(src);
    if (! dir.exists())
        return false;

    foreach (QString d, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString dst_path = dst + QDir::separator() + d;
        dir.mkpath(dst_path);
        copyPath(src+ QDir::separator() + d, dst_path);//use recursion
    }

    foreach (QString f, dir.entryList(QDir::Files)) {
        QFile::copy(src + QDir::separator() + f, dst + QDir::separator() + f);
    }
    return true;
}


QString getCenter(QString s, QString s1, QString s2, bool fromEnd) //获取s中夹s1和s2中间的字符串
{
    if(!fromEnd){
        if(s.indexOf(s1)==-1){
            return "";
        }
        int start = s.indexOf(s1) + s1.length();
        int end;
        QString ss=s.right(s.length()-start);
        end=ss.indexOf(s2);
        return ss.left(end);
    }else {
        if(s.indexOf(s2)==-1){
            return "";
        }
        int start = -1;
        int end =s.indexOf(s2);
        QString ss=s.left(end);
        qDebug()<<ss;
        start=ss.lastIndexOf(s1)+s1.length();
        return ss.right(ss.length()-start);
    }

}
