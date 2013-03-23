#include <QtGui>
#include "mainwindow.h"
#include "data_provider.h"

QSettings MainWindow::settings("LzV progs", "Qtimer");

MainWindow::MainWindow ()
{
	// Установки главного окна.
	setWindowTitle(tr("Time management", "main window title"));
	setMinimumSize(600, 400);
	QDesktopWidget * desktop = QApplication::desktop();
	move(desktop->width() / 2 - 300, desktop->height() / 2 - 200);

	create_actions();
	create_main_menu();
	create_status_bar();
	
	// Добавляем виджеты для главной области.
	main_widgets = new QStackedWidget();
	main_widgets_indexes["out of day"] = main_widgets->addWidget(create_widget_out_of_day());
	main_widgets_indexes["in day"] = main_widgets->addWidget(create_widget_in_day());
	setCentralWidget(main_widgets);
	
	connect(data_provider::get_obj(), SIGNAL(database_file_was_changed()), SLOT(set_params_from_new_db()));
	
	// Восстановление настроек. Должно идти после соединения сигналов и слотов.
	QString last_file = settings.value("last_data_file", "").toString();
	if (!last_file.isEmpty()) {
		if (QFile(last_file).exists()) data_provider::get_obj()->set_file(last_file);
		else settings.setValue("last_data_file", "");
	}
}

QWidget * MainWindow::create_widget_out_of_day () {
	QWidget * widg = new QWidget;
	
	
	
	return widg;
}

QWidget * MainWindow::create_widget_in_day () {
	QWidget * widg = new QWidget;
	
	
	
	return widg;
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
}

void MainWindow::create_main_menu ()
{
	QMenuBar * mnuBar = new QMenuBar;

	QMenu * pm = new QMenu(tr("File"), mnuBar);
	pm->addAction(actions["open or create"]);
	pm->addSeparator();
	pm->addAction(actions["exit"]);
	mnuBar->addMenu(pm);

	pm = new QMenu(tr("Help"), mnuBar);
	pm->addAction(actions["about programm"]);
	mnuBar->addMenu(pm);

	setMenuBar(mnuBar);
}

void MainWindow::show_select_file_dialog (const QString * cur_dir) {
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

void MainWindow::set_params_from_new_db () {
	QString new_db_name = (data_provider::get_obj()->isOpen() ? data_provider::get_obj()->get_file_full_name() : "");
	status_bar_label->setText(new_db_name.isEmpty() ? tr("Data file is not open.") : tr("Current data file") + ": " + new_db_name);
	settings.setValue("last_data_file", new_db_name);
}
