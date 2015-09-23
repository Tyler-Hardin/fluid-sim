#include "MainWindow.hpp"
#include "NewDialog.hpp"

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

	QAction* newAction = new QAction(tr("&New"), this);
	newAction->setStatusTip(tr("Start new simulation"));
	connect(newAction, SIGNAL(triggered()), this, SLOT(newSimulation()));

	editAction = new QAction(tr("&Edit"), this);
	editAction->setStatusTip(tr("Edit (and reset) this simulation"));
	connect(editAction, SIGNAL(triggered()), this, SLOT(editSimulation()));
	editAction->setEnabled(false);

	auto fileMenu = menuBar->addMenu(tr("&File"));
	fileMenu->addAction(newAction);
	fileMenu->addAction(editAction);
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

void MainWindow::editSimulation() {
	NewDialog* newDialog = new NewDialog();
	if(newDialog->exec()) {
		SimState s(newDialog->height, newDialog->width);
		vizWidget->setState(s);
	}
	delete newDialog;
}


void MainWindow::newSimulation() {
	NewDialog* newDialog = new NewDialog();
	if(newDialog->exec()) {
		SimState s(newDialog->height, newDialog->width);
		vizWidget->setState(s);
	}
	delete newDialog;
}
