#ifndef WEATHERADDRESS_H
#define WEATHERADDRESS_H


#include <QDialog>

class QHostInfo;
class QNetworkAccessManager;



class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void on_queryButton_clicked();
    void onHostLookedUp(const QHostInfo &host);
    void queryLocationOfIP(const QString& strIp);
    void replyFinished();

private:
    QVariantMap parseLocationData(const QByteArray& data);
    void showLocation(const QVariantMap& varMap);

};

#endif // WEATHERADDRESS_H
