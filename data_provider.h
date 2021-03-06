#ifndef DATA_PROVIDER_H
#define DATA_PROVIDER_H

#include "content_types.h"
#include <QString>
#include <QObject>
#include <QSqlDatabase>
#include <QMap>

// Это класс, осуществляющий запросы к БД и предоставляющий данные остальной программе. Поддерживается только SQLite средствами Qt.

class data_provider : public QObject
{
	Q_OBJECT

	// Поддержка шаблона "одиночка".
	private:
		data_provider ();										// Запрет создания объекта вне класса.
		data_provider (const data_provider &) : QObject() {}	// Запрет копирования объекта вне класса.
		void operator= (data_provider &) {}						// Запрет присваивания объекта вне класса.
		~data_provider ();
	public:
		static data_provider * get_obj ();						// Возвращает указатель на объект.
	// Поддержка шаблона "одиночка" завершена.

	private:
		QSqlDatabase cur_db;	// Соединение с БД, где храним данные.
		QSqlDatabase new_db; // Соединение для проверки БД при изменении файла. Обычно закрыто.

		QString file_full_name;	// Полное имя файла с каталогом.
		QString file_dir;		// Только каталог.
		QString file_name;		// Только имя файла, полное с расширением.
		
		bool try_connect_new_DB (const QString & full_name, QString & error_message); // Открываем БД и пробуем создать нужные таблицы.
		void change_db_from_new (); // Прописывает в cur_db то же, что и в new_db, и открывает cur_db.
		
		// В следующих методах сосредоточена логика по добавлению и обновлению данных в БД. Методы модифицируют добавляемые данные. 
		bool check_allow_add_day (day & element, QString & error_message);
		bool check_allow_udpate_day (day & element, QString & error_message);
		bool check_allow_add_work (work & element, QString & error_message);
		bool check_allow_update_work (work & element, QString & error_message);
		bool check_allow_add_time_period (time_period & element, QString & error_message);
		bool check_allow_update_time_period (time_period & element, QString & error_message);

	public:
		static QString DATETIME_DB_FORMAT;
		
		QString get_file_full_name () const {return file_full_name;}
		QString get_file_dir () const {return file_dir;}
		QString get_file_name () const {return file_name;}
		bool check_file_name (const QString & full_name) const {return full_name == file_full_name;}
		bool isOpen () {return cur_db.isOpen();}

		bool set_file (const QString & full_name, QString * p_error_message = 0); // Установка нового файла БД. Возвращает true, если файл существует и его можно открыть на запись, либо файла не существует и его можно создать. Иначе возвращает false, не меняя текущие данные. В этом случае error_message перезаписывается текстом ошибки.
		void check_last_time_period_for_close (); // Если последний временной промежуток еще не закрыт, а учитываемый день уже закончился, закрываем промежуток на момент окончания дня.
		
		// Методы получения данных.
		day get_last_day ();		// Последний день из БД. Если дней нет, возвращается не валидный день.
		work get_work (int id);		// Если id <= 0 или работы нет в БД, возвращается невалидный work.
		QList<work> get_works ();	// Получить все дела из БД.
		time_period get_last_time_period ();					// Возвращается последний временной период. 
		QList<time_period> get_time_periods_for_current_day ();	// Все периоды за последний день.
		QMap<work, int> get_planned_works_for_last_day ();		// Дела, которые еще запланированы на сегодя, и количество оставшихся секунд.
		
		// Методы модификации данных. Данные передаются не по ссылке, а копируются.
		bool add_day (day element, QString * error_message = 0);
		bool update_day (day element, QString * error_message = 0);
		bool add_work (work element, QString * error_message = 0);
		bool update_work (work element, QString * error_message = 0);
		bool delete_work (int id, QString * error_message = 0);
		bool add_time_period (time_period element, QString * error_message = 0);
		bool update_time_period (time_period element, QString * error_message = 0);

	signals:
		void database_file_was_changed ();	// Была открыта другая БД.
		void out_from_day ();				// Учитываемый день начался либо завершился.
		void works_updated ();				// Изменения в делах - добавлено, удалено или отредактировано.
};

#endif // DATA_PROVIDER_H
