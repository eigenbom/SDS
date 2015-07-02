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
** Form interface generated from reading ui file 'VRenderInterface.Qt3.ui'
**
** Created: lun. août 31 22:37:52 2009
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef VRENDERINTERFACE_H
#define VRENDERINTERFACE_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QCheckBox;
class QLabel;
class QComboBox;
class QPushButton;

class VRenderInterface : public QDialog
{
    Q_OBJECT

public:
    VRenderInterface( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~VRenderInterface();

    QCheckBox* includeHidden;
    QCheckBox* cullBackFaces;
    QCheckBox* blackAndWhite;
    QCheckBox* colorBackground;
    QCheckBox* tightenBBox;
    QLabel* sortLabel;
    QComboBox* sortMethod;
    QPushButton* SaveButton;
    QPushButton* CancelButton;

protected:
    QVBoxLayout* VRenderInterfaceLayout;
    QSpacerItem* spacer1;
    QHBoxLayout* layout3;
    QHBoxLayout* Layout4;

protected slots:
    virtual void languageChange();

};

#endif // VRENDERINTERFACE_H
