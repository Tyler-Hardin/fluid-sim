#ifndef NEWDIALOG_HPP
#define NEWDIALOG_HPP

#include <QDialog>
#include <QLineEdit>
#include <QStatusBar>

class NewDialog : public QDialog
{
	QLineEdit* heightEdit;
	QLineEdit* widthEdit;

public:
	NewDialog();

	int height;
	int width;

signals:

public slots:
	void accept() override;
};

#endif // NEWDIALOG_HPP
