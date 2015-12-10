#include "MainWindow.hpp"

#include <QApplication>
#include <QLabel>
#include <QWidget>

#include <QDebug>

int main(int argc, char *argv[ ])
{
	QApplication app(argc, argv);
	auto mainWindow = new MainWindow();
	mainWindow->show();
	return app.exec();
}
