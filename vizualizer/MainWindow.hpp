/* Tyler Hardin
 * 4/4/2015
 */
 
#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include "VizWidget.hpp"

#include <QMainWindow>

/**
 * Main window of the project. Contains menu and VizWidget.
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
	/**
	 * Called when the "Load file" menu option is clicked. Opens file dialog to select file.
	 */
	void load();
};

#endif // MAIN_WINDOW_HPP
