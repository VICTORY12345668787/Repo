#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QMouseEvent>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageBox>
#include "weatherdata.h"

class QHostInfo;
class QNetworkAccessManager;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

   protected:
    void contextMenuEvent(QContextMenuEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    //获取天气数据
    void getWeatherInfo(QString cityName);

    //解析数据
    void parseJson(QByteArray& byteArray);

    //更新UI
    void updateUI();

    //重写父类的eventfilter方法
    bool eventFilter(QObject* watched,QEvent* event);

    //绘制高低温曲线
    void paintHighCurve();
    void paintLowCurve();

private slots:
    void on_btnSearch_clicked();
    void onHostLookedUp(const QHostInfo &host);
    void queryLocationOfIP(const QString& strIp);
    void replyFinished();

private:
    void onReplied(QNetworkReply* reply);

    QVariantMap parseLocationData(const QByteArray& data);
    void showLocation(const QVariantMap& varMap);


   private:
    Ui::MainWindow* ui;

    QMenu* mExitMenu;   // 右键退出的菜单
    QAction* mExitAct;  // 退出的行为
    QPoint mOffset;     // 窗口移动时, 鼠标与窗口左上角的偏移

    //Http请求
    QNetworkAccessManager* mNetAccessManager;

    //今天和6天的天气
    Today mToday;
    Day mDay[6];

    //星期和日期
    QList<QLabel*> mWeekList;
    QList<QLabel*> mDateList;

    //天气和天气图标
    QList<QLabel*> mTypeList;
    QList<QLabel*> mTypeIconList;

    //天气污染指数
    QList<QLabel*> mAqiList;

    //风力和风向
    QList<QLabel*> mFxList;
    QList<QLabel*> mFlList;

    QMap<QString,QString> mTypeMap;

    QNetworkAccessManager* m_pNet;
};
#endif  // MAINWINDOW_H
