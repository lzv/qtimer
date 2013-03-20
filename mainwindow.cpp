#include <QtGui>
#include "mainwindow.h"

MainWindow::MainWindow ()
{
	// Установки главного окна.
	setWindowTitle(tr("Time management", "main window title"));
	setMinimumSize(500, 300);
	QDesktopWidget * desktop = QApplication::desktop();
	move(desktop->width() / 2 - 250, desktop->height() / 2 - 150);

	// Добавление действий.
	actions["exit"] = new QAction(this);
	actions["exit"]->setText(tr("Exit"));
	connect(actions["exit"], SIGNAL(triggered()), qApp, SLOT(quit()));
	actions["about programm"] = new QAction(this);
	actions["about programm"]->setText(tr("About"));
	connect(actions["about programm"], SIGNAL(triggered()), SLOT(show_about_message()));
	actions["open or create"] = new QAction(this);
	actions["open or create"]->setText(tr("Select/create file..."));
	connect(actions["open or create"], SIGNAL(triggered()), SLOT(show_select_file_dialog()));

	// Создание меню.
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
		QMessageBox(QMessageBox::Warning, tr("Open/create error"), tr("Can not open/create file.\n") + error, QMessageBox::Ok).exec();
		show_select_file_dialog(& selected_file);
	}
}

void MainWindow::show_about_message () {
	QMessageBox::about(0, tr("About"), tr("Programm for time managment. \nCreated by LzV in 2013."));
}
