#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QMap>
#include "data_provider.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

	private:
		QMap<QString, QAction *> actions;
    
	public:
		MainWindow ();

	public slots:
		void show_about_message ();									// Показ окошка с информацией о программе.
		void show_select_file_dialog (const QString * cur_dir = 0);	// Выбор файла с БД.
};

#endif // MAINWINDOW_H
