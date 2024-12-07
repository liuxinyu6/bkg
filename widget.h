#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QString>
#include <thread>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void sclicked();
    void onButtonStart();
    void onButtonTimeOut();

private:
    Ui::Widget *ui;
    QTimer* timer;
    QString videoPathafter;
    QString videoPath;
    bool beStart;   //是否启用设定
    bool beChanged; //是否启动线程
    bool beOffsetDir;   //是否改变目录
    std::thread f_;
    static const std::string pic[];
    static const std::string vdo[];
    static const std::string exe[];
    static int timeval;
    static int timevalpic;

};

#endif // WIDGET_H
