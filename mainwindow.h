#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListWidget>
#include <QMainWindow>
#include "adddeckform.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void deck_add();

private slots:
    void on_listWidget_customContextMenuRequested(const QPoint &pos);
    void remove_deck();
    void on_pushButton_4_clicked();

    void DatabaseConnect();
    void DatabaseInit();
    void DatabasePopulate();

    void ListWidgetPopulate();

    void on_pushButton_clicked();

    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_pushButton_3_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
