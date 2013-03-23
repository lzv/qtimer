#ifndef DATA_PROVIDER_H
#define DATA_PROVIDER_H

#include "content_types.h"
#include <QString>
#include <QObject>
#include <QSqlDatabase>

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
		static QSqlDatabase cur_db;	// Соединение с БД, где храним данные.
		static QSqlDatabase new_db; // Соединение для проверки БД при изменении файла. Обычно закрыто.

		QString file_full_name;	// Полное имя файла с каталогом.
		QString file_dir;		// Только каталог.
		QString file_name;		// Только имя файла, полное с расширением.
		
		bool try_connect_new_DB (const QString & full_name, QString & error_message); // Открываем БД и пробуем создать нужные таблицы.
		void change_db_from_new (); // Прописывает в cur_db то же, что и в new_db, и открывает cur_db.

	public:
		QString get_file_full_name () const {return file_full_name;}
		QString get_file_dir () const {return file_dir;}
		QString get_file_name () const {return file_name;}
		bool check_file_name (const QString & full_name) const {return full_name == file_full_name;}
		bool isOpen () {return cur_db.isOpen();}

		bool set_file (const QString & full_name, QString * p_error_message = 0); // Установка нового файла БД. Возвращает true, если файл существует и его можно открыть на запись, либо файла не существует и его можно создать. Иначе возвращает false, не меняя текущие данные. В этом случае error_message перезаписывается текстом ошибки.

	signals:
		void database_file_was_changed (); // Сигнал о том, что была открыта другая БД.
};

#endif // DATA_PROVIDER_H
