#ifndef REQUESTINTERCEPTOR_H
#define REQUESTINTERCEPTOR_H
#include <QWebEnginePage>
#include <QDebug>
#include <QObject>
#include <QWebEngineView>
#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QWebEngineUrlRequestInfo>
#include <QWebEngineUrlRequestInterceptor>

class RequestInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    QSettings settings;
    RequestInterceptor(QObject *parent = nullptr) : QWebEngineUrlRequestInterceptor(parent)
    {
        adFile.setFileName(":/js/ads.txt");
        if (!adFile.open(QIODevice::ReadOnly | QIODevice::Text))
                  return;

          while (!adFile.atEnd()) {
              QByteArray line = adFile.readLine();
              adsUrl.append(line.trimmed());
          }
    }

    void interceptRequest(QWebEngineUrlRequestInfo &info)
    {
        //enable adblocker if settings set true
        if(settings.value("adblocker",true).toBool())
        {
            //add log_event to blacklist urls
            if(settings.value("eventlogger",true).toBool())
            {
                if(!adsUrl.contains("log_event?",Qt::CaseInsensitive))
                adsUrl.append("log_event?");
            }else
            {
                int index = adsUrl.indexOf("log_event?",0);
                if(index!=-1)
                {
                    adsUrl.removeAt(index);
                }
            }

            //add comment_service_ajax? to blacklist urls
            if(settings.value("comments",true).toBool())
            {
                if(!adsUrl.contains("comment_service_ajax?",Qt::CaseInsensitive))
                adsUrl.append("comment_service_ajax?");
            }else
            {
                int index = adsUrl.indexOf("comment_service_ajax?",0);
                if(index!=-1)
                {
                    adsUrl.removeAt(index);
                }
            }


            bool shouldBlock = false;
            foreach (QString adUrl, adsUrl)
            {
                if(info.requestUrl().toString().contains(adUrl,Qt::CaseInsensitive))
                {
                    qDebug()<<"BLOCKED DUE TO:"<<adUrl;
                    shouldBlock = true;
                    break;
                }
            }

            if (shouldBlock)
            {
                qDebug() << "blocking" << info.requestUrl();
                info.block(true);
            }
        }
    }
private:
    QFile adFile;
    QStringList adsUrl;

};


class WebEnginePage : public QWebEnginePage
{
    Q_OBJECT
public:
    explicit WebEnginePage(QWebEngineProfile *profile, QObject *parent = 0):QWebEnginePage(profile,parent)
    {

    }

    ~WebEnginePage(){}

protected:
//    QWebEnginePage *createWindow(WebWindowType type) Q_DECL_OVERRIDE
//    {
//        if(type==QWebEnginePage::WebBrowserWindow)
//        {
//            QWebEngineView *engine = new QWebEngineView(0);
//            engine->setAttribute(Qt::WA_DeleteOnClose);
//            engine->setWindowFlag(Qt::Dialog);
//            QWebEnginePage *page = new QWebEnginePage(this->profile(),engine);
//            engine->setPage(page);
//            engine->resize(500,500);
//            engine->show();
//            connect(page,&WebEnginePage::windowCloseRequested,[=](){
//                qDebug()<<"PAGE CLOSE REQUESTED";
//                engine->close();
//            });
//            connect(engine,&QWebEngineView::titleChanged,[=](QString title)
//            {
//               engine->setWindowTitle(QApplication::applicationName()+" v"+QApplication::applicationVersion()
//                                    +" | "+title);
//            });
//            return engine->page();
//        }else{

//        }
//    }
};
#endif // REQUESTINTERCEPTOR_H
