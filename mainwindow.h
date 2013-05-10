#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef HAVE_QT5
	#include <QMainWindow>
#else 
	#include <QtGui/QMainWindow>
#endif
#include <QMap>
#include <QStackedWidget>
#include <QLabel>
#include <QSettings>
#include <QGridLayout>
#include <QPushButton>
#include <QDateTimeEdit>
#include <QTableWidget>
#include <QSignalMapper>
#include <QComboBox>
#include <QBasicTimer>
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
		dayInLine * in_day_line_of_day;		// Линия дня с временными отметками.
		QComboBox * works_dropdown_list;	// Выпадающий список дел.
		QPushButton * start_stop_button;	// Кнопка начала/остановки учета.
		QTableWidget * planned_works_table;	// Список дел, которые еще запланированы на текущий день.
		QBasicTimer timer;					// Таймер для обновления окна во время учета дела.
		
		// Список дел.
		QWidget works_list_window;
		QTableWidget * works_list_table;
		bool need_reaction_for_signal_cellChanged;	// Необходимо, что бы не было реакции на сигнал при заполнении таблицы. Так же, что бы восстановление данных при неудачном редактировании не вызвало связанный слот рекурсивно.
		bool for_add_work_was_modif_name;			// Было ли изменение названия добавляемого дела.
		bool for_add_work_was_modif_plan;			// Было ли изменение плана добавляемого дела.
		QSignalMapper delete_work_buttons_map;
		
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
		void show_warning_message (const QString & title, const QString & message); // Показ окошка с сообщением и кнопкой закрытия.
		
	protected:
		void timerEvent (QTimerEvent *);
    
	public:
		MainWindow ();

	public slots:
		void show_about_message ();									// Показ окошка с информацией о программе.
		void show_works_list_window ();								// Показать окно со списком дел.
		void update_works_list_window ();							// Обновить содержимое окна со списком дел.
		void show_select_file_dialog (const QString * cur_dir = 0);	// Выбор файла с БД.
		void set_params_from_new_db ();								// Реакция на открытие новой БД.
		void change_main_widget ();			// Изменение главного виджета в зависимости от текущего статуса.
		void update_widget_out_of_day ();	// Обновление окна out day.
		void update_widget_in_day ();		// Обновление окна in day.
		void add_day_button_clicked ();		// Добавляем новый день. Если ошибка, показываем сообщение.
		void update_day_button_clicked ();	// Обновляем окончание последнего дня. Если ошибка, показываем сообщение.
		
		void delete_work (int id);			// При ошибке показывается сообщение, иначе виджеты обновляются.
		void modif_work_from_table_widget (int row, int col); // Обновление или добавление дела. В параметрах - координаты ячейки, измененные пользователем. 
		void start_stop_button_clicked ();	// Была нажата кнопка старт/стоп для учета дела.
};

#endif // MAINWINDOW_H
