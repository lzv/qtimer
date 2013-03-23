#include <QtGui>
#include "utils.h"
#include "content_types.h"
#include "data_provider.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
	QTextCodec* codec = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForTr(codec);
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForLocale(codec);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}