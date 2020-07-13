#include "manager.h"
#include "utils.h"
#include "ui_manager.h"
#include <QClipboard>
#include <QTimer>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

#include <QMessageBox>
#include <QFileDialog>

inline void saveJson(QJsonDocument document, QString fileName)
{
    QFile jsonFile(fileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(document.toJson());
}

QJsonDocument loadJson(QString fileName)
{
    QFile jsonFile(fileName);
    jsonFile.open(QFile::ReadOnly);
    return QJsonDocument().fromJson(jsonFile.readAll());
}

Manager::Manager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Manager)
{
    ui->setupUi(this);

    clipboard = QApplication::clipboard();

    ui->clearApplication->setEnabled(!ui->application->text().trimmed().isEmpty());
    ui->clearCommand->setEnabled(!ui->command->text().trimmed().isEmpty());
    ui->copyDefinition->setEnabled(!ui->definition->text().trimmed().isEmpty());
    ui->saveDefinition->setEnabled(!ui->definition->text().trimmed().isEmpty());
    loadSavedItems();
}

void Manager::updateDefinition(QString urlStr)
{
    QString application = ui->application->text().trimmed();
    QString command     = ui->command->text().trimmed();

    //set to definition
    if(urlStr.trimmed().isEmpty()==false){
        if(urlStr.contains("#")){
            QString definition  = urlStr.split("/#").last();
            definition.replace("_"," ");
            lastDefinition = definition.remove("#");
            ui->definition->setText(definition+
                                    " "+
                                    application+
                                    " "+
                                    command);
        }else{
            QTimer::singleShot(1000,this,[=](){
                emit getDefinition();
            });
        }
    }else if(lastDefinition.trimmed().isEmpty()==false)
    {
        ui->definition->setText(lastDefinition+
                                " "+
                                application+
                                " "+
                                command);
    }

    QTimer::singleShot(300,this,[=](){
        emit getHumanReadble();
    });

    bool valid = false;
    if(ui->application->text().trimmed().isEmpty()==false){
        valid = true;
    }

    if(ui->command->text().trimmed().isEmpty()==false && ui->application->text().trimmed().isEmpty()==true)
    {
        valid = false;
    }
    ui->copyDefinition->setEnabled(valid);
    ui->saveDefinition->setEnabled(valid);
}

void Manager::updateTitle(QString title)
{
    QString application = getApplicationName();
    QString command     = ui->command->text().trimmed();

    QString titleStr    =  "Run ("+application+" "+command+") "+title;
    ui->title->setText(titleStr);
}


// Get name of application binary
QString Manager::getApplicationName()
{
    QString app = ui->application->text().trimmed();
    if(app.contains("/")){
        app = app.split("/").last();
    }
    return app;
}


void Manager::urlChanged(QString urlStr)
{
    updateDefinition(urlStr);
}

void Manager::add_to_table(QString id,QStringList itemData,bool saving)
{
    QWidget *track_widget = new QWidget(ui->saved);
    track_widget->setObjectName("track-widget-"+id);
    savedItemUi.setupUi(track_widget);
    track_widget->setStyleSheet("QWidget#"+track_widget->objectName()+"{background-color: transparent;}");

    QListWidgetItem* item;
    if(saving){
        QStringList itemData;                        //count 4
        itemData<<ui->application->text().trimmed(); //application 0
        itemData<<ui->command->text().trimmed();     //command 1
        itemData<<ui->definition->text().trimmed();  //definition 2
        itemData<<ui->title->text().trimmed();       //title 3
        saveItem(itemData);
        track_widget->setProperty("itemData",itemData);
        track_widget->setProperty("id",id);
        track_widget->setToolTip(ui->definition->text().trimmed());
        savedItemUi.title->setText(ui->title->text().trimmed());
    }else{
        track_widget->setProperty("itemData",itemData);
        track_widget->setProperty("id",id);
        track_widget->setToolTip(itemData.at(2));
        savedItemUi.title->setText(itemData.at(3));
    }
    item = new QListWidgetItem(ui->saved);
    connect(savedItemUi.load,&QPushButton::clicked,[=](){
        showItemDetail(track_widget->property("itemData"));
    });
    connect(savedItemUi.remove,&QPushButton::clicked,[=](){
        removeItem(track_widget->objectName());
    });

    item->setSizeHint(track_widget->minimumSizeHint());
    ui->saved->setItemWidget(item,track_widget);
    ui->saved->addItem(item);
}

void Manager::removeItem(QString objectName)
{
    QMessageBox msgBox;
    msgBox.setText("Remove item from bookmarks ?");
    msgBox.setIconPixmap(QPixmap(":/icons/others/error-warning-line.png").scaled(42,42,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    msgBox.setInformativeText("Choosing \"Yes\" will remove item from bookmarks.");
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes );
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Cancel){
        return;
    }else{
        auto *itemWidget = ui->saved->findChild<QWidget*>(objectName);
        if(itemWidget==nullptr)
            return;
        if(deleteSavedFile(itemWidget->property("id").toString())){
            QPoint pos = itemWidget->pos();
            int index = ui->saved->row(ui->saved->itemAt(pos));
            delete ui->saved->takeItem(index);
        }
    }
}

bool Manager::deleteSavedFile(QString fileId)
{
    QFile file(utils::returnPath("saved")+fileId+".json");
    return file.remove();
}

void Manager::showItemDetail(QVariant data)
{
    QStringList dataList = data.toStringList();
    QWidget *detailsWidget = new QWidget(this);
    detailsUi.setupUi(detailsWidget);
    detailsWidget->setAttribute(Qt::WA_DeleteOnClose);
    detailsWidget->setWindowFlags(Qt::Window);
    detailsWidget->setWindowModality(Qt::ApplicationModal);
    detailsWidget->setWindowTitle(QApplication::applicationName()+" | "+dataList.at(3));
    detailsUi.application->setText(dataList.at(0));
    detailsUi.command->setText(dataList.at(1));
    detailsUi.definition->setText(dataList.at(2));
    detailsUi.title->setText(dataList.at(3));
    connect(detailsUi.copyDefinition,&QPushButton::clicked,[=](){
        detailsUi.definition->selectAll();
        clipboard->setText(detailsUi.definition->text().trimmed());
        emit copied();
    });
    detailsWidget->show();
}

void Manager::saveItem(const QStringList itemData)
{
    if(itemData.count() == 4)
    {
        QJsonObject recordObject;
        recordObject.insert("application",itemData.at(0));
        recordObject.insert("command",itemData.at(1));
        recordObject.insert("definition",itemData.at(2));
        recordObject.insert("title",itemData.at(3));
        QJsonDocument doc(recordObject);
        saveJson(doc,utils::returnPath("saved")+utils::generateRandomId(20)+".json");
    }
}

void Manager::loadSavedItems()
{
    QDir savedDir(utils::returnPath("saved"));
    savedDir.setFilter(QDir::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot);
    savedDir.setNameFilters(QStringList()<<"*.json");
    foreach(QFileInfo info, savedDir.entryInfoList()){
            QJsonDocument doc = loadJson(info.filePath());
            QJsonObject   recordObj = doc.object();
            QStringList itemData;
            foreach(const QString& key, recordObj.keys()) {
                itemData.append(recordObj.value(key).toString());
            }
            add_to_table(info.baseName(),itemData,false);
    }
}

Manager::~Manager()
{
    delete ui;
}

void Manager::on_saveDefinition_clicked()
{
    add_to_table(utils::generateRandomId(14),QStringList(),true);
}

void Manager::on_clearApplication_clicked()
{
    ui->application->clear();
    ui->application->setFocus();
}

void Manager::on_clearCommand_clicked()
{
    ui->command->clear();
    ui->command->setFocus();
}

void Manager::on_copyDefinition_clicked()
{
    if(ui->definition->text().trimmed().isEmpty()==false){
        ui->definition->selectAll();
        clipboard->setText(ui->definition->text().trimmed());
        emit copied();
    }
}

void Manager::on_command_textChanged(const QString &arg1)
{
    ui->clearCommand->setEnabled(!arg1.trimmed().isEmpty());
    updateDefinition(lastDefinition);
}

void Manager::on_application_textChanged(const QString &arg1)
{
    ui->clearApplication->setEnabled(!arg1.trimmed().isEmpty());
    updateDefinition(lastDefinition);
}

void Manager::on_definition_textChanged(const QString &arg1)
{
    ui->copyDefinition->setEnabled(!arg1.trimmed().isEmpty());
    ui->saveDefinition->setEnabled(!arg1.trimmed().isEmpty());
}

void Manager::on_title_textChanged(const QString &arg1)
{
    ui->copyDefinition->setEnabled(!arg1.trimmed().isEmpty());
    ui->saveDefinition->setEnabled(!arg1.trimmed().isEmpty());
}

void Manager::on_selectApplication_clicked()
{
    QFileDialog file_dialog(this, tr("Select Executable"));
    file_dialog.setFilter(QDir::Executable | QDir::Files);
    QString file = file_dialog.getOpenFileName(this, tr("Select Exectuable"));
    ui->application->setText(file);
}

void Manager::on_removeAll_clicked()
{
    if(ui->saved->count()<1)
        return;
    QMessageBox msgBox;
    msgBox.setText("Remove all items from bookmark ?");
    msgBox.setIconPixmap(QPixmap(":/icons/others/error-warning-line.png").scaled(42,42,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    msgBox.setInformativeText("Choosing \"Yes\" will remove all item from bookmark.");
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes );
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Cancel){
        return;
    }else{
        QDir saved(utils::returnPath("saved"));
        saved.removeRecursively();
        ui->saved->clear();
    }
}
