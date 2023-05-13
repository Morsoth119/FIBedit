#pragma once

#include <QPlainTextEdit>

class MyPlainTextEdit : public QPlainTextEdit {
    Q_OBJECT

public:
        MyPlainTextEdit(QWidget *parent);
        QFile *currentFile;

public slots:
    void openFile();
    void saveFile();
    void newFile();

signals:
};
