#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include "VizWidget.hpp"

#include <QMainWindow>

class MainWindow : public QMainWindow{
	Q_OBJECT
	
	VizWidget* vizWidget;
	
	void setupMenu();
	void setupUI();
	
public:
	MainWindow(QWidget* parent = nullptr);
	
private slots:
	void load();
};

#endif // MAIN_WINDOW_HPP
