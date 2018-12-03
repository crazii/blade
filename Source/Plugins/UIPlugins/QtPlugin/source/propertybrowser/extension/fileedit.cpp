/****************************************************************************
**
** Copyright (C) 2006 Trolltech ASA. All rights reserved.
**
** This file is part of the documentation of Qt. It was originally
** published as part of Qt Quarterly.
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation or under the
** terms of the Qt Commercial License Agreement. The respective license
** texts for these are provided with the open source and commercial
** editions of Qt.
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include <QtPluginPCH.h>
#include "fileedit.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QFileDialog>
#include <QFocusEvent>
#include "filepathmanager.h"

FileEdit::FileEdit(QWidget *parent)
    : QWidget(parent)
	, theType(FT_FILE)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    theLineEdit = new QLineEdit(this);
    theLineEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    theButton = new QToolButton(this);
    theButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
    theButton->setText(QLatin1String("..."));
    layout->addWidget(theLineEdit);
    layout->addWidget(theButton);
    setFocusProxy(theLineEdit);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled);
    connect(theLineEdit, SIGNAL(textEdited(const QString &)),
                this, SIGNAL(filePathChanged(const QString &)));
    connect(theButton, SIGNAL(clicked()),
                this, SLOT(buttonClicked()));
}

void FileEdit::buttonClicked()
{
	if(theType == FT_FILE)
	{
		QFileDialog dialog(this);

		dialog.setWindowTitle(tr("Choose a file"));
		dialog.setOption(QFileDialog::DontUseNativeDialog, true);
		dialog.setAcceptMode(QFileDialog::AcceptOpen);
		dialog.setFileMode(QFileDialog::ExistingFile);
		dialog.setNameFilter(theFilter);
		dialog.setDirectory(theLineEdit->text());
		dialog.selectFile(theLineEdit->text());

		if(dialog.exec() && dialog.selectedFiles().count() > 0)
		{
			QString filePath = dialog.selectedFiles()[0];
			if (filePath.isNull())
				return;
			theLineEdit->setText(filePath);
			emit filePathChanged(filePath);
		}
	}
	else if(theType == FT_FOLDER)
	{
		QFileDialog dialog(this);

		dialog.setWindowTitle(tr("Choose a folder"));
		dialog.setOption(QFileDialog::DontUseNativeDialog, true);
		dialog.setAcceptMode(QFileDialog::AcceptOpen);
		dialog.setFileMode(QFileDialog::DirectoryOnly);
		dialog.setNameFilter(theFilter);
		dialog.setDirectory(theLineEdit->text());

		if(dialog.exec() && dialog.selectedFiles().count() > 0)
		{
			QString filePath = dialog.selectedFiles()[0];
			if (filePath.isNull())
				return;
			theLineEdit->setText(filePath);
			emit filePathChanged(filePath);
		}
	}
	else
	{
		QString filePath;
		bool done = false;
		emit callbackInvoked(this, filePath, done);
		if(done)
		{
			theLineEdit->setText(filePath);
			emit filePathChanged(filePath);
		}
	}
}

void FileEdit::focusInEvent(QFocusEvent *e)
{
    if (e->reason() == Qt::TabFocusReason || e->reason() == Qt::BacktabFocusReason) {
        theLineEdit->selectAll();
    }
    QWidget::focusInEvent(e);
	theLineEdit->setFocus();
}

void FileEdit::focusOutEvent(QFocusEvent *e)
{
    theLineEdit->event(e);
    QWidget::focusOutEvent(e);
}

void FileEdit::keyPressEvent(QKeyEvent *e)
{
    theLineEdit->event(e);
}

void FileEdit::keyReleaseEvent(QKeyEvent *e)
{
    theLineEdit->event(e);
}
