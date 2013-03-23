#include "data_provider.h"
#include "utils.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>

QSqlDatabase data_provider::cur_db = QSqlDatabase::addDatabase("QSQLITE");
QSqlDatabase data_provider::new_db = QSqlDatabase::addDatabase("QSQLITE", "new db");

data_provider * data_provider::get_obj() {
	static data_provider obj;
	return & obj;
}

data_provider::data_provider () : file_full_name(""), file_dir(""), file_name("") {}

data_provider::~data_provider () {
	if (cur_db.isOpen()) cur_db.close();
}

bool data_provider::try_connect_new_DB (const QString & full_name, QString & error_message) {
	bool result = false;
	if (new_db.isOpen()) new_db.close();
	new_db.setDatabaseName(full_name);
	if (new_db.open()) {
		QSqlQuery query(new_db);
		QString init_query[6];
		init_query[0] = "CREATE TABLE IF NOT EXISTS works (id INTEGER PRIMARY KEY, name TEXT, type TEXT, created_datetime TEXT, deleted INTEGER)";
		init_query[1] = "CREATE TABLE IF NOT EXISTS one_works (id INTEGER PRIMARY KEY, work_id INTEGER, count INTEGER)";
		init_query[2] = "CREATE TABLE IF NOT EXISTS long_works (id INTEGER PRIMARY KEY, work_id INTEGER, plan INTEGER)";
		init_query[3] = "CREATE TABLE IF NOT EXISTS days (id INTEGER PRIMARY KEY, datetime_start TEXT, datetime_end TEXT)";
		init_query[4] = "CREATE TABLE IF NOT EXISTS time_periods (id INTEGER PRIMARY KEY, work_id INTEGER, datetime_start TEXT, datetime_end TEXT)";
		init_query[5] = "CREATE TABLE IF NOT EXISTS work_checkeds (id INTEGER PRIMARY KEY, work_id INTEGER, datetime TEXT)";
		result = true;
		for (int i = 0; i < 6; i++)
			if (!query.exec(init_query[i])) {
				error_message = tr("Can not init database.") + "\n" + query.lastError().text();
				result = false;
				break;
			}
		new_db.close();
	} else {
		error_message = tr("Can not open database.") + "\n" + new_db.lastError().text();
	}
	return result;
}

void data_provider::change_db_from_new () {
	if (cur_db.isOpen()) cur_db.close();
	cur_db.setDatabaseName(new_db.databaseName());
	cur_db.open();
	QFileInfo finfo(new_db.databaseName());
	file_full_name = finfo.absoluteFilePath();
	file_dir = finfo.absolutePath();
	file_name = finfo.fileName();
	emit database_file_was_changed();
}

bool data_provider::set_file (const QString & full_name, QString * p_error_message) {
	bool result = false;
	QString error_message = "";
	if (utils::check_can_be_edit(full_name, error_message)) {
		QFileInfo finfo(full_name);
		if (cur_db.isOpen() and cur_db.databaseName() == finfo.absoluteFilePath()) {
			error_message = tr("This file is already opened");
		} else {
			result = try_connect_new_DB(full_name, error_message);
			if (result) change_db_from_new();
		}
	}
	if (p_error_message) *p_error_message = error_message;
	return result;
}
