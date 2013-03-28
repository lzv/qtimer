#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QMap>
#include <QStackedWidget>
#include <QLabel>
#include <QSettings>
#include <QGridLayout>
#include <QPushButton>
#include <QDateTimeEdit>
#include <QTableWidget>
#include "data_provider.h"
#include "myWidgets.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

	private:
		QMap<QString, QAction *> actions;			// Действия, для главного меню и еще чего понадобится.
		QStackedWidget * main_widgets;				// Главная область программы, разные виджеты в разных состояниях.
		QMap<QString, int> main_widgets_indexes;	// Индексы виджетов в main_widgets.
		QLabel * status_bar_label;					// Виджет с постоянным текстом в строке состояния.
		
		// Виджеты для окна out_of_day
		QLabel * out_of_day_message;
		QLabel * out_of_day_add_hint;
		QLabel * out_of_day_update_hint;
		QDateTimeEdit * out_of_day_add_day_edit;
		QDateTimeEdit * out_of_day_update_day_edit;
		QPushButton * out_of_day_add_day_button;
		QPushButton * out_of_day_update_day_button;
		
		// Виджеты для окна in_day.
		dayInLine * in_day_line_of_day; // Линия дня с временными отметками.
		
		// Список дел.
		QWidget works_list_window;
		int works_list_window_width, works_list_window_height;
		QTableWidget * works_list_table;
		
		static QSettings settings;
		
		// Методы для разгрузки конструктора.
		void create_actions ();					// Добавление действий.
		void create_main_menu ();				// Создание главного меню.
		void create_status_bar ();				// Создание строки состояния.
		void create_works_list_window ();		// Сознание окна со списком дел без показа.
		QWidget * create_widget_db_not_open ();	// Виджет, когда БД еще не открыта.
		QWidget * create_widget_out_of_day ();	// Виджет, когда текущий момент вне дня, или дней еще нет.
		QWidget * create_widget_in_day ();		// Виджет, когда текущий момент внутри дня, и идет основная работа.
		int get_need_main_widget_index ();		// Индекс главного виджета в зависимости от текущего состояния.
		QGridLayout * get_stretch_QGridLayout (int row1, int row2, int col1, int col2, int stretch, int other_stretch = 0); // Возвращает QGridLayout, в котором указанным строкам и столбцам установлен коэффициент растяжения stretch. Тем, которые между указанных, устанавливается other_stretch.
    
	public:
		MainWindow ();

	public slots:
		void show_about_message ();									// Показ окошка с информацией о программе.
		void show_works_list_window ();								// Показать окно со списком дел.
		void update_works_list_window ();							// Обновить содержимое окна со списком дел.
		void show_select_file_dialog (const QString * cur_dir = 0);	// Выбор файла с БД.
		void set_params_from_new_db ();								// Реакция на открытие новой БД.
		void change_main_widget ();			// Изменение главного виджета в зависимости от текущего статуса.
		void update_widget_out_of_day ();	// Обновление виджета при окончании дня или открытии новой БД.
		void update_widget_in_day ();		// Обновление виджета при начале дня или открытии новой БД.
		void add_day_button_clicked ();		// Добавляем новый день. Если ошибка, показываем сообщение.
		void update_day_button_clicked ();	// Обновляем окончание последнего дня. Если ошибка, показываем сообщение.
};

#endif // MAINWINDOW_H
