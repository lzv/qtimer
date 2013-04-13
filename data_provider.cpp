#include "data_provider.h"
#include "utils.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

QString data_provider::DATETIME_DB_FORMAT = "yyyy-MM-dd HH:mm:ss";

data_provider * data_provider::get_obj() {
	static data_provider obj;
	return & obj;
}

data_provider::data_provider () : file_full_name(""), file_dir(""), file_name("")
{
	cur_db = QSqlDatabase::addDatabase("QSQLITE");
	new_db = QSqlDatabase::addDatabase("QSQLITE", "new db");
}

data_provider::~data_provider () {
	if (cur_db.isOpen()) cur_db.close();
}

// День можно добавлять, только если предыдущий день уже завершился. Начало дня устанавливается на текущий момент. Окончание дня должно быть после начала.
bool data_provider::check_allow_add_day (day & element, QString & error_message)
{
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
bool data_provider::check_allow_add_work (work & element, QString & error_message)
{
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

// Можно добавлять, только если предыдущий временной период закрыт и текущий день еще не завершен. Дело, к которому привязан период, должно существовать. Начало периода должно быть в пределах текущего дня и после окончания прошлого периода, но не позже текущего момента. Окончание устанавливается на QDateTime(). 
bool data_provider::check_allow_add_time_period (time_period & element, QString & error_message)
{
	bool result = false;
	QDateTime current_dt = QDateTime::currentDateTime();
	day last_day = get_last_day();
	if (!last_day.isValid()) {
		error_message = tr("no current day");
	} else if (last_day.end <= current_dt) {
		error_message = tr("last day is over already");
	} else {
		time_period last_period = get_last_time_period();
		work linked_work = get_work(element.work_id);
		if (last_period.isValid() and last_period.isOpened()) {
			error_message = tr("last time period is not closed");
		} else if (!linked_work.isValid()) {
			error_message = tr("work of time period is not exists");
		} else {
			QDateTime need_from = last_day.start;
			if (last_period.isValid() and need_from < last_period.end) need_from = last_period.end;
			if (element.start < need_from) element.start = need_from;
			else if (element.start > current_dt) element.start = current_dt;
			element.end = QDateTime();
			if (element.isCorrect()) {
				element.id = last_period.isValid() ? last_period.id + 1 : 1;
				result = true;
			} else {
				error_message = tr("added time period is not correct");
			}
		}
	}
	if (!result) error_message = tr("Add time period error") + " - " + error_message;
	return result;
}

// Можно менять только последний учитываемый день. Меняется только окончание дня, причем новое окончание должно быть после текущего момента.
bool data_provider::check_allow_udpate_day (day & element, QString & error_message)
{
	bool result = false;
	day last_day = get_last_day();
	QDateTime now = QDateTime::currentDateTime();
	if (!last_day.isValid() or last_day.id != element.id) {
		error_message = tr("only last day can be updated");
	} else {
		element.start = last_day.start;
		if (element.end <= now) error_message = tr("end of day must be in future");
		else if (!element.isCorrect()) error_message = tr("updated day is not correct");
		else result = true;
	}
	if (!result) error_message = tr("Day update error") + " - " + error_message;
	return result;
}

// Элемент с указанным id должен существовать. Данные должны быть корректны. Если меняется название, оно не должно совпадать с уже существующими делами.
bool data_provider::check_allow_update_work (work & element, QString & error_message)
{
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
						error_message = tr("duplication of name");
						break;
					}
			}
		} else {
			error_message = tr("updated work is not correct");
		}
	} else {
		error_message = tr("updated work is not exists");
	}
	if (!result) error_message = tr("Update work error") + " - " + error_message;
	return result;
}

// Элемент с указанным id должен существовать. Можно обновить только окончание открытого периода, закрыв его. Если случилось так, что последний учитываемый день уже закончился, временной период закрывается на момент окончания дня.
bool data_provider::check_allow_update_time_period (time_period & element, QString & error_message) {
	bool result = false;
	time_period last_period = get_last_time_period();
	if (!last_period.isValid() or element.id != last_period.id) {
		error_message = tr("element is not last time period");
	} else if (last_period.isClosed()) {
		error_message = tr("last time period is closed already");
	} else if (!element.end.isValid()) {
		error_message = tr("need end of updated time period");
	} else {
		day last_day = get_last_day();
		if (!last_day.isValid()) {
			error_message = tr("last day is not exists");
		} else {
			element.work_id = last_period.work_id;
			element.start = last_period.start;
			if (last_day.end < element.end) element.end = last_day.end;
			if (element.isCorrect()) result = true;
			else error_message = tr("updated time period is not correct");
		}
	}
	if (!result) error_message = tr("Update time period error") + " - " + error_message;
	return result;
}

bool data_provider::try_connect_new_DB (const QString & full_name, QString & error_message)
{
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

void data_provider::change_db_from_new ()
{
	if (cur_db.isOpen()) cur_db.close();
	cur_db.setDatabaseName(new_db.databaseName());
	cur_db.open();
	QFileInfo finfo(new_db.databaseName());
	file_full_name = finfo.absoluteFilePath();
	file_dir = finfo.absolutePath();
	file_name = finfo.fileName();
	check_last_time_period_for_close();
	emit database_file_was_changed();
}

bool data_provider::set_file (const QString & full_name, QString * p_error_message)
{
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

day data_provider::get_last_day ()
{
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

work data_provider::get_work (int id)
{
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

QList<work> data_provider::get_works ()
{
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

time_period data_provider::get_last_time_period ()
{
	time_period result;
	if (cur_db.isOpen()) {
		QSqlQuery query(cur_db);
		QString sql = "SELECT * FROM time_periods ORDER BY id DESC LIMIT 1";
		if (query.exec(sql) and query.next()) {
			QSqlRecord rec = query.record();
			result.id = query.value(rec.indexOf("id")).toInt();
			result.work_id = query.value(rec.indexOf("work_id")).toInt();
			result.start = QDateTime::fromString(query.value(rec.indexOf("datetime_start")).toString(), DATETIME_DB_FORMAT);
			QString str_end = query.value(rec.indexOf("datetime_end")).toString();
			if (str_end != "-") result.end = QDateTime::fromString(str_end, DATETIME_DB_FORMAT);
		}
	}
	return result;
}

QList<time_period> data_provider::get_time_periods_for_current_day ()
{
	QList<time_period> result;
	day last_day = get_last_day();
	if (cur_db.isOpen() and last_day.isValid()) {
		QSqlQuery query(cur_db);
		query.prepare("SELECT * FROM time_periods WHERE datetime_start >= :start ORDER BY id ASC");
		query.bindValue(":start", last_day.start.toString(DATETIME_DB_FORMAT));
		if (query.exec()) {
			QSqlRecord rec = query.record();
			int ind_id = rec.indexOf("id"), ind_work_id = rec.indexOf("work_id"), ind_start = rec.indexOf("datetime_start"), ind_end = rec.indexOf("datetime_end");
			time_period element;
			while (query.next()) {
				element.id = query.value(ind_id).toInt();
				element.work_id = query.value(ind_work_id).toInt();
				element.start = QDateTime::fromString(query.value(ind_start).toString(), DATETIME_DB_FORMAT);
				QString str_end = query.value(ind_end).toString();
				element.end = str_end == "-" ? QDateTime() : QDateTime::fromString(str_end, DATETIME_DB_FORMAT);
				result.push_back(element);
			}
		}
	}
	return result;
}

bool data_provider::add_day (day element, QString * error_message)
{
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

bool data_provider::update_day (day element, QString * error_message)
{
	bool result = false;
	QString err;
	if (!cur_db.isOpen()) {
		err = tr("Update day error") + " - " + tr("data file is not open");
	} else if (check_allow_udpate_day(element, err)) {
		QSqlQuery query(cur_db);
		query.prepare("UPDATE days SET datetime_start = :start, datetime_end = :end WHERE id = :id");
		query.bindValue(":start", element.start.toString(DATETIME_DB_FORMAT));
		query.bindValue(":end", element.end.toString(DATETIME_DB_FORMAT));
		query.bindValue(":id", element.id);
		if (query.exec()) result = true;
		else err = tr("Update day error") + " - " + query.lastError().text();
	}
	if (error_message) *error_message = err;
	return result;
}


bool data_provider::delete_work (int id, QString * error_message)
{
	bool result = false;
	QString err;
	if (id <= 0) {
		err = tr("Delete work error") + " - " + tr("id is not correct");
	} else if (!cur_db.isOpen()) {
		err = tr("Delete work error") + " - " + tr("data file is not open");
	} else {
		QSqlQuery query(cur_db);
		QString sql1 = QString("DELETE FROM time_periods WHERE work_id = %1").arg(id);
		QString sql2 = QString("DELETE FROM works WHERE id = %1").arg(id);
		if (cur_db.transaction()) {
			if (query.exec(sql1) and query.exec(sql2)) {
				if (cur_db.commit()) {
					result = true;
					emit works_updated();
				} else {
					err = tr("Delete work error") + " - " + tr("can not commit. Database error: ") + cur_db.lastError().text();
					cur_db.rollback();
				}
			} else {
				err = tr("Delete work error") + " - " + query.lastError().text();
				cur_db.rollback();
			}
		} else {
			err = tr("Delete work error") + " - " + tr("can not start transaction. Database error: ") + cur_db.lastError().text();
		}
	}
	if (error_message) *error_message = err;
	return result;
}


bool data_provider::add_work (work element, QString * error_message)
{
	bool result = false;
	QString err;
	if (!cur_db.isOpen()) {
		err = tr("Add work error") + " - " + tr("data file is not open");
	} else if (check_allow_add_work(element, err)) {
		QSqlQuery query(cur_db);
		query.prepare("INSERT INTO works (id, name, plan) VALUES (:id, :name, :plan)");
		query.bindValue(":id", element.id);
		query.bindValue(":name", element.name);
		query.bindValue(":plan", element.plan);
		if (query.exec()) {
			result = true;
			emit works_updated();
		} else {
			err = tr("Add work error") + " - " + query.lastError().text();
		}
	}
	if (error_message) *error_message = err;
	return result;
}

// CREATE TABLE IF NOT EXISTS time_periods (id INTEGER PRIMARY KEY, work_id INTEGER, datetime_start TEXT, datetime_end TEXT)
bool data_provider::update_time_period (time_period element, QString * error_message)
{
	bool result = false;
	QString err;
	if (!cur_db.isOpen()) {
		err = tr("Update time period error") + " - " + tr("data file is not open");
	} else if (check_allow_update_time_period(element, err)) {
		QSqlQuery query(cur_db);
		query.prepare("UPDATE time_periods SET work_id = :work_id, datetime_start = :start, datetime_end = :end WHERE id = :id");
		query.bindValue(":id", element.id);
		query.bindValue(":work_id", element.work_id);
		query.bindValue(":start", element.start.toString(DATETIME_DB_FORMAT));
		query.bindValue(":end", element.end.isValid() ? element.end.toString(DATETIME_DB_FORMAT) : "-");
		if (query.exec()) result = true;
		else err = tr("Update time period error") + " - " + query.lastError().text();
	}
	if (error_message) *error_message = err;
	return result;
}

bool data_provider::update_work (work element, QString * error_message)
{
	bool result = false;
	QString err;
	if (!cur_db.isOpen()) {
		err = tr("Update work error") + " - " + tr("data file is not open");
	} else if (check_allow_update_work(element, err)) {
		QSqlQuery query(cur_db);
		query.prepare("UPDATE works SET name = :name, plan = :plan WHERE id = :id");
		query.bindValue(":name", element.name);
		query.bindValue(":plan", element.plan);
		query.bindValue(":id", element.id);
		if (query.exec()) {
			result = true;
			emit works_updated();
		} else {
			err = tr("Update work error") + " - " + query.lastError().text();
		}
	}
	if (error_message) *error_message = err;
	return result;
}

bool data_provider::add_time_period (time_period element, QString * error_message)
{
	bool result = false;
	QString err;
	if (!cur_db.isOpen()) {
		err = tr("Add time period error") + " - " + tr("data file is not open");
	} else if (check_allow_add_time_period(element, err)) {
		QSqlQuery query(cur_db);
		query.prepare("INSERT INTO time_periods (id, work_id, datetime_start, datetime_end) VALUES (:id, :work_id, :start, :end)");
		query.bindValue(":id", element.id);
		query.bindValue(":work_id", element.work_id);
		query.bindValue(":start", element.start.toString(DATETIME_DB_FORMAT));
		query.bindValue(":end", element.end.isValid() ? element.end.toString(DATETIME_DB_FORMAT) : "-");
		if (query.exec()) result = true;
		else err = tr("Add time period error") + " - " + query.lastError().text();
	}
	if (error_message) *error_message = err;
	return result;
}

void data_provider::check_last_time_period_for_close ()
{
	time_period last_tp = get_last_time_period();
	if (last_tp.isValid() and last_tp.isOpened()) {
		day last_day = get_last_day();
		if (last_day.isValid() and last_day.end <= QDateTime::currentDateTime()) {
			last_tp.end = last_day.end;
			update_time_period(last_tp);
		}
	}
}

QMap<work, int> data_provider::get_planned_works_for_last_day ()
{
	QMap<work, int> result;
	day last_day = get_last_day();
	if (last_day.isValid()) {
		QList<work> works = get_works();
		QList<time_period> periods = get_time_periods_for_current_day();
		QMap<int, int> work_id_to_plan;
		for (QList<work>::const_iterator i = works.begin(); i != works.end(); ++i)
			work_id_to_plan[i->id] = i->plan * 60;
		for (QList<time_period>::const_iterator i = periods.begin(); i != periods.end(); ++i)
			work_id_to_plan[i->work_id] -= i->length_sec();
		for (QList<work>::const_iterator i = works.begin(); i != works.end(); ++i)
			if (work_id_to_plan[i->id] > 0) result[*i] = work_id_to_plan[i->id];
	}
	return result;
}
