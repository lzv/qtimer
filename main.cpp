#include <QtGui>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
	QTextCodec* codec = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForTr(codec);
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForLocale(codec);

    QApplication a(argc, argv);
	QString sys_lang = QLocale::system().name();
	QTranslator translator;
	if (sys_lang == "ru_RU" or sys_lang == "ru") {
		translator.load("lang_ru", QCoreApplication::applicationDirPath());
		a.installTranslator(& translator);
	}
	
    MainWindow w;
    w.show();
    
    return a.exec();
}
