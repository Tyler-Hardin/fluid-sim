#include "MainWindow.hpp"

#include <QAction>
#include <QFileDialog>
#include <QKeySequence>
#include <QMenuBar>

#include <iostream>

void MainWindow::setupMenu(){
	auto menuBar = this->menuBar();
	
	QAction* exitAction = new QAction(tr("&Quit"), this);
	exitAction->setShortcuts(QKeySequence::Quit);
	exitAction->setStatusTip(tr("Exit application"));
	connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
	
	QAction* loadAction = new QAction(tr("&Load"), this);
	loadAction->setShortcuts(QKeySequence::Quit);
	loadAction->setStatusTip(tr("Load fluid file"));
	connect(loadAction, SIGNAL(triggered()), this, SLOT(load()));
	
	auto fileMenu = menuBar->addMenu(tr("&File"));
	fileMenu->addAction(loadAction);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAction);
}

void MainWindow::setupUI(){
	vizWidget = new VizWidget();
	setCentralWidget(vizWidget);
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
	setupMenu();
	setupUI();
	setWindowTitle(tr("Vizualizer"));
}

void MainWindow::load() {
	QString fname = QFileDialog::getOpenFileName(nullptr, tr("Open File"), "./", tr("*.dat"));
	
	std::cout << fname.toUtf8().constData() << std::endl;
	vizWidget->loadFile(fname);
}
