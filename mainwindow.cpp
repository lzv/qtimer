#ifdef HAVE_QT5
	#include <QtWidgets>
#else
	#include <QtGui>
#endif
#include "mainwindow.h"
#include "data_provider.h"

QSettings MainWindow::settings("LzV progs", "Qtimer");

MainWindow::MainWindow ()
	: need_reaction_for_signal_cellChanged(true), 
	  for_add_work_was_modif_name(false), for_add_work_was_modif_plan(false)
{
	// Установки главного окна.
	setWindowTitle(tr("Time management", "main window title"));
	setMinimumSize(600, 400);
	QDesktopWidget * desktop = QApplication::desktop();
	move(desktop->width() / 2 - 300, desktop->height() / 2 - 200);

	create_actions();
	create_main_menu();
	create_status_bar();
	create_works_list_window();
	
	// Добавляем виджеты для главной области. Первый добавленный будет по умолчанию.
	main_widgets = new QStackedWidget();
	main_widgets_indexes["db not open"] = main_widgets->addWidget(create_widget_db_not_open());
	main_widgets_indexes["out of day"] = main_widgets->addWidget(create_widget_out_of_day());
	main_widgets_indexes["in day"] = main_widgets->addWidget(create_widget_in_day());
	setCentralWidget(main_widgets);
	
	connect(data_provider::get_obj(), SIGNAL(database_file_was_changed()), SLOT(set_params_from_new_db()));
	connect(data_provider::get_obj(), SIGNAL(out_from_day()), SLOT(change_main_widget()));
	connect(data_provider::get_obj(), SIGNAL(works_updated()), SLOT(update_works_list_window()));
	connect(data_provider::get_obj(), SIGNAL(works_updated()), SLOT(update_widget_in_day()));
	connect(& delete_work_buttons_map, SIGNAL(mapped(int)), SLOT(delete_work(int)));
	connect(works_list_table, SIGNAL(cellChanged(int,int)), SLOT(modif_work_from_table_widget(int,int)));
	
	// Восстановление настроек. Должно идти после соединения сигналов и слотов.
	QString last_file = settings.value("last_data_file", "").toString();
	if (!last_file.isEmpty()) {
		if (QFile(last_file).exists()) data_provider::get_obj()->set_file(last_file);
		else settings.setValue("last_data_file", "");
	}
}

int MainWindow::get_need_main_widget_index ()
{
	int need_index = main_widgets_indexes["db not open"];
	if (data_provider::get_obj()->isOpen()) {
		need_index = main_widgets_indexes["out of day"];
		day last_day = data_provider::get_obj()->get_last_day();
		if (last_day.isValid() and last_day.isIn()) need_index = main_widgets_indexes["in day"];
	}
	return need_index;
}

void MainWindow::create_works_list_window ()
{
	QLabel * message = new QLabel(tr("You can add, delete or edit works. For edit work make double click on cell. For add work insert data in the lowest row. For delete work click delete button. <b>Warning:</b> you can not restore work after deletion.<br /> Note: plan in minutes."));
	message->setWordWrap(true);
	works_list_table = new QTableWidget();
	works_list_table->setColumnCount(3);
	works_list_table->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Plan") << "");
	works_list_table->setColumnWidth(0, 495);
	works_list_table->setColumnWidth(1, 45);
	QPushButton * close_button = new QPushButton(tr("Close"));
	connect(close_button, SIGNAL(clicked()), & works_list_window, SLOT(hide()));
	QVBoxLayout * lay = new QVBoxLayout;
	lay->addWidget(message);
	lay->addWidget(works_list_table);
	lay->addWidget(close_button);
	works_list_window.setLayout(lay);
	works_list_window.resize(700, 300);
}

void MainWindow::update_works_list_window () {
	bool need_reaction_value = need_reaction_for_signal_cellChanged;
	need_reaction_for_signal_cellChanged = false;
	QList<work> works = data_provider::get_obj()->get_works();
	int row_count = works_list_table->rowCount();
	for (int i = 0; i < row_count; ++i) works_list_table->removeRow(0);
	row_count = 0;
	for (QList<work>::const_iterator i = works.begin(); i != works.end(); ++i) {
		works_list_table->insertRow(row_count);
		QTableWidgetItem * id_item = new QTableWidgetItem;
		QTableWidgetItem * name_item = new QTableWidgetItem;
		QTableWidgetItem * plan_item = new QTableWidgetItem;
		id_item->setData(Qt::DisplayRole, i->id);
		name_item->setData(Qt::DisplayRole, i->name);
		plan_item->setData(Qt::DisplayRole, i->plan);
		QPushButton * delete_button = new QPushButton(tr("Delete"));
		delete_work_buttons_map.setMapping(delete_button, i->id);
		connect(delete_button, SIGNAL(clicked()), & delete_work_buttons_map, SLOT(map()));
		works_list_table->setVerticalHeaderItem(row_count, id_item);
		works_list_table->setItem(row_count, 0, name_item);
		works_list_table->setItem(row_count, 1, plan_item);
		works_list_table->setCellWidget(row_count, 2, delete_button);
		++row_count;
	}
	works_list_table->insertRow(row_count);
	QTableWidgetItem * empty_item1 = new QTableWidgetItem;
	empty_item1->setData(Qt::DisplayRole, QString(""));
	empty_item1->setFlags(empty_item1->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEditable);
	QTableWidgetItem * empty_item2 = new QTableWidgetItem(*empty_item1);
	QTableWidgetItem * name_item = new QTableWidgetItem;
	QTableWidgetItem * plan_item = new QTableWidgetItem;
	name_item->setData(Qt::DisplayRole, QString(""));
	plan_item->setData(Qt::DisplayRole, int(0));
	works_list_table->setVerticalHeaderItem(row_count, empty_item1);
	works_list_table->setItem(row_count, 0, name_item);
	works_list_table->setItem(row_count, 1, plan_item);
	works_list_table->setItem(row_count, 2, empty_item2);
	for_add_work_was_modif_name = false;
	for_add_work_was_modif_plan = false;
	need_reaction_for_signal_cellChanged = need_reaction_value;
}

QWidget * MainWindow::create_widget_db_not_open () {
	QWidget * widg = new QWidget;
	QLabel * mess_for_user = new QLabel(tr("Data file is not open. Please open or create it."));
	mess_for_user->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
	QPushButton * button = new QPushButton(actions["open or create"]->text());
	button->setToolTip(actions["open or create"]->toolTip());
	button->setStatusTip(actions["open or create"]->statusTip());
	connect(button, SIGNAL(clicked()), actions["open or create"], SIGNAL(triggered()));
	QHBoxLayout * hlay = new QHBoxLayout;
	hlay->addStretch(10);
	hlay->addWidget(button);
	hlay->addStretch(10);
	QGridLayout * lay = get_stretch_QGridLayout(0, 3, 0, 2, 10);
	lay->addWidget(mess_for_user, 1, 1);
	lay->addLayout(hlay, 2, 1);
	widg->setLayout(lay);
	return widg;
}

QWidget * MainWindow::create_widget_out_of_day () {
	QWidget * widg = new QWidget;
	out_of_day_message = new QLabel;
	out_of_day_message->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
	out_of_day_message->setWordWrap(true);
	out_of_day_add_day_edit = new QDateTimeEdit;
	out_of_day_add_day_edit->setDisplayFormat(data_provider::DATETIME_DB_FORMAT);
	out_of_day_update_day_edit = new QDateTimeEdit;
	out_of_day_update_day_edit->setDisplayFormat(data_provider::DATETIME_DB_FORMAT);
	out_of_day_add_day_button = new QPushButton(tr("Add new day"));
	connect(out_of_day_add_day_button, SIGNAL(clicked()), SLOT(add_day_button_clicked()));
	out_of_day_update_day_button = new QPushButton(tr("Update end of day"));
	connect(out_of_day_update_day_button, SIGNAL(clicked()), SLOT(update_day_button_clicked()));
	out_of_day_add_hint = new QLabel(tr("Please enter end of new day:"));
	out_of_day_add_hint->setWordWrap(true);
	out_of_day_add_hint->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	out_of_day_update_hint = new QLabel(tr("Please enter new end of the last day:"));
	out_of_day_update_hint->setWordWrap(true);
	out_of_day_update_hint->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	QGridLayout * lay = get_stretch_QGridLayout(0, 5, 0, 3, 10);
	lay->addWidget(out_of_day_message, 1, 1, 1, 2);
	lay->addWidget(out_of_day_update_hint, 2, 1);
	lay->addWidget(out_of_day_update_day_edit, 3, 1);
	lay->addWidget(out_of_day_update_day_button, 4, 1);
	lay->addWidget(out_of_day_add_hint, 2, 2);
	lay->addWidget(out_of_day_add_day_edit, 3, 2);
	lay->addWidget(out_of_day_add_day_button, 4, 2);
	widg->setLayout(lay);
	update_widget_out_of_day();
	return widg;
}

void MainWindow::update_widget_out_of_day () {
	bool last_day_exists = data_provider::get_obj()->get_last_day().isValid();
	out_of_day_message->setText(last_day_exists ? tr("The last day was over. You can update end of it or add a new day. New day will begin from now.") : tr("No days in data file. You can add a new day. It will begin from now."));
	QDateTime now = QDateTime::currentDateTime();
	out_of_day_add_day_edit->setMinimumDateTime(now);
	out_of_day_add_day_edit->setDateTime(now.addSecs(3600 * 8));
	out_of_day_update_day_edit->setMinimumDateTime(now);
	out_of_day_update_day_edit->setDateTime(now.addSecs(3600));
	// Обновлять последний день можно, только если он существует.
	out_of_day_update_hint->setEnabled(last_day_exists);
	out_of_day_update_day_edit->setEnabled(last_day_exists);
	out_of_day_update_day_button->setEnabled(last_day_exists);
}

QWidget * MainWindow::create_widget_in_day () {
	QWidget * widg = new QWidget;
	in_day_line_of_day = new dayInLine;
	works_dropdown_list = new QComboBox;
	start_stop_button = new QPushButton(tr("Start"));
	connect(start_stop_button, SIGNAL(clicked()), SLOT(start_stop_button_clicked()));
	QHBoxLayout * hlay_ddlist = new QHBoxLayout;
	hlay_ddlist->addWidget(works_dropdown_list, 1);
	hlay_ddlist->addWidget(start_stop_button, 0);
	QLabel * message_for_planned_table = new QLabel(tr("Planned works for today:"), this);
	message_for_planned_table->setWordWrap(true);
	planned_works_table = new QTableWidget;
	planned_works_table->setColumnCount(2);
	planned_works_table->setHorizontalHeaderLabels(QStringList() << tr("Work name") << tr("Time left"));
	planned_works_table->setColumnWidth(0, 363);
	planned_works_table->setColumnWidth(1, 100);
	QGridLayout * lay = get_stretch_QGridLayout(0, 5, 0, 2, 1, 10);
	lay->addLayout(hlay_ddlist, 1, 1);
	lay->setRowStretch(1, 0);
	lay->addWidget(in_day_line_of_day, 2, 1);
	lay->addWidget(message_for_planned_table, 3, 1);
	lay->setRowStretch(3, 0);
	lay->addWidget(planned_works_table, 4, 1);
	widg->setLayout(lay);
	return widg;
}

void MainWindow::update_widget_in_day () {
	works_dropdown_list->clear();
	QList<work> works = data_provider::get_obj()->get_works();
	time_period last_tp = data_provider::get_obj()->get_last_time_period();
	bool now_work_tracking = last_tp.isValid() and last_tp.isOpened();
	int current_work_id = last_tp.isValid() ? last_tp.work_id : 0;
	int works_count = works.count();
	for (int i = 0; i < works_count; ++i) {
		works_dropdown_list->addItem(works[i].name, works[i].id);
		if (works[i].id == current_work_id) works_dropdown_list->setCurrentIndex(i);
	}
	works_dropdown_list->setEnabled(!now_work_tracking);
	start_stop_button->setText(now_work_tracking ? tr("Stop") : tr("Start"));
	QMap<work, int> planned_works = data_provider::get_obj()->get_planned_works_for_last_day();
	int row_count = planned_works_table->rowCount();
	for (int i = 0; i < row_count; ++i) planned_works_table->removeRow(0);
	row_count = 0;
	time_count tc_obj;
	for (QMap<work, int>::const_iterator i = planned_works.begin(); i != planned_works.end(); ++i) {
		planned_works_table->insertRow(row_count);
		QTableWidgetItem * name_item = new QTableWidgetItem;
		QTableWidgetItem * plan_item = new QTableWidgetItem;
		name_item->setFlags(name_item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEditable);
		plan_item->setFlags(plan_item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEditable);
		name_item->setData(Qt::DisplayRole, i.key().name);
		name_item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		tc_obj.set_from_seconds(i.value());
		plan_item->setData(Qt::DisplayRole, tc_obj.get_string_for_print('c'));
		plan_item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		planned_works_table->setItem(row_count, 0, name_item);
		planned_works_table->setItem(row_count, 1, plan_item);
		++row_count;
	}
	in_day_line_of_day->update();
}

void MainWindow::add_day_button_clicked ()
{
	day added_day(0, QDateTime::currentDateTime(), out_of_day_add_day_edit->dateTime());
	QString error("");
	if (data_provider::get_obj()->add_day(added_day, & error)) change_main_widget();
	else show_warning_message(tr("Add day error"), error);
}

void MainWindow::update_day_button_clicked ()
{
	day last_day = data_provider::get_obj()->get_last_day();
	if (last_day.isValid()) {
		last_day.end = out_of_day_update_day_edit->dateTime();
		QString error("");
		if (data_provider::get_obj()->update_day(last_day, & error)) change_main_widget();
		else show_warning_message(tr("Update day error"), error);
	} else {
		show_warning_message(tr("Update day error"), tr("Update day error") + " - " + tr("the last day is not exists."));
	}
}

QGridLayout * MainWindow::get_stretch_QGridLayout (int row1, int row2, int col1, int col2, int stretch, int other_stretch)
{
	QGridLayout * lay = new QGridLayout();
	lay->setRowStretch(row1, stretch);
	lay->setRowStretch(row2, stretch);
	lay->setColumnStretch(col1, stretch);
	lay->setColumnStretch(col2, stretch);
	if (other_stretch > 0) {
		int i, min_row = qMin(row1, row2), max_row = qMax(row1, row2), min_col = qMin(col1, col2), max_col = qMax(col1, col2);
		for (i = min_row + 1; i < max_row; ++i) lay->setRowStretch(i, other_stretch);
		for (i = min_col + 1; i < max_col; ++i) lay->setColumnStretch(i, other_stretch);
	}
	return lay;
}

void MainWindow::create_status_bar ()
{
	QStatusBar * status_bar = new QStatusBar;
	status_bar_label = new QLabel(tr("Data file is not open."));
	status_bar->addWidget(status_bar_label);
	setStatusBar(status_bar);
}

void MainWindow::create_actions ()
{
	actions["exit"] = new QAction(this);
	actions["exit"]->setText(tr("Exit"));
	actions["exit"]->setToolTip(tr("Exit from programm, without stop current work."));
	actions["exit"]->setStatusTip(tr("Exit from programm, without stop current work."));
	connect(actions["exit"], SIGNAL(triggered()), qApp, SLOT(quit()));
	actions["about programm"] = new QAction(this);
	actions["about programm"]->setText(tr("About"));
	actions["about programm"]->setToolTip(tr("Show information about programm."));
	actions["about programm"]->setStatusTip(tr("Show information about programm."));
	connect(actions["about programm"], SIGNAL(triggered()), SLOT(show_about_message()));
	actions["open or create"] = new QAction(this);
	actions["open or create"]->setText(tr("Select/create file..."));
	actions["open or create"]->setToolTip(tr("Open or create new data file, without stop current work."));
	actions["open or create"]->setStatusTip(tr("Open or create new data file, without stop current work."));
	connect(actions["open or create"], SIGNAL(triggered()), SLOT(show_select_file_dialog()));
	actions["show works list"] = new QAction(this);
	actions["show works list"]->setText(tr("Works list..."));
	actions["show works list"]->setToolTip(tr("View, edit, add or delete works."));
	actions["show works list"]->setStatusTip(tr("View, edit, add or delete works."));
	connect(actions["show works list"], SIGNAL(triggered()), SLOT(show_works_list_window()));
}

void MainWindow::create_main_menu ()
{
	QMenuBar * mnuBar = new QMenuBar;

	myMenu * pm = new myMenu(tr("File"), mnuBar);
	pm->addAction(actions["open or create"]);
	pm->addSeparator();
	pm->addAction(actions["exit"]);
	mnuBar->addMenu(pm);
	
	pm = new myMenu(tr("Works"), mnuBar);
	pm->addAction(actions["show works list"]);
	mnuBar->addMenu(pm);
	
	pm = new myMenu(tr("Help"), mnuBar);
	pm->addAction(actions["about programm"]);
	mnuBar->addMenu(pm);

	setMenuBar(mnuBar);
}

void MainWindow::show_select_file_dialog (const QString * cur_dir)
{
	QString opened_file = data_provider::get_obj()->get_file_full_name();
	QString dir = cur_dir ? *cur_dir : (opened_file.isEmpty() ? QDir::homePath() : opened_file);
	QString selected_file = QFileDialog::getSaveFileName(this, tr("Select/create file..."), dir, "", 0, QFileDialog::DontConfirmOverwrite);
	QString error("");
	if (!selected_file.isEmpty() and !data_provider::get_obj()->set_file(selected_file, & error)) {
		show_warning_message(tr("Open/create error"), tr("Can not open/create file.") + "\n" + error);
		show_select_file_dialog(& selected_file);
	}
}

void MainWindow::show_about_message () {
	QMessageBox::about(0, tr("About"), tr("Programm for time managment. \nCreated by LzV in 2013."));
}

void MainWindow::show_works_list_window ()
{
	QPoint main_window_center = rect().center(), main_pos = pos();
	main_pos.rx() += main_window_center.x() - (works_list_window.width() / 2);
	main_pos.ry() += main_window_center.y() - (works_list_window.height() / 2);
	works_list_window.move(main_pos);
	update_works_list_window();
	works_list_window.show();
}

void MainWindow::change_main_widget ()
{
	int need_index = get_need_main_widget_index();
	if (need_index == main_widgets_indexes["in day"]) {
		if (!timer.isActive()) timer.start(1000, this);
		update_widget_in_day();
	} else {
		if (timer.isActive()) timer.stop();
		if (need_index == main_widgets_indexes["out of day"]) update_widget_out_of_day();
	}
	main_widgets->setCurrentIndex(need_index);
}

void MainWindow::set_params_from_new_db ()
{
	QString new_db_name = (data_provider::get_obj()->isOpen() ? data_provider::get_obj()->get_file_full_name() : "");
	status_bar_label->setText(new_db_name.isEmpty() ? tr("Data file is not open.") : tr("Current data file") + ": " + new_db_name);
	settings.setValue("last_data_file", new_db_name);
	change_main_widget();
}

void MainWindow::show_warning_message (const QString & title, const QString & message) {
	QMessageBox(QMessageBox::Warning, title, message, QMessageBox::Ok).exec();
}

void MainWindow::delete_work (int id)
{
	QString error("");
	QMessageBox question(QMessageBox::Warning, tr("Are you sure?"), tr("You can not restore work after deletion. Do you really want to delete?"), QMessageBox::Yes | QMessageBox::No);
	if (question.exec() == QMessageBox::Yes and !data_provider::get_obj()->delete_work(id, & error)) {
		show_warning_message(tr("Delete work error"), error);
		update_works_list_window();
	}
}

void MainWindow::modif_work_from_table_widget (int row, int col)
{
	if (need_reaction_for_signal_cellChanged) {
		int last_row = works_list_table->rowCount() - 1;
		QString new_name = works_list_table->item(row, 0)->text();
		int new_plan = works_list_table->item(row, 1)->text().toInt();
		work element(0, new_name, new_plan);
		QString error("");
		if (row == last_row) {
			if (col == 0) for_add_work_was_modif_name = true;
			if (col == 1) for_add_work_was_modif_plan = true;
			if (
					for_add_work_was_modif_name 
					and for_add_work_was_modif_plan 
					and !data_provider::get_obj()->add_work(element, & error)
				) {
				show_warning_message(tr("Add work error"), error);
				update_works_list_window();
			}
		} else {
			element.id = works_list_table->verticalHeaderItem(row)->text().toInt();
			if (!data_provider::get_obj()->update_work(element, & error)) {
				show_warning_message(tr("Update work error"), error);
				update_works_list_window();
			}
		}
	}
}

void MainWindow::timerEvent (QTimerEvent * ev)
{
	if (ev->timerId() == timer.timerId()) {
		day last_day = data_provider::get_obj()->get_last_day();
		if (last_day.end < QDateTime::currentDateTime()) {
			data_provider::get_obj()->check_last_time_period_for_close();
			change_main_widget();
		} else {
			time_period last_tp = data_provider::get_obj()->get_last_time_period();
			if (last_tp.isValid() and last_tp.isOpened()) update_widget_in_day();
			else in_day_line_of_day->update();
		}
	} else {
		QMainWindow::timerEvent(ev);
	}
}

void MainWindow::start_stop_button_clicked ()
{
	if (works_dropdown_list->count() > 0) {
		time_period last_tp = data_provider::get_obj()->get_last_time_period();
		QString error;
		if (last_tp.isValid() and last_tp.isOpened()) {
			last_tp.end = QDateTime::currentDateTime();
			if (data_provider::get_obj()->update_time_period(last_tp, & error)) update_widget_in_day();
			else show_warning_message(tr("Work stop error"), error);
		} else {
			int work_id = works_dropdown_list->itemData(works_dropdown_list->currentIndex()).toInt();
			time_period new_tp(0, work_id, QDateTime::currentDateTime());
			if (data_provider::get_obj()->add_time_period(new_tp, & error)) update_widget_in_day();
			else show_warning_message(tr("Work start error"), error);
		}
	}
}
