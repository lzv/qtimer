#include <QtGui>
#include "mainwindow.h"
#include "data_provider.h"

QSettings MainWindow::settings("LzV progs", "Qtimer");

MainWindow::MainWindow ()
	: works_list_window_width(500), works_list_window_height(200)
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
	connect(data_provider::get_obj(), SIGNAL(change_in_out_day()), SLOT(change_main_widget()));
	
	// Восстановление настроек. Должно идти после соединения сигналов и слотов.
	QString last_file = settings.value("last_data_file", "").toString();
	if (!last_file.isEmpty()) {
		if (QFile(last_file).exists()) data_provider::get_obj()->set_file(last_file);
		else settings.setValue("last_data_file", "");
	}
}

int MainWindow::get_need_main_widget_index () {
	int need_index = main_widgets_indexes["db not open"];
	if (data_provider::get_obj()->isOpen()) {
		need_index = main_widgets_indexes["out of day"];
		day last_day = data_provider::get_obj()->get_last_day();
		if (last_day.isValid() and last_day.isIn()) need_index = main_widgets_indexes["in day"];
	}
	return need_index;
}

void MainWindow::create_works_list_window () {
	QLabel * message = new QLabel(tr("You can add, delete or edit works."));
	message->setWordWrap(true);
	works_list_table = new QTableWidget();
	works_list_table->setColumnCount(3);
	QStringList headers;
	headers << tr("ID") << tr("Name") << tr("Plan");
	works_list_table->setHorizontalHeaderLabels(headers);
	QPushButton * close_button = new QPushButton(tr("Close"));
	connect(close_button, SIGNAL(clicked()), & works_list_window, SLOT(hide()));
	QVBoxLayout * lay = new QVBoxLayout;
	lay->addWidget(message);
	lay->addWidget(works_list_table);
	lay->addWidget(close_button);
	works_list_window.setLayout(lay);
	works_list_window.resize(works_list_window_width, works_list_window_height);
}

void MainWindow::update_works_list_window () {
	
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
	
	QGridLayout * lay = get_stretch_QGridLayout(0, 2, 0, 2, 1, 10);
	lay->addWidget(in_day_line_of_day, 1, 1);
	
	widg->setLayout(lay);
	return widg;
}

void MainWindow::update_widget_in_day () {
	
}

void MainWindow::add_day_button_clicked () {
	day added_day(0, QDateTime::currentDateTime(), out_of_day_add_day_edit->dateTime());
	QString error("");
	if (data_provider::get_obj()->add_day(added_day, & error)) change_main_widget();
	else QMessageBox(QMessageBox::Warning, tr("Add day error"), error, QMessageBox::Ok).exec();
}

void MainWindow::update_day_button_clicked () {
	day last_day = data_provider::get_obj()->get_last_day();
	if (last_day.isValid()) {
		last_day.end = out_of_day_update_day_edit->dateTime();
		QString error("");
		if (data_provider::get_obj()->update_day(last_day, & error)) change_main_widget();
		else QMessageBox(QMessageBox::Warning, tr("Update day error"), error, QMessageBox::Ok).exec();
	} else {
		QMessageBox(QMessageBox::Warning, tr("Update day error"), tr("Update day error") + " - " + tr("the last day is not exists."), QMessageBox::Ok).exec();
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

void MainWindow::create_status_bar () {
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
	QString dir = cur_dir ? *cur_dir : (opened_file.isEmpty() ? "." : opened_file);
	QString selected_file = QFileDialog::getSaveFileName(this, tr("Select/create file..."), dir, "", 0, QFileDialog::DontConfirmOverwrite);
	QString error("");
	if (!selected_file.isEmpty() and !data_provider::get_obj()->set_file(selected_file, & error)) {
		QMessageBox(QMessageBox::Warning, tr("Open/create error"), tr("Can not open/create file.") + "\n" + error, QMessageBox::Ok).exec();
		show_select_file_dialog(& selected_file);
	}
}

void MainWindow::show_about_message () {
	QMessageBox::about(0, tr("About"), tr("Programm for time managment. \nCreated by LzV in 2013."));
}

void MainWindow::show_works_list_window () {
	QPoint main_window_center = rect().center(), main_pos = pos();
	main_pos.rx() += main_window_center.x() - (works_list_window.width() / 2);
	main_pos.ry() += main_window_center.y() - (works_list_window.height() / 2);
	works_list_window.move(main_pos);
	update_works_list_window();
	works_list_window.show();
}

void MainWindow::change_main_widget () {
	int need_index = get_need_main_widget_index();
	if (need_index == main_widgets_indexes["out of day"]) update_widget_out_of_day();
	else if (need_index == main_widgets_indexes["in day"]) update_widget_in_day();
	main_widgets->setCurrentIndex(need_index);
}

void MainWindow::set_params_from_new_db () {
	QString new_db_name = (data_provider::get_obj()->isOpen() ? data_provider::get_obj()->get_file_full_name() : "");
	status_bar_label->setText(new_db_name.isEmpty() ? tr("Data file is not open.") : tr("Current data file") + ": " + new_db_name);
	settings.setValue("last_data_file", new_db_name);
	change_main_widget();
}
