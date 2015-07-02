/*
 * comments.cpp
 *
 *  Created on: 27/10/2009
 *      Author: ben
 */

#include "comments.h"

Comments::Comments(QWidget* parent)
:QDialog(parent)
{
	mUI.setupUi(this);
	mPTE = findChild<QPlainTextEdit*>("plainTextEdit");
}

QString Comments::getText()
{
	mPTE->toPlainText();
}

void Comments::setText(QString text)
{
	mPTE->setPlainText(text);
}
