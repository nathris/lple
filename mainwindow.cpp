#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    p_model = new QStandardItemModel(0,6);
    setHeaders();

    //Context menu for add button
    QMenu* addMenu = new QMenu(this);
    QAction* newEntryAction = new QAction("New Entry",this);
    QAction* addFileAction = new QAction("Existing Rom(s)",this);
    addMenu->addAction(newEntryAction);
    addMenu->addAction(addFileAction);
    ui->addButton->setMenu(addMenu);

    //Default entry
    QList<QStandardItem*> entry;
    entry   << new QStandardItem(QString("<rom path>"))
            << new QStandardItem(QString("<rom name>"))
            << new QStandardItem(QString("DETECT"))
            << new QStandardItem(QString("DETECT"))
            << new QStandardItem(QString("1|crc"))
            << new QStandardItem(ui->playlistText->text());

    p_model->appendRow(entry);


    ui->tableView->setModel(p_model);


    connect(ui->openButton, SIGNAL(released()), this, SLOT(loadFile()));
    connect(ui->saveButton, SIGNAL(released()), this, SLOT(saveFile()));
    connect(ui->newButton, SIGNAL(released()), this, SLOT(newFile()));

    //add/remove buttons
    connect(newEntryAction, SIGNAL(triggered()), this, SLOT(addRow()));
    connect(addFileAction, SIGNAL(triggered()), this, SLOT(addExistingRom()));
    connect(ui->removeButton, SIGNAL(released()), this, SLOT(deleteRow()));

    connect(ui->uniformCheck, SIGNAL(released()), this, SLOT(makeUniform()));
    connect(ui->playlistText, SIGNAL(editingFinished()), this, SLOT(makeUniform()));


    currentDir = QDir::homePath();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setHeaders()
{
    p_model->setHorizontalHeaderItem(0,new QStandardItem(QString("Rom Path")));
    p_model->setHorizontalHeaderItem(1,new QStandardItem(QString("Rom Name")));
    p_model->setHorizontalHeaderItem(2,new QStandardItem(QString("Core Path")));
    p_model->setHorizontalHeaderItem(3,new QStandardItem(QString("Core Name")));
    p_model->setHorizontalHeaderItem(4,new QStandardItem(QString("CRC")));
    p_model->setHorizontalHeaderItem(5,new QStandardItem(QString("Playlist")));
}

void MainWindow::loadFile()
{
    QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Open Playlist"),
                currentDir,
                tr("Playlist files (*.lpl);;All files (*.*)") );

    currentDir = QFileInfo(filename).path();
    //Update the playlist text box
    ui->playlistText->setText(QFileInfo(filename).fileName());

    QFile file(filename);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        p_model->clear(); //clear existing entries
        setHeaders();
        while(!in.atEnd()) {
            QList<QStandardItem*> entry;
            for(int i = 0; i < 6; i++) {
                if(in.atEnd()) {
                    break;
                }
                entry << new QStandardItem(in.readLine());
            }
            p_model->appendRow(entry);
        }
    }
    file.close();

    //first column's width gets messed up after clearing model data, have to reset it
    ui->tableView->setColumnWidth(0, ui->centralWidget->width()/6);
}

void MainWindow::newFile()
{
    QMessageBox msg;
    msg.setText("          Clear all entries?          ");
    msg.setInformativeText("<p>All unsaved changes will be lost</p>");
    msg.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    msg.setDefaultButton(QMessageBox::No);
    int ret = msg.exec();

    switch(ret){
    case QMessageBox::No:
        return;
    case QMessageBox::Yes:
        p_model->clear(); //clear existing entries
        setHeaders();
        ui->playlistText->setText(QString("New Playlist.lpl"));
        QList<QStandardItem*> entry;
        entry   << new QStandardItem(QString("<rom path>"))
                << new QStandardItem(QString("<rom name>"))
                << new QStandardItem(QString("DETECT"))
                << new QStandardItem(QString("DETECT"))
                << new QStandardItem(QString("1|crc"))
                << new QStandardItem(ui->playlistText->text());
        p_model->appendRow(entry);

        //first column's width gets messed up after clearing model data, have to reset it
        ui->tableView->setColumnWidth(0, ui->centralWidget->width()/6);
        break;
    }
}

void MainWindow::saveFile()
{
    QString filename = QFileDialog::getSaveFileName(
                this,
                tr("Save Playlist"),
                currentDir+"/"+ui->playlistText->text(),
                tr("Playlist files (*.lpl);;All files (*.*)") );
    currentDir = QFileInfo(filename).path();

    //iterate over all rows/columns, write contents of each cell to new line
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        for(int i = 0; i < p_model->rowCount(); i++) {
            for(int j = 0; j < p_model->columnCount(); j++) {
                stream << p_model->index(i,j).data().toString() << endl;
            }
        }
    }
    file.close();

}

void MainWindow::addRow()
{
    QList<QStandardItem*> entry;
    entry << new QStandardItem(QString("<rom path>"))
          << new QStandardItem(QString("<rom name>"))
          << new QStandardItem(QString("DETECT"))
          << new QStandardItem(QString("DETECT"))
          << new QStandardItem(QString("1|crc"))
          << new QStandardItem(ui->playlistText->text());
    p_model->appendRow(entry);

    ui->tableView->scrollToBottom();
}

void MainWindow::addExistingRom()
{
    QStringList filenames = QFileDialog::getOpenFileNames(
                this,
                tr("Open Playlist"),
                currentDir,
                tr("All files (*.*)") );

    currentDir = QFileInfo(filenames.first()).path();

    for(int i = 0; i < filenames.length(); i++) {
        QList<QStandardItem*> entry;
        entry << new QStandardItem(filenames.at(i))
              << new QStandardItem(QFileInfo(filenames.at(i)).baseName())
              << new QStandardItem(QString("DETECT"))
              << new QStandardItem(QString("DETECT"))
              << new QStandardItem(QString("1|crc"))
              << new QStandardItem(ui->playlistText->text());

        p_model->appendRow(entry);
    }
}

void MainWindow::deleteRow()
{
    QMessageBox msg;
    msg.setText("               Delete row?               ");
    msg.setInformativeText("This will delete the currently selected row.");
    msg.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    msg.setDefaultButton(QMessageBox::No);

    int ret = msg.exec();

    switch(ret){
    case QMessageBox::No:
        return;
    case QMessageBox::Yes:
        ui->tableView->model()->removeRow(ui->tableView->selectionModel()->currentIndex().row());
        break;
    }
}

//Sets all entries in the playlist column to the same value
void MainWindow::makeUniform()
{
    if(ui->uniformCheck->isChecked()){
        for(int i = 0; i < p_model->rowCount(); i++) {
            p_model->setItem(i, 5, new QStandardItem(ui->playlistText->text()));
        }
    }
}
