#include "dialog.h"
#include<QMessageBox>
#include<QNetworkAccessManager>
#include<QHostInfo>
#include<QNetworkReply>
#include<QNetworkRequest>
#include<QJsonDocument>
#include<QJsonObject>
#include<QJsonParseError>
#include<QDebug>

const static QString AK_STR="你的AK";


Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    m_pNet(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    this->setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);
    this->setWindowIcon(QIcon(":/head.jpg"));
    this->setWindowTitle(QStringLiteral("获取IP地理位置"));
    ui->plainTextEdit->setEnabled(false);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_queryButton_clicked()
{
    const QString& txt=ui->lineEdit->text();
    if(txt==""){
        QMessageBox::warning(this,QStringLiteral("提示"),QStringLiteral("请输入IP地址或域名！"));
        return;
    }
    ui->plainTextEdit->clear();
    QHostInfo::lookupHost(txt,this,SLOT(onHostLookedUp(QHostInfo)));
}

void Dialog::onHostLookedUp(const QHostInfo &host)
{
    if(host.error()!=QHostInfo::NoError){
        QMessageBox::information(this,QStringLiteral("提示"),host.errorString());
        return;
    }
    QList<QHostAddress> adds = host.addresses();
    if(adds.size()){
        const QHostAddress& addr = adds.first();
        queryLocationOfIP(addr.toString());
        qDebug()<<addr.toString();

    }
}

void Dialog::queryLocationOfIP(const QString &strIp)
{
    const QString& strUrl=QString("http://api.map.baidu.com/location/ip?ak=%1&ip=%2&coor=bd09ll").
            arg(AK_STR).arg(strIp);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setUrl(QUrl(strUrl));

    QNetworkReply* reply =m_pNet->get(request);
    connect(reply,SIGNAL(finished()),this,SLOT(replyFinished()));

}

void Dialog::replyFinished()
{
    QNetworkReply* reply=qobject_cast<QNetworkReply *>(sender());

    if(reply->error()!=QNetworkReply::NoError){
        QMessageBox::information(this,QStringLiteral("提示"),QStringLiteral("请求出错:%1").arg(reply->errorString()));
    }
    const QByteArray& bytes=reply->readAll();
    const QVariantMap& varMap=parseLocationData(bytes);
    showLocation(varMap);

    reply->deleteLater();
}

QVariantMap Dialog::parseLocationData(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument doc=QJsonDocument::fromJson(data,&error);
    if(error.error!=QJsonParseError::NoError){
        QMessageBox::information(this,QStringLiteral("提示"),QStringLiteral("JSON:数据结构有问题！"));
        return QVariantMap();
    }
    QJsonObject obj=doc.object();
    int status=obj.value("status").toInt();
    if(status!=0){
        QMessageBox::information(this,QStringLiteral("提示"),QStringLiteral("状态码：%1").arg(status));
        return QVariantMap();
    }
    QJsonObject contentObj=obj.value("content").toObject();
    qDebug()<<contentObj.value("address").toString();
    return contentObj.toVariantMap();
}

void Dialog::showLocation(const QVariantMap &varMap)
{
    const QString& addrStr=varMap.value("address").toString();
    const QVariantMap& ptMap=varMap.value("point").toMap() ;
    const QString& longitudeStr=ptMap.value("x").toString(); //经度
    const QString& latitudeStr=ptMap.value("y").toString(); //纬度
    const QString& txt=QStringLiteral("城市：%1 \n经度：%2   纬度：%3").arg(addrStr).arg(longitudeStr).arg(latitudeStr);
    ui->plainTextEdit->setPlainText(txt);
}
