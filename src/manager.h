#ifndef MANAGER_H
#define MANAGER_H

#include <QWidget>
#include <QClipboard>
#include "ui_saveditemui.h"
#include "ui_details.h"

namespace Ui {
class Manager;
}

class Manager : public QWidget
{
    Q_OBJECT

signals:
    void copied();
    void getHumanReadble();
    void getDefinition();

public:
    explicit Manager(QWidget *parent = nullptr);
    ~Manager();

public slots:
    void add_to_table(QString id, QStringList itemData, bool saving);

    void urlChanged(QString urlStr);

    void updateTitle(QString title);

    void updateDefinition(QString urlStr);
private slots:

    void on_saveDefinition_clicked();

    void on_clearApplication_clicked();

    void on_clearCommand_clicked();

    void on_copyDefinition_clicked();

    void on_application_textChanged(const QString &arg1);

    void on_command_textChanged(const QString &arg1);

    void on_definition_textChanged(const QString &arg1);

    QString getApplicationName();

    void saveItem(const QStringList itemData);

    void on_title_textChanged(const QString &arg1);

    void loadSavedItems();
    void showItemDetail(QVariant data);
    void removeItem(QString objectName);
    bool deleteSavedFile(QString fileId);
private:
    Ui::Manager *ui;
    QClipboard *clipboard = nullptr;
    QString lastDefinition;
    Ui::savedItem savedItemUi;
    Ui::detailsUiForm detailsUi;
};

#endif // MANAGER_H
