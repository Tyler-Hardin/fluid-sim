/* Tyler Hardin
 * 4/4/2015
 */
 
#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include "VizWidget.hpp"

#include <QMainWindow>

#include <boost/optional.hpp>

/**
 * @brief Main window of the project. Contains a menu, controls and a display widget.
 */
class MainWindow : public QMainWindow{
	Q_OBJECT

	VizWidget* vizWidget;
	
	/**
	 * Creates menu items.
	 */
	void setupMenu();
	
	/**
	 * Creates widget/layout hierarchy.
	 */
	void setupUI();
	
public:
	MainWindow(QWidget* parent = nullptr);
	
private slots:
	void editSimulation();
	void newSimulation();
};

#endif // MAIN_WINDOW_HPP
