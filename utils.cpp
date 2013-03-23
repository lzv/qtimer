#include "utils.h"

#include <QFile>
#include <QFileInfo>

bool utils::check_can_be_edit (const QString & full_name, QString & error_message) {
	bool result = false;
	if (full_name.isEmpty()) {
		error_message = QObject::tr("File name is empty");
	} else {
		QFileInfo finfo(full_name);
		if (finfo.fileName().isEmpty()) {
			error_message = QObject::tr("File name is directory");
		} else {
			QFile new_file(finfo.absoluteFilePath());
			bool file_exists = new_file.exists();
			if (new_file.open(QIODevice::ReadWrite)) {
				new_file.close();
				if (!file_exists) new_file.remove();
				result = true;
			} else {
				error_message = QObject::tr("Can not open file for write: ") + new_file.errorString();
			}
		}
	}
	return result;
}
