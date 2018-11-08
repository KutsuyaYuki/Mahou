#include "adddeckform.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QInputDialog>
#include <QMenu>
#include <QDebug>

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

// Current ID of selected deck in database
QString static id;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->listWidget, SIGNAL(customContextMenuRequested(const QPoint &)),
            ui->listWidget, SLOT(ShowContextMenu(const QPoint &)));

    // -- DATABASE INIT --
    DatabaseConnect();
    DatabaseInit();
    DatabasePopulate();

    ListWidgetPopulate();
}

void MainWindow::ListWidgetPopulate()
{
    QSqlQuery * query = new QSqlQuery(qApp->applicationDirPath()
                                      + QDir::separator()
                                      + "db.sqlite");
    query->setForwardOnly(true);
    query->exec("SELECT name FROM decks ORDER BY name");
    while(query->next())
    {
        ui->listWidget->addItem(query->value(0).toString());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_listWidget_customContextMenuRequested(const QPoint &pos)
{
    // Make the context menu
    QMenu *contextMenu = new QMenu(tr("Context menu"), this);

    // Make action
    QAction remove_deck("Remove deck", this);
    // SLOT = Function that will be triggered?
    connect(&remove_deck, SIGNAL(triggered()), this, SLOT(remove_deck()));
    contextMenu->addAction(&remove_deck);

    contextMenu->exec(mapToGlobal(pos));
}


void MainWindow::remove_deck()
{
    // If multiple selection is on, we need to erase all selected items
    qDebug() << ui->listWidget->currentRow();
    // Remove from database
    QSqlQuery query_deck;
    QSqlQuery query_cards;

    QModelIndex index = ui->listWidget->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();

    query_deck.prepare("DELETE FROM decks WHERE name = ?");
    query_deck.addBindValue(itemText);
    query_deck.exec();

    query_cards.prepare("DELETE FROM cards WHERE id_deck = ?");
    query_cards.addBindValue(id);
    query_cards.exec();

       for (int i = 0; i < ui->listWidget->selectedItems().size(); ++i) {
           // Get curent item on selected row
           QListWidgetItem *item = ui->listWidget->takeItem(ui->listWidget->currentRow());
           // And remove it
           delete item;

    }
}

void MainWindow::on_pushButton_4_clicked()
{
    bool ok;
        // Parent, Title, Label, Echo, Text, Bool (if pressed ok).
        QString text = QInputDialog::getText(this, tr("New deck name"),
                                             tr("Deck name:"), QLineEdit::Normal,
                                             "", &ok);
        // Ok is pressed, add item to listview.
        if (ok && !text.isEmpty()) {
            QSqlQuery query;

            qDebug() << "Removing " + text;
            query.prepare("INSERT INTO decks(name) VALUES(?)");
            query.addBindValue(text);
            query.exec();
            ui->listWidget->addItem(text);
            ui->listWidget->setCurrentRow(ui->listWidget->count() - 1 ); // size - 1 = last item
        }
}

void MainWindow::DatabaseConnect()
{
    const QString DRIVER("QSQLITE");

    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(DRIVER);

        //db.setDatabaseName(":memory:");
        db.setDatabaseName(qApp->applicationDirPath()
                            + QDir::separator()
                            + "db.sqlite" );

        if(!db.open())
            qWarning() << "MainWindow::DatabaseConnect - ERROR: " << db.lastError().text();
    }
    else{
        qWarning() << "MainWindow::DatabaseConnect - ERROR: no driver " << DRIVER << " available";
    }
}

void MainWindow::DatabaseInit()
{
    QSqlQuery query;
    query.exec("CREATE TABLE decks (id INTEGER PRIMARY KEY, name TEXT, losses TEXT, wins TEXT)");
    query.exec("CREATE TABLE stats (id INTEGER PRIMARY KEY, losses TEXT, wins TEXT)");
    query.exec("CREATE TABLE cards (id INTEGER PRIMARY KEY, id_deck TEXT, name TEXT, wins TEXT, FOREIGN KEY (id_deck) REFERENCES decks(id))");
}

void MainWindow::DatabasePopulate()
{
    QSqlQuery query;

    query.prepare("SELECT * FROM decks");
    query.exec();

    int i = 0;
    while (query.next()) {
        i++;
        qDebug() << query.value(0).toString() + " - " + query.value(1).toString();
    }
}

void MainWindow::on_pushButton_clicked()
{
    QSqlQuery query;
    query.prepare("SELECT name FROM cards WHERE id_deck = ?");
    query.addBindValue(id);
    query.exec();
    //query.exec("SELECT cards.name FROM cards LEFT JOIN decks ON decks.id = cards.id_deck");
    while (query.next()) {
    qDebug() << query.value(0).toString();
    }
}

void MainWindow::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QSqlQuery query;

    QModelIndex index = ui->listWidget->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();

    query.prepare("SELECT id FROM decks WHERE name = ?");
    query.addBindValue(itemText);
    query.exec();
    while (query.next()) {
        id = query.value(0).toString();
    }
    query.finish();
    qDebug() << ui->listWidget->row(ui->listWidget->currentItem());

    query.prepare("SELECT wins, name FROM cards WHERE id_deck = ?");
    query.addBindValue(id);
    query.exec();
    //query.exec("SELECT cards.name FROM cards LEFT JOIN decks ON decks.id = cards.id_deck");
    ui->treeWidget_Cards->clear();
    while (query.next()) {
        QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->treeWidget_Cards);

        // QTreeWidgetItem::setText(int column, const QString & text)
        treeItem->setText(0, query.value(0).toString());
        treeItem->setText(1, query.value(1).toString());

//        ui->treeWidget_Cards->addi(query.value(0).toString());
    qDebug() << query.value(0).toString();
    }

    // If cards exists, look up the wins and loses everytime a different card gets clicked.
    // Put the card with the most wins at top.

}

void MainWindow::on_pushButton_3_clicked()
{
    bool ok;
        // Parent, Title, Label, Echo, Text, Bool (if pressed ok).
        QString text = QInputDialog::getText(this, tr("Winning card"),
                                             tr("Card name:"), QLineEdit::Normal,
                                             "", &ok);
        // Ok is pressed, add item to listview.
        if (ok && !text.isEmpty()) {
            QSqlQuery query;

            QModelIndex index = ui->listWidget->currentIndex();
            QString itemText = index.data(Qt::DisplayRole).toString();

            /*query.prepare("SELECT id FROM decks WHERE name = ?");
            query.addBindValue(itemText);
            query.exec();
            QString id;
            while (query.next()) {
                id = query.value(0).toString();
            }
            query.finish();*/

            QSqlQuery query2;
            query2.prepare("INSERT INTO cards(id_deck, name) VALUES(?, ?)");
            query2.addBindValue(id);
            query2.addBindValue(text);
            if (query2.exec())
                qDebug() << "Done OK";
            else
                qDebug() << "Huh!";
            query2.finish();
//            ui->treeWidget_Cards->addItem(text);
        }
}
