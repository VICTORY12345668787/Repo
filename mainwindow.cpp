#include "mainwindow.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>
#include<QHostInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPainter>
#include <QPen>
#include "ui_mainwindow.h"
#include "weathertool.h"

#define INCREMENT 3 //温度每升高/降低1°，y轴坐标的增量
#define POINT_RADIUS 3 //曲线描点的大小
#define TEXT_OFFSET_X 12
#define TEXT_OFFSET_Y 12


const static QString AK_STR="azOhhhbH7PcEf0uPQaCYlD0djxrIu8zn";

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    //设置窗口属性
    setWindowFlag(Qt::FramelessWindowHint);  // 无边框
    setFixedSize(width(), height());         // 固定窗口大小

    // 右键菜单：退出程序
    mExitMenu = new QMenu(this);
    mExitAct = new QAction();
    mExitAct->setText(tr("退出"));
    mExitAct->setIcon(QIcon(":/res/close.png"));
    mExitMenu->addAction(mExitAct);

    connect(mExitAct, &QAction::triggered, this, [=]() { qApp->exit(0); });

    //将控件添加到控件数组
    //星期和日期
    mWeekList << ui->lblWeek0 << ui->lblWeek1 << ui->lblWeek2 << ui->lblWeek3 << ui->lblWeek4 << ui->lblWeek5;
    mDateList << ui->lblDate0 << ui->lblDate1 << ui->lblDate2 << ui->lblDate3 << ui->lblDate4 << ui->lblDate5;

    //天气和天气图标
    mTypeList << ui->lblType0 << ui->lblType1 << ui->lblType2 << ui->lblType3 << ui->lblType4 << ui->lblType5;
    mTypeIconList << ui->lblTypeIcon0 << ui->lblTypeIcon1 << ui->lblTypeIcon2 << ui->lblTypeIcon3 << ui->lblTypeIcon4 << ui->lblTypeIcon5;


    //天气指数
    mAqiList << ui->lblQuality0 << ui->lblQuality1 << ui->lblQuality2 << ui->lblQuality3 << ui->lblQuality4 << ui->lblQuality5;

    //风力和风向
    mFxList << ui->lblFx0 << ui->lblFx1 << ui->lblFx2 << ui->lblFx3 << ui->lblFx4 << ui->lblFx5;
    mFlList << ui->lblFl0 << ui->lblFl1 << ui->lblFl2 << ui->lblFl3 << ui->lblFl4 << ui->lblFl5;

    mTypeMap.insert("暴雪",":/res/type/BaoXue.png");
    mTypeMap.insert("暴雨",":/res/type/BaoYu.png");
    mTypeMap.insert("暴雨到大暴雨",":/res/type/BaoYuDaoDaBaoYu.png");
    mTypeMap.insert("大暴雨",":/res/type/DaBaoYu.png");
    mTypeMap.insert("大暴雨到特大暴雨",":/res/type/DaBaoYuDaoTeDaBaoYu.png");
    mTypeMap.insert("大到暴雪",":/res/type/DaDaoBaoXue.png");
    mTypeMap.insert("大雪",":/res/type/DaXue.png");
    mTypeMap.insert("大雨",":/res/type/DaYu.png");
    mTypeMap.insert("冻雨",":/res/type/DongYu.png");
    mTypeMap.insert("多云",":/res/type/DuoYun.png");
    mTypeMap.insert("浮沉",":/res/type/FuChen.png");
    mTypeMap.insert("雷阵雨",":/res/type/LeiZhenYu.png");
    mTypeMap.insert("雷阵雨伴有冰雹",":/res/type/LeiZhenYuBanYouBingBao.png");
    mTypeMap.insert("霾",":/res/type/Mai.png");
    mTypeMap.insert("强沙尘暴",":/res/type/QiangShaChenBao.png");
    mTypeMap.insert("晴",":/res/type/Qing.png");
    mTypeMap.insert("沙尘暴",":/res/type/ShaChenBao.png");

    mTypeMap.insert("特大暴雨",":/res/type/TeDaBaoYu.png");
    mTypeMap.insert("undefined",":/res/type/undefined.png");
    mTypeMap.insert("雾",":/res/type/Wu.png");
    mTypeMap.insert("小到中雪",":/res/type/XiaoDaoZhongXue.png");
    mTypeMap.insert("小到中雨",":/res/type/XiaoDaoZhongYu.png");
    mTypeMap.insert("小雪",":/res/type/XiaoXue.png");
    mTypeMap.insert("小雨",":/res/type/XiaoYu.png");
    mTypeMap.insert("雪",":/res/type/Xue.png");
    mTypeMap.insert("扬沙",":/res/type/YangSha.png");
    mTypeMap.insert("阴",":/res/type/Yin.png");
    mTypeMap.insert("雨",":/res/type/Yu.png");
    mTypeMap.insert("雨夹雪",":/res/type/YuJiaXue.png");
    mTypeMap.insert("阵雪",":/res/type/ZhenXue.png");
    mTypeMap.insert("阵雨",":/res/type/ZhenYu.png");
    mTypeMap.insert("中到大雪",":/res/type/ZhongDaoDaXue.png");
    mTypeMap.insert("中到大雨",":/res/type/ZhongDaoDaYu.png");
    mTypeMap.insert("中雪",":/res/type/ZhongXue.png");
    mTypeMap.insert("中雨",":/res/type/ZhongYu.png");



    //网络请求
    mNetAccessManager = new QNetworkAccessManager(this);
    connect(mNetAccessManager,&QNetworkAccessManager::finished,this,&MainWindow::onReplied);


    //通过ip地址定位所在城市
    m_pNet =new QNetworkAccessManager(this);
    const QString& txt="116.16.241.59";
    QHostInfo::lookupHost(txt,this,SLOT(onHostLookedUp(QHostInfo)));

    //给标签添加事件过滤器
    ui->lblHighCurve->installEventFilter(this);
    ui->lblLowCurve->installEventFilter(this);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::contextMenuEvent(QContextMenuEvent* event) {
    mExitMenu->exec(QCursor::pos());
    event->accept();
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
    qDebug() << "窗口左上角：" << this->pos() << ", 鼠标坐标点："
             << event->globalPos();
    mOffset = event->globalPos() - this->pos();
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    this->move(event->globalPos() - mOffset);
}

void MainWindow::getWeatherInfo(QString cityName)
{
    QString cityCode = WeatherTool::getCityCode(cityName);
    if(cityCode.isEmpty()) {
        QMessageBox::warning(this,"天气","请检查输入是否正确！",QMessageBox::Ok);
        return;
    }
    QUrl url("http://t.weather.itboy.net/api/weather/city/" + cityCode);
    mNetAccessManager->get(QNetworkRequest(url));
}

void MainWindow::parseJson(QByteArray &byteArray)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(byteArray,&err);
    if(err.error != QJsonParseError::NoError) {
        return;
    }

    QJsonObject rootObj = doc.object();

    //1.解析日期和城市
    mToday.date = rootObj.value("date").toString();
    mToday.city = rootObj.value("cityInfo").toObject().value("city").toString();

    //2.解析yesterday
    QJsonObject objData = rootObj.value("data").toObject();

    QJsonObject objYesterday = objData.value("yesterday").toObject();
    mDay[0].week =objYesterday.value("week").toString();
    mDay[0].date =objYesterday.value("ymd").toString();

    mDay[0].type =objYesterday.value("type").toString();

    QString s;
    s = objYesterday.value("high").toString().split(" ").at(1);
    mDay[0].high = s.left(s.length()-1).toInt();


    s = objYesterday.value("low").toString().split(" ").at(1);
    mDay[0].low = s.left(s.length()-1).toInt();


    //风向风力
    mDay[0].fx =objYesterday.value("fx").toString();
    mDay[0].fl =objYesterday.value("fl").toString();

    //污染指数
    mDay[0].aqi =objYesterday.value("aqi").toDouble();

    //3.解析forecast中五天的数据
    QJsonArray forecastArr = objData.value("forecast").toArray();

    for(int i=0;i<5;i++) {
        QJsonObject objForecast =forecastArr[i].toObject();
        mDay[i+1].week = objForecast.value("week").toString();
        mDay[i+1].date = objForecast.value("ymd").toString();

        //天气类型
        mDay[i+1].type =objForecast.value("type").toString();

        //高温低温
        QString s;
        s = objForecast.value("high").toString().split(" ").at(1);
        mDay[i+1].high = s.left(s.length()-1).toInt();

        s = objForecast.value("low").toString().split(" ").at(1);
        mDay[i+1].low = s.left(s.length()-1)
                .toInt();
        //风向风力
        mDay[i+1].fx =objForecast.value("fx").toString();
        mDay[i+1].fl =objForecast.value("fl").toString();

        //污染指数
        mDay[i+1].aqi =objForecast.value("aqi").toDouble();
    }

    //4.解析今天的数据
    mToday.ganmao = objData.value("ganmao").toString();

    mToday.wendu = objData.value("wendu").toString().toInt();
    mToday.shidu = objData.value("shidu").toString();
    mToday.pm25 = objData.value("pm25").toDouble();
    mToday.quality = objData.value("quality").toString();

    mToday.type =mDay[1].type;

    mToday.fx =mDay[1].fx;
    mToday.fl =mDay[1].fl;

    mToday.high =mDay[1].high;
    mToday.low =mDay[1].low;

    //5.更新UI
    updateUI();


    ui->lblHighCurve->update();
    ui->lblLowCurve->update();
}

void MainWindow::updateUI()
{
    //更新日期和城市
    ui->lblDate->setText(QDateTime::fromString(mToday.date,"yyyyMMdd").toString("yyyy/MM/dd") + " " + mDay[1].week);
    ui->lblCity->setText(mToday.city);

    //2.更新今天
    ui->lblTypeIcon->setPixmap(mTypeMap[mToday.type]);
    ui->lblTemp->setText(QString::number(mToday.wendu)+"°");
    ui->lblLowHigh->setText(QString::number(mToday.low) + "~" + QString::number(mToday.high) + "°C");
    ui->lblGanMao->setText("感冒指数：" + mToday.ganmao);
    ui->lblWindFx->setText(mToday.fx);
    ui->lblWindFl->setText(mToday.fl);
    ui->lblPM25->setText(QString::number(mToday.pm25));
    ui->lblShiDu->setText(mToday.shidu);
    ui->lblQuality->setText(mToday.quality);

    //3.更新六天
    for(int i=0;i<6;i++) {
        //3.1更新时间和日期
        mWeekList[i]->setText("周" + mDay[i].week.right(1));
        ui->lblWeek0->setText("昨天");
        ui->lblWeek1->setText("今天");
        ui->lblWeek2->setText("明天");

        QStringList ymdList =mDay[i].date.split("-");
        mDateList[i]->setText(ymdList[1] + "/" + ymdList[2]);

        //3.2更新天气类型
        mTypeList[i]->setText(mDay[i].type);
        mTypeIconList[i]->setPixmap(mTypeMap[mDay[i].type]);

        //3.3更新空气质量
        if(mDay[i].aqi >= 0 &&mDay[i].aqi <= 50) {
            mAqiList[i]->setText("优");
            mAqiList[i]->setStyleSheet("background-color: rgb(121,184,0);");
        } else if(mDay[i].aqi > 50 && mDay[i].aqi <= 100) {
            mAqiList[i]->setText("良");
            mAqiList[i]->setStyleSheet("background-color: rgb(255,187,23);");
        } else if(mDay[i].aqi > 100 && mDay[i].aqi <= 150) {
            mAqiList[i]->setText("轻度");
            mAqiList[i]->setStyleSheet("background-color: rgb(255,87,97);");
        } else if(mDay[i].aqi > 150 && mDay[i].aqi <= 200) {
            mAqiList[i]->setText("中度");
            mAqiList[i]->setStyleSheet("background-color: rgb(235,17,27);");
        } else if(mDay[i].aqi > 200 && mDay[i].aqi <= 250) {
            mAqiList[i]->setText("重度");
            mAqiList[i]->setStyleSheet("background-color: rgb(170,0,0);");
        } else {
            mAqiList[i]->setText("严重");
            mAqiList[i]->setStyleSheet("background-color: rgb(110,0,0);");
        }

        //3.4 更新风力、风向
        mFxList[i]->setText(mDay[i].fx);
        mFlList[i]->setText(mDay[i].fl);

    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->lblHighCurve && event->type() == QEvent::Paint){
        paintHighCurve();
    }

    if(watched == ui->lblLowCurve && event->type() == QEvent::Paint){
        paintLowCurve();
    }

    return QWidget::eventFilter(watched,event);
}

void MainWindow::paintHighCurve()
{
    QPainter painter(ui->lblHighCurve);

    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing,true);

    //1.获取x坐标
    int pointX[6] ={0};

    for(int i=0;i<6;i++) {
        pointX[i] = mWeekList[i]->pos().x() + mWeekList[i]->width()/2;
    }

    //2.获取y坐标

    int tempSum =0;
    int tempAverage =0;
    for(int i=0;i<6;i++) {
        tempSum += mDay[i].high;
    }
    tempAverage =tempSum/6;

    int pointY[6] = {0};
    int yCenter = ui->lblHighCurve->height()/2;
    for(int i=0;i<6;i++) {
        pointY[i] =yCenter - ((mDay[i].high-tempAverage)* INCREMENT);
    }

    //3.开始绘制
    //3.1初始化画笔
    QPen pen = painter.pen();
    pen.setWidth(1);
    pen.setColor(QColor(255,170,0));

    painter.setPen(pen);
    painter.setBrush(QColor(255,170,0));
    //3.2画点、写文本
    for(int i=0;i<6;i++) {
        //显示点
        painter.drawEllipse(QPoint(pointX[i],pointY[i]),POINT_RADIUS,POINT_RADIUS);

        //显示文本
        painter.drawText(pointX[i]-TEXT_OFFSET_X,pointY[i]-TEXT_OFFSET_Y,QString::number(mDay[i].high) + "°");
    }

    //3.3绘制曲线
    for(int i=0;i<5;i++) {
        if(i == 0){
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
        } else {
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
        }
        painter.drawLine(pointX[i],pointY[i],pointX[i+1],pointY[i+1]);
    }
}

void MainWindow::paintLowCurve()
{
    QPainter painter(ui->lblLowCurve);

    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing,true);

    //1.获取x坐标
    int pointX[6] ={0};

    for(int i=0;i<6;i++) {
        pointX[i] = mWeekList[i]->pos().x() + mWeekList[i]->width()/2;
    }

    //2.获取y坐标

    int tempSum =0;
    int tempAverage =0;
    for(int i=0;i<6;i++) {
        tempSum += mDay[i].low;
    }
    tempAverage =tempSum/6;

    int pointY[6] = {0};
    int yCenter = ui->lblLowCurve->height()/2;
    for(int i=0;i<6;i++) {
        pointY[i] =yCenter - ((mDay[i].low-tempAverage)* INCREMENT);
    }

    //3.开始绘制
    //3.1初始化画笔
    QPen pen = painter.pen();
    pen.setWidth(1);
    pen.setColor(QColor(0,255,255));

    painter.setPen(pen);
    painter.setBrush(QColor(0,255,255));
    //3.2画点、写文本
    for(int i=0;i<6;i++) {
        //显示点
        painter.drawEllipse(QPoint(pointX[i],pointY[i]),POINT_RADIUS,POINT_RADIUS);

        //显示文本
        painter.drawText(pointX[i]-TEXT_OFFSET_X,pointY[i]-TEXT_OFFSET_Y,QString::number(mDay[i].low) + "°");
    }

    //3.3绘制曲线
    for(int i=0;i<5;i++) {
        if(i == 0){
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
        } else {
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
        }
        painter.drawLine(pointX[i],pointY[i],pointX[i+1],pointY[i+1]);
    }
}

void MainWindow::onReplied(QNetworkReply *reply)
{
    int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();


    if(reply->error() != QNetworkReply::NoError || status_code != 200) {
        qDebug() << reply->errorString().toLatin1().data();
        QMessageBox::warning(this,"天气","请求数据失败",QMessageBox::Ok);
    } else {
        QByteArray byteArray = reply->readAll();
        parseJson(byteArray);
    }
    reply->deleteLater();
}

QVariantMap MainWindow::parseLocationData(const QByteArray &data)
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

void MainWindow::showLocation(const QVariantMap &varMap)
{
    const QString& addrStr=varMap.value("address").toString();
    QString address = addrStr.split("省").at(1);
    qDebug() << address.left(address.length()-1);
    getWeatherInfo(address.left(address.length()-1));
}

void MainWindow::on_btnSearch_clicked()
{
    QString cityName =ui->leCity->text();
    getWeatherInfo(cityName);
}

void MainWindow::onHostLookedUp(const QHostInfo &host)
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

void MainWindow::queryLocationOfIP(const QString &strIp)
{
    const QString& strUrl=QString("http://api.map.baidu.com/location/ip?ak=%1&ip=%2&coor=bd09ll").
                arg(AK_STR).arg(strIp);

        QNetworkRequest request;
        request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
        request.setUrl(QUrl(strUrl));

        QNetworkReply* reply =m_pNet->get(request);
        connect(reply,SIGNAL(finished()),this,SLOT(replyFinished()));
}

void MainWindow::replyFinished()
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

