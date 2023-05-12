#include "MyPlainTextEdit.h"
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>

MyPlainTextEdit::MyPlainTextEdit(QWidget *parent) : QPlainTextEdit(parent) {}

void MyPlainTextEdit::openFile() {
    QString filename = QFileDialog::getOpenFileName(this, "Open File:", "/home");
    QFile file(filename);
    QTextStream in(&file);
    this->document()->setPlainText(in.readAll());
}