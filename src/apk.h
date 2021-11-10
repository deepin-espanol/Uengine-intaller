#ifndef APK_H
#define APK_H

#include <QObject>
#include <QPixmap>
#include <QTemporaryDir>
struct ApkInfo{
    QString packageName;
    QString versionName;
    QString versionCode;
    QString applicationName;
    QPixmap applicationIcon;
    QString component;
};

QString getCenter(QString s, QString s1,QString s2, bool fromEnd=false);
bool copyPath(QString ,QString );
class Apk : public QObject
{
    Q_OBJECT
public:
    explicit Apk(QObject *parent = nullptr);
    ~Apk();

    bool open(QString path);
    bool clickEnv();
    ApkInfo info();
    bool toDeb(QString path="");
    bool install();
    bool autoinstall;


private:
    QString m_filePath;
    ApkInfo m_apkInfo;
signals:
    void allDone(int exitCode);
    void iconOpened(int code);
public slots:
};

#endif // APK_H
