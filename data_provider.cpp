#include "data_provider.h"
#include "utils.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

QSqlDatabase data_provider::cur_db = QSqlDatabase::addDatabase("QSQLITE");
QSqlDatabase data_provider::new_db = QSqlDatabase::addDatabase("QSQLITE", "new db");
QString data_provider::datetime_db_format = "yyyy-MM-dd HH:mm:ss";

data_provider * data_provider::get_obj() {
	static data_provider obj;
	return & obj;
}

data_provider::data_provider () : file_full_name(""), file_dir(""), file_name("") {}

data_provider::~data_provider () {
	if (cur_db.isOpen()) cur_db.close();
}

// День можно добавлять, только если предыдущий день уже завершился. Начало дня устанавливается на текущий момент. Окончание дня должно быть после начала.
bool data_provider::check_allow_add_day (day & element, QString & error_message) {
	bool result = false;
	day last_day = get_last_day();
	QDateTime now = QDateTime::currentDateTime();
	element.start = now;
	if (last_day.isValid() and last_day.end >= now) {
		error_message = tr("Day add error") + " - " + tr("current day is not end");
	} else if (!element.isCorrect()) {
		error_message = tr("Day add error") + " - " + tr("added day is not correct");
	} else {
		result = true;
		element.id = last_day.id + 1;
	}
	return result;
}

// Можно менять только последний учитываемый день. Меняется только окончание дня, причем новое окончание должно быть после текущего момента.
bool data_provider::check_allow_udpate_day (day & element, QString & error_message) {
	bool result = false;
	day last_day = get_last_day();
	QDateTime now = QDateTime::currentDateTime();
	if (!last_day.isValid() or last_day.id != element.id) {
		error_message = tr("Day update error") + " - " + tr("only last day can be updated");
	} else {
		element.start = last_day.start;
		if (element.end <= now) error_message = tr("Day update error") + " - " + tr("end of day must be in future");
		else if (!element.isCorrect()) error_message = tr("Day update error") + " - " + tr("updated day is not correct");
		else result = true;
	}
	return result;
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

day data_provider::get_last_day () {
	day result;
	if (cur_db.isOpen()) {
		QSqlQuery query(cur_db);
		QString sql = "SELECT * FROM days ORDER BY id DESC LIMIT 1";
		if (query.exec(sql)) {
			QSqlRecord rec = query.record();
			if (query.next()) {
				result.id = query.value(rec.indexOf("id")).toInt();
				result.start = QDateTime::fromString(query.value(rec.indexOf("datetime_start")).toString(), datetime_db_format);
				result.end = QDateTime::fromString(query.value(rec.indexOf("datetime_end")).toString(), datetime_db_format);
			}
		}
	}
	return result;
}

bool data_provider::add_day (day element, QString * error_message) {
	bool result = false;
	QString err;
	if (!cur_db.isOpen()) {
		err = tr("Day add error") + " - " + tr("data file is not open");
	} else if (check_allow_add_day(element, err)) {
		QSqlQuery query(cur_db);
		query.prepare("INSERT INTO days (id, datetime_start, datetime_end) VALUES (:id, :start, :end)");
		query.bindValue(":id", element.id);
		query.bindValue(":start", element.start.toString(datetime_db_format));
		query.bindValue(":end", element.end.toString(datetime_db_format));
		if (query.exec()) result = true;
		else err = tr("Day add error") + " - " + query.lastError().text();
	}
	if (error_message) *error_message = err;
	return result;
}

bool data_provider::update_day (day element, QString * error_message) {
	bool result = false;
	QString err;
	if (!cur_db.isOpen()) {
		err = tr("Day update error") + " - " + tr("data file is not open");
	} else if (check_allow_udpate_day(element, err)) {
		QSqlQuery query(cur_db);
		query.prepare("UPDATE days SET datetime_start = :start, datetime_end = :end WHERE id = :id");
		query.bindValue(":start", element.start.toString(datetime_db_format));
		query.bindValue(":end", element.end.toString(datetime_db_format));
		query.bindValue(":id", element.id);
		if (query.exec()) result = true;
		else err = tr("Day update error") + " - " + query.lastError().text();
	}
	if (error_message) *error_message = err;
	return result;
}
