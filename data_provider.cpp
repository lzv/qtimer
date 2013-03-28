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
QString data_provider::DATETIME_DB_FORMAT = "yyyy-MM-dd HH:mm:ss";

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
		error_message = tr("Add day error") + " - " + tr("current day is not end");
	} else if (!element.isCorrect()) {
		error_message = tr("Add day error") + " - " + tr("added day is not correct");
	} else {
		result = true;
		element.id = last_day.id + 1;
	}
	return result;
}

// Имя нового дела не должно совпадать с уже имеющимися делами. Дело должно быть корректным.
bool data_provider::check_allow_add_work (work & element, QString & error_message) {
	bool result = false;
	if (element.isCorrect()) {
		element.id = 0;
		QList<work> db_works = get_works();
		result = true;
		for (QList<work>::const_iterator i = db_works.begin(); i != db_works.end(); ++i) {
			if (element.name == i->name) {
				result = false;
				error_message = tr("Add work error") + " - " + tr("duplication of name");
				break;
			}
			if (element.id < i->id) element.id = i->id;
		}
		if (result) element.id++;
	} else {
		error_message = tr("Add work error") + " - " + tr("added work is not correct");
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

// Элемент с указанным id должен существовать. Данные должны быть корректны. Если меняется название, оно не должно совпадать с уже существующими делами.
bool data_provider::check_allow_update_work (work & element, QString & error_message) {
	bool result = false;
	work selected = get_work(element.id);
	if (selected.isValid()) {
		if (element.isCorrect()) {
			result = true;
			if (selected.name != element.name) {
				QList<work> db_works = get_works();
				for (QList<work>::const_iterator i = db_works.begin(); i != db_works.end(); ++i)
					if (element.name == i->name) { /* id не проверяем, так как проверка в  if выше */
						result = false;
						error_message = error_message = tr("Update work error") + " - " + tr("duplication of name");
						break;
					}
			}
		} else {
			error_message = tr("Update work error") + " - " + tr("updated work is not correct");
		}
	} else {
		error_message = tr("Update work error") + " - " + tr("updated work is not exists");
	}
	return result;
}

bool data_provider::try_connect_new_DB (const QString & full_name, QString & error_message) {
	bool result = false;
	if (new_db.isOpen()) new_db.close();
	new_db.setDatabaseName(full_name);
	if (new_db.open()) {
		QSqlQuery query(new_db);
		QString init_query[3];
		init_query[0] = "CREATE TABLE IF NOT EXISTS works (id INTEGER PRIMARY KEY, name TEXT, plan INTEGER)";
		init_query[1] = "CREATE TABLE IF NOT EXISTS days (id INTEGER PRIMARY KEY, datetime_start TEXT, datetime_end TEXT)";
		init_query[2] = "CREATE TABLE IF NOT EXISTS time_periods (id INTEGER PRIMARY KEY, work_id INTEGER, datetime_start TEXT, datetime_end TEXT)";
		result = true;
		for (int i = 0; i < 3; i++)
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
		if (query.exec(sql) and query.next()) {
			QSqlRecord rec = query.record();
			result.id = query.value(rec.indexOf("id")).toInt();
			result.start = QDateTime::fromString(query.value(rec.indexOf("datetime_start")).toString(), DATETIME_DB_FORMAT);
			result.end = QDateTime::fromString(query.value(rec.indexOf("datetime_end")).toString(), DATETIME_DB_FORMAT);
		}
	}
	return result;
}

work data_provider::get_work (int id) {
	work result;
	if (id > 0 and cur_db.isOpen()) {
		QSqlQuery query(cur_db);
		query.prepare("SELECT * FROM works WHERE id = :id");
		query.bindValue(":id", id);
		if (query.exec() and query.next()) {
			QSqlRecord rec = query.record();
			result.id = query.value(rec.indexOf("id")).toInt();
			result.name = query.value(rec.indexOf("name")).toString();
			result.plan = query.value(rec.indexOf("plan")).toInt();
		}
	}
	return result;
}

QList<work> data_provider::get_works () {
	QList<work> result;
	if (cur_db.isOpen()) {
		QSqlQuery query(cur_db);
		QString sql = "SELECT * FROM works ORDER BY id ASC";
		if (query.exec(sql)) {
			QSqlRecord rec = query.record();
			int index_id = rec.indexOf("id"), index_name = rec.indexOf("name"), index_plan = rec.indexOf("plan");
			work element;
			while (query.next()) {
				element.id = query.value(index_id).toInt();
				element.name = query.value(index_name).toString();
				element.plan = query.value(index_plan).toInt();
				result.push_back(element);
			}
		}
	}
	return result;
}

bool data_provider::add_day (day element, QString * error_message) {
	bool result = false;
	QString err;
	if (!cur_db.isOpen()) {
		err = tr("Add day error") + " - " + tr("data file is not open");
	} else if (check_allow_add_day(element, err)) {
		QSqlQuery query(cur_db);
		query.prepare("INSERT INTO days (id, datetime_start, datetime_end) VALUES (:id, :start, :end)");
		query.bindValue(":id", element.id);
		query.bindValue(":start", element.start.toString(DATETIME_DB_FORMAT));
		query.bindValue(":end", element.end.toString(DATETIME_DB_FORMAT));
		if (query.exec()) result = true;
		else err = tr("Add day error") + " - " + query.lastError().text();
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
		query.bindValue(":start", element.start.toString(DATETIME_DB_FORMAT));
		query.bindValue(":end", element.end.toString(DATETIME_DB_FORMAT));
		query.bindValue(":id", element.id);
		if (query.exec()) result = true;
		else err = tr("Day update error") + " - " + query.lastError().text();
	}
	if (error_message) *error_message = err;
	return result;
}
