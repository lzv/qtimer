#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QMap>
#include <QStackedWidget>
#include <QLabel>
#include <QSettings>
#include "data_provider.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

	private:
		QMap<QString, QAction *> actions;			// Действия, для главного меню и еще чего понадобится.
		QStackedWidget * main_widgets;				// Главная область программы, разные виджеты в разных состояниях.
		QMap<QString, int> main_widgets_indexes;	// Индексы виджетов в main_widgets.
		QLabel * status_bar_label;					// Виджет с постоянным текстом в строке состояния.
		
		static QSettings settings;
		
		// Методы для разгрузки конструктора.
		void create_actions ();					// Добавление действий.
		void create_main_menu ();				// Создание главного меню.
		void create_status_bar ();				// Создание строки состояния.
		QWidget * create_widget_out_of_day ();	// Виджет, когда текущий момент вне дня, или дней еще нет.
		QWidget * create_widget_in_day ();		// Виджет, когда текущий момент внутри дня, и идет основная работа.
    
	public:
		MainWindow ();

	public slots:
		void show_about_message ();									// Показ окошка с информацией о программе.
		void show_select_file_dialog (const QString * cur_dir = 0);	// Выбор файла с БД.
		void set_params_from_new_db ();								// Реакция на открытие новой БД.
};

#endif // MAINWINDOW_H
