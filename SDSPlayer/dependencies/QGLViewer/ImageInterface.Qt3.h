/****************************************************************************

 Copyright (C) 2002-2008 Gilles Debunne. All rights reserved.

 This file is part of the QGLViewer library version 2.3.4.

 http://www.libqglviewer.com - contact@libqglviewer.com

 This file may be used under the terms of the GNU General Public License 
 versions 2.0 or 3.0 as published by the Free Software Foundation and
 appearing in the LICENSE file included in the packaging of this file.
 In addition, as a special exception, Gilles Debunne gives you certain 
 additional rights, described in the file GPL_EXCEPTION in this package.

 libQGLViewer uses dual licensing. Commercial/proprietary software must
 purchase a libQGLViewer Commercial License.

 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*****************************************************************************/

/****************************************************************************
** Form interface generated from reading ui file 'ImageInterface.Qt3.ui'
**
** Created: lun. août 31 22:37:52 2009
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef IMAGEINTERFACE_H
#define IMAGEINTERFACE_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QSpinBox;
class QCheckBox;
class QPushButton;

class ImageInterface : public QDialog
{
    Q_OBJECT

public:
    ImageInterface( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ImageInterface();

    QLabel* textLabel1;
    QSpinBox* imgWidth;
    QLabel* textLabel2;
    QSpinBox* imgHeight;
    QLabel* textLabel3;
    QSpinBox* imgQuality;
    QLabel* textLabel3_2;
    QSpinBox* oversampling;
    QCheckBox* whiteBackground;
    QCheckBox* expandFrustum;
    QPushButton* pushButton1;
    QPushButton* pushButton2;

protected:
    QVBoxLayout* ImageInterfaceLayout;
    QHBoxLayout* layout5;
    QHBoxLayout* layout2;
    QSpacerItem* spacer1;
    QHBoxLayout* layout2_2;
    QSpacerItem* spacer1_2;
    QHBoxLayout* layout4;
    QSpacerItem* spacer3;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;

};

#endif // IMAGEINTERFACE_H
