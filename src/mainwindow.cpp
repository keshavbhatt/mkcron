#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopServices>
#include <QUrl>
#include <QSplitter>
#include <requestinterceptor.h>


MainWindow::MainWindow(QWidget *parent,QString serverBase) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    home = serverBase;

    this->setWindowIcon(QIcon(":/icons/icon-64.png"));
    this->setWindowTitle(qApp->applicationName()+" v"+qApp->applicationVersion());

    ui->webView->setStyleSheet("background-color:#181818;");

    connect(this,&MainWindow::urlChanged,[=](const QUrl url){
       managerWidget->urlChanged(url.toString());
    });

    // so we can get readble on first page load
    connect(ui->webView,&QWebEngineView::loadFinished,[=](const bool loaded){
        if(loaded){
           ui->webView->page()->runJavaScript("simulateMouseClick(document.querySelector('.cron-parts div').firstElementChild)");
           emit managerWidget->getHumanReadble();
           emit managerWidget->getDefinition();
        }
    });

    // we send url to manager on very definition change
    connect(ui->webView,&QWebEngineView::urlChanged,[=](QUrl url){
        emit urlChanged(url);
    });

    QWebEngineProfile *weProfile = new QWebEngineProfile(QApplication::applicationName(),this);

    weProfile->setHttpUserAgent("Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:72.0) Gecko/20100101 Firefox/72.0");
    weProfile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);

    WebEnginePage *webenginepage = new WebEnginePage(weProfile, this);
    webenginepage->setBackgroundColor(QColor("#181818"));


    RequestInterceptor *interceptor = new RequestInterceptor(weProfile);
    weProfile->setUrlRequestInterceptor(interceptor);

    webenginepage->settings()->setAttribute(QWebEngineSettings::ShowScrollBars,false);

    //page should be set parent of profile to prevent
    //Release of profile requested but WebEnginePage still not deleted. Expect troubles !
    weProfile->setParent(webenginepage);

    ui->webView->setPage(webenginepage);
    setStyle(":/dark.qss");

    ui->webView->setMinimumSize(400,500);

    managerWidget = new Manager(this);
    managerWidget->setMinimumSize(350,managerWidget->minimumSizeHint().height());
    managerWidget->setStyleSheet("QWidget{background-image:url(:/cron/texture.png)}");

    connect(managerWidget,&Manager::getDefinition,[=](){
        definition = "";
        ui->webView->page()->runJavaScript(
            "$('#input').attr('value');",
            [this](const QVariant &result){
                definition = result.toString().trimmed();
                if(definition.trimmed().isEmpty()==false)
                managerWidget->updateDefinition("#"+definition);
            }
        );
    });

    // manager request human readable values on every definition update/url change
    connect(managerWidget,&Manager::getHumanReadble,[=](){
        humanReadable = "";
        ui->webView->page()->runJavaScript(
            "$('.human-readable').text().trim();",
            [this](const QVariant &result){
                humanReadable = result.toString().trimmed();
                if(humanReadable.trimmed().isEmpty()==false)
                managerWidget->updateTitle(humanReadable);
            }
        );

    });

    //    "var hmr = document.querySelector('.human-readable');"
    //    "if(hmr != null) hmr.textContent.trim()"

    connect(managerWidget,&Manager::copied,[=](){
       ui->webView->page()->runJavaScript("definitionCopied()");
    });


    QSplitter *split1 = new QSplitter;
    split1->setObjectName("split1");
    split1->setOrientation(Qt::Horizontal);
    split1->addWidget(ui->webViewWidget);
    split1->addWidget(managerWidget);
    split1->setStretchFactor(0,2);
    split1->setStretchFactor(1,2);
    ui->centralWidget->layout()->addWidget(split1);

    if(settings.value("geometry").isValid()){
        restoreGeometry(settings.value("geometry").toByteArray());
        restoreState(settings.value("windowState").toByteArray());
    }

    ui->webView->load(serverBase+"/#5_4_*_*_sun");
}

void MainWindow::loadUrl(QString urlStr)
{
    ui->webView->load(urlStr);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings.setValue("geometry",saveGeometry());
    settings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}

void MainWindow::setStyle(QString fname)
{
    QFile styleSheet(fname);
    if (!styleSheet.open(QIODevice::ReadOnly)) {
        qWarning("Unable to open file");
    return; }
    qApp->setStyleSheet(styleSheet.readAll());
    styleSheet.close();
}

MainWindow::~MainWindow()
{
    delete ui;
}
