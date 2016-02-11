#include "NewDialog.hpp"

#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>

#include <QDebug>

NewDialog::NewDialog() :
	heightEdit(new QLineEdit()),
	widthEdit(new QLineEdit())
{
	heightEdit->setAlignment(Qt::AlignRight);
	widthEdit->setAlignment(Qt::AlignRight);
	QFormLayout* formLayout = new QFormLayout(this);

	auto startButton = new QPushButton("Start");
	startButton->setDefault(true);
	connect(startButton, SIGNAL(clicked()), this, SLOT(accept()));
	auto cancelButton = new QPushButton("Cancel");
	cancelButton->setDefault(false);
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	auto buttonBox = new QHBoxLayout;
	buttonBox->addWidget(startButton);
	buttonBox->addWidget(cancelButton);

	connect(cancelButton, SIGNAL(clicked()), this, SLOT(accept()));
	formLayout->addRow("Height", heightEdit);
	formLayout->addRow("Width", widthEdit);
	formLayout->addRow(buttonBox);
}

/**
 * @brief Validates input.
 */
void NewDialog::accept() {
	bool ok;

	height = heightEdit->text().toInt(&ok);
	if(!ok || height < 10) {
		return;
	}
	width = widthEdit->text().toInt(&ok);
	if(!ok || width < 10) {
		return;
	}

	if(ok) {
		QDialog::accept();
	}
}
