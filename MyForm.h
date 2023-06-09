#pragma once

#include "ui_Form.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include "MyPlainTextEdit.h"
#include <map>
#include <QMessageBox>

class MyForm : public QMainWindow {
    Q_OBJECT

public:
    MyForm(QWidget *parent = 0);
    std::map<QString, QString> languages;

private:
    Ui::MyForm ui;
    QFileSystemWatcher *watcher;
    QString guessLang();

public slots:
    void showHelp();
    void newFile();
    void openFile();
    void saveFile();
    void closeFile();
    void closeFile(int index);
    void onFileChanged(const QString &path);
    void onCursorChanged();
    int existsFile(const QString &path);
    void showFileSize();
    void showLanguage();
    void setModified();
    void setUnmodified();

private slots:
    QWidget *getCurrentTab();
    MyPlainTextEdit *getCurrentText();
    MyPlainTextEdit *getTextByIndex(int index);
    MyPlainTextEdit *getTextByPath(const QString &path);

signals:
    void line(const QString &l);
    void column(const QString &c);
    void fileSize(const QString &s);
    void lang(const QString &g);
    void tabChanged();
};
