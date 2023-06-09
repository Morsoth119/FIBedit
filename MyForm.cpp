#include "MyForm.h"

MyForm::MyForm(QWidget *parent) : QMainWindow(parent) {
    ui.setupUi(this);
    watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(onFileChanged(QString)));
    connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SIGNAL(tabChanged()));
    connect(this, SIGNAL(tabChanged()), this, SLOT(onCursorChanged()));
    connect(this, SIGNAL(tabChanged()), this, SLOT(showFileSize()));
    connect(this, SIGNAL(tabChanged()), this, SLOT(showLanguage()));
        
    languages = {
        {"cpp", "C++"},
        {"hpp", "C++"},
        {"cc", "C++"},
        {"hh", "C++"},
        {"c", "C"},
        {"h", "C"},
        {"txt", "Plain Text"},
        {"py", "Python"},
        {"java", "Java"},
        {"html", "HTML"},
        {"css", "CSS"},
        {"js", "JavaScript"},
        {"xml", "XML"},
        {"json", "JSON"},
        {"sql", "SQL"},
        {"md", "Markdown"},
        {"sh", "Shell"},
        {"bash", "Bash"},
        {"el", "Emacs Lisp"},
        {"lua", "Lua"},
        {"scm", "Scheme"},
        {"lisp", "Common Lisp"},
        {"php", "PHP"},
        {"cs", "C#"},
        {"go", "Go"},
        {"kt", "Kotlin"},
        {"swift", "Swift"},
        {"rs", "Rust"},
        {"rb", "Ruby"},
        {"ts", "TypeScript"},
        {"org", "Org"},
        {"yml", "Yaml"},
    };
}

QString MyForm::guessLang() {
    QString::const_iterator it = getCurrentText()->file->fileName().end();
    --it;
    QString ext;
    while (it != getCurrentText()->file->fileName().begin() and *it != '.') {
        ext.prepend(*it);
        --it;
    }
    std::map<QString, QString>::iterator guess = languages.find(ext);
    if (guess == languages.end()) return "?";
    else return guess->second;
}

void MyForm::showFileSize() {
    if (ui.tabWidget->count() == 0 or getCurrentText()->file == nullptr) {
        emit fileSize("-");
        return;
    }
    QString size = QString::number((unsigned int)getCurrentText()->file->size());
    emit fileSize("size: " + size + " bytes");
}

void MyForm::showLanguage() {
    if (ui.tabWidget->count() == 0 or getCurrentText()->file == nullptr) {
        emit lang("-");
        return;
    }
    emit lang("lang: " + guessLang());
}

void MyForm::setModified() {
    if (getCurrentText()->saved) {
        getCurrentText()->saved = false;
        return;
    }
    if (getCurrentText()->file == nullptr) {
        getCurrentText()->modified = true;
        ui.tabWidget->setTabText(ui.tabWidget->currentIndex(), "New File*");
    }
    else {
        getCurrentText()->modified = true;
        ui.tabWidget->setTabText(ui.tabWidget->currentIndex(), QFileInfo(*(getCurrentText()->file)).fileName() + "*");
    }
}

void MyForm::setUnmodified() {
    getCurrentText()->modified = false;
    if (getCurrentText()->file == nullptr)
        ui.tabWidget->setTabText(ui.tabWidget->currentIndex(), "New File");
    else
        ui.tabWidget->setTabText(ui.tabWidget->currentIndex(), QFileInfo(*(getCurrentText()->file)).fileName());
}

void MyForm::showHelp(){
    QMessageBox msgBox;
    msgBox.setText("Thanks for using FIBedit!");
    msgBox.setInformativeText("Here are some key combinations:\nCtrl+C\tCopy selected text\nCtrl+V\tPaste selected text"
            "\nCtrl+Z\tUndo last changes\nCtrl+Shift+Z\tRedo last changes\nCtrl+A\tSelect all file\nCtrl+O\tOpen file from disk"
            "\nCtrl+N\tNew blank file\nCtrl+S\tSave file to disk\nCtrl+W\tClose file");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("Information");
    msgBox.setWindowFlags(msgBox.windowFlags() | Qt::WindowCloseButtonHint); // Agregar el botón de cerrar
    msgBox.exec();
}

void MyForm::newFile() {
    ui.tabWidget->insertTab(ui.tabWidget->count(), new MyPlainTextEdit(this), QIcon(QString("")), "New File");
    ui.tabWidget->setCurrentIndex(ui.tabWidget->count()-1);
    connect(getCurrentText(), SIGNAL(cursorPositionChanged()), this, SLOT(onCursorChanged()));
    connect(getCurrentText(), SIGNAL(textChanged()), this, SLOT(setModified()));
    getCurrentTab()->setFont(QFont("monospace", 12));
    setUnmodified();
}

void MyForm::openFile() {
    QString path = QFileDialog::getOpenFileName(this, "Open File:");
    if (path == "") return;
    int index = existsFile(path);
    if(index != -1){
        ui.tabWidget->setCurrentIndex(index);
        return;
    }
    QFile *nfile = new QFile(path);
    if (nfile->open(QFile::ReadOnly)) {
        newFile();
        ui.tabWidget->setTabText(ui.tabWidget->currentIndex(), QFileInfo(*nfile).fileName());
        getCurrentText()->file = nfile;
        QTextStream in(nfile);
        getCurrentText()->setPlainText(in.readAll());
        nfile->close();
        watcher->addPath(nfile->fileName());
        showFileSize();
        showLanguage();
        setUnmodified();
    }
}

void MyForm::saveFile() {
    if (ui.tabWidget->count() == 0) return;
    if (getCurrentText()->file == nullptr) {
        QString path = QFileDialog::getSaveFileName(this, "Save File As:");
        if (path == "") return;
        getCurrentText()->file = new QFile(path);
        ui.tabWidget->setTabText(ui.tabWidget->currentIndex(), QFileInfo(*(getCurrentText()->file)).fileName());
    }
    QString text = getCurrentText()->toPlainText();
    if (getCurrentText()->file->open(QFile::WriteOnly)) {
        QTextStream out(getCurrentText()->file);
        out << text;
        getCurrentText()->file->close();
        watcher->addPath(getCurrentText()->file->fileName());
        showFileSize();
        showLanguage();
        setUnmodified();
        getCurrentText()->saved = true;
    }
}

void MyForm::closeFile() {
    if (ui.tabWidget->count() == 0) return;
    
    //check if is not saved
    if (getCurrentText()->modified == true) {
        auto ret = QMessageBox::warning(this, "Closing unsaved file!",
                "You are about to close a modified file,\nclosing it will delete your changes",
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        switch (ret) {
        case QMessageBox::Save:
            saveFile();
            return;
            break;
        case QMessageBox::Discard:
            break;
        case QMessageBox::Cancel:
            return;
            break;
        }
    }
    if (getCurrentText()->file != nullptr)
        watcher->removePath(getCurrentText()->file->fileName());
    QWidget *tab = getCurrentTab();
    tab->close();
    delete tab;
}

void MyForm::closeFile(int index) {
    if (ui.tabWidget->count() == 0) return;
    
    //check if is not saved
    if (getCurrentText()->modified == true) {
        auto ret = QMessageBox::warning(this, "Closing unsaved file!",
                "You are about to close a modified file,\nclosing it will delete your changes",
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        switch (ret) {
        case QMessageBox::Save:
            saveFile();
            break;
        case QMessageBox::Discard:
            goto cont;
            break;
        case QMessageBox::Cancel:
            return;
            break;
        }
    }
cont:
    if (getTextByIndex(index)->file != nullptr)
        watcher->removePath(getTextByIndex(index)->file->fileName());
    ui.tabWidget->removeTab(index);
}

void MyForm::onFileChanged(const QString &path) {
    if (getTextByPath(path)->file->open(QFile::ReadOnly)) {
        watcher->removePath(path);
        QTextStream in(getTextByPath(path)->file);
        getTextByPath(path)->setPlainText(in.readAll());
        showFileSize();
        getTextByPath(path)->file->close();
        watcher->addPath(path);
    }
}

void MyForm::onCursorChanged() {
    if (ui.tabWidget->count() == 0) {
        emit line("-");
        emit column("-");
        return;
    }
    QTextCursor cursor = getCurrentText()->textCursor();
    emit line("lin: " + QString::number(cursor.blockNumber() + 1));
    emit column("col: " + QString::number(cursor.columnNumber() + 1));
}

QWidget* MyForm::getCurrentTab() {
    return dynamic_cast<QWidget*>(ui.tabWidget->currentWidget());
}

MyPlainTextEdit* MyForm::getCurrentText() {
    return dynamic_cast<MyPlainTextEdit*>(ui.tabWidget->currentWidget());
}

MyPlainTextEdit* MyForm::getTextByIndex(int index) {
    return dynamic_cast<MyPlainTextEdit*>(ui.tabWidget->widget(index));
}

MyPlainTextEdit* MyForm::getTextByPath(const QString &path) {
    for (int i = 0; i < ui.tabWidget->count(); ++i) {
        if (getTextByIndex(i)->file != nullptr)
            if (getTextByIndex(i)->file->fileName() == path) return getTextByIndex(i);
    }
    return nullptr;
}

int MyForm::existsFile(const QString &path){
    for (int i = 0; i < ui.tabWidget->count(); ++i) {
        if (getTextByIndex(i)->file != nullptr)
            if (getTextByIndex(i)->file->fileName() == path) return i;
    }
    return -1;
}
