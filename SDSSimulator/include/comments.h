/*
 * comments.h
 *
 *  Created on: 27/10/2009
 *      Author: ben
 */

#ifndef COMMENTS_H_
#define COMMENTS_H_

#include <QDialog>
#include <QWidget>
#include <QPlainTextEdit>

#include "ui_comments.h"

class Comments: public QDialog
{
	Q_OBJECT

public:
	Comments(QWidget* parent = NULL);
	QString getText();

public slots:
	void setText(QString text);

private:
	Ui::Comments mUI;
	QPlainTextEdit* mPTE;

};

#endif /* COMMENTS_H_ */
