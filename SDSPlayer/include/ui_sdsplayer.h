/********************************************************************************
** Form generated from reading ui file 'sdsplayer.ui'
**
** Created: Wed 16. Sep 14:37:04 2009
**      by: Qt User Interface Compiler version 4.5.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_SDSPLAYER_H
#define UI_SDSPLAYER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QScrollArea>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QTextEdit>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "timelinewidget.h"
#include "viewer.h"

QT_BEGIN_NAMESPACE

class Ui_SDSPlayerClass
{
public:
    QAction *action_Open;
    QAction *actionQuit;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_8;
    Viewer *viewer;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_3;
    QTextEdit *textEditConsole;
    QHBoxLayout *horizontalLayout_5;
    QComboBox *comboBoxViewMode;
    QComboBox *comboBoxEditMode;
    QToolButton *toolButtonFitToWindow;
    QComboBox *comboBoxSelectMode;
    QPushButton *pushButtonTetTopograph;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *horizontalLayout_6;
    QLineEdit *lineEdit;
    QHBoxLayout *horizontalLayout;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_4;
    QToolButton *toolButtonRewind;
    QToolButton *toolButtonStop;
    QToolButton *toolButtonPlay;
    QSlider *sliderPlaybackRate;
    TimeLineWidget *timeline;
    QHBoxLayout *horizontalLayout_3;
    QLabel *labelFilename;
    QLabel *labelVersion;
    QLabel *labelDate;
    QSpacerItem *horizontalSpacer;
    QMenuBar *menubar;
    QMenu *menuFile;

    void setupUi(QMainWindow *SDSPlayerClass)
    {
        if (SDSPlayerClass->objectName().isEmpty())
            SDSPlayerClass->setObjectName(QString::fromUtf8("SDSPlayerClass"));
        SDSPlayerClass->resize(790, 678);
        SDSPlayerClass->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        action_Open = new QAction(SDSPlayerClass);
        action_Open->setObjectName(QString::fromUtf8("action_Open"));
        actionQuit = new QAction(SDSPlayerClass);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        centralwidget = new QWidget(SDSPlayerClass);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout_2 = new QVBoxLayout(centralwidget);
        verticalLayout_2->setSpacing(2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(-1, 2, -1, 2);
        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        horizontalLayout_8->setContentsMargins(-1, 5, -1, -1);
        viewer = new Viewer(centralwidget);
        viewer->setObjectName(QString::fromUtf8("viewer"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(viewer->sizePolicy().hasHeightForWidth());
        viewer->setSizePolicy(sizePolicy);

        horizontalLayout_8->addWidget(viewer);

        scrollArea = new QScrollArea(centralwidget);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 380, 523));
        verticalLayout_3 = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        textEditConsole = new QTextEdit(scrollAreaWidgetContents);
        textEditConsole->setObjectName(QString::fromUtf8("textEditConsole"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(textEditConsole->sizePolicy().hasHeightForWidth());
        textEditConsole->setSizePolicy(sizePolicy1);

        verticalLayout_3->addWidget(textEditConsole);

        scrollArea->setWidget(scrollAreaWidgetContents);

        horizontalLayout_8->addWidget(scrollArea);


        verticalLayout_2->addLayout(horizontalLayout_8);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        comboBoxViewMode = new QComboBox(centralwidget);
        QIcon icon;
        icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/icons/wireframe.png")), QIcon::Normal, QIcon::Off);
        comboBoxViewMode->addItem(icon, QString());
        QIcon icon1;
        icon1.addPixmap(QPixmap(QString::fromUtf8(":/icons/icons/surface.png")), QIcon::Normal, QIcon::Off);
        comboBoxViewMode->addItem(icon1, QString());
        QIcon icon2;
        icon2.addPixmap(QPixmap(QString::fromUtf8(":/icons/icons/smooth.png")), QIcon::Normal, QIcon::Off);
        comboBoxViewMode->addItem(icon2, QString());
        QIcon icon3;
        icon3.addPixmap(QPixmap(QString::fromUtf8(":/icons/icons/cells.png")), QIcon::Normal, QIcon::Off);
        comboBoxViewMode->addItem(icon3, QString());
        comboBoxViewMode->setObjectName(QString::fromUtf8("comboBoxViewMode"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(comboBoxViewMode->sizePolicy().hasHeightForWidth());
        comboBoxViewMode->setSizePolicy(sizePolicy2);

        horizontalLayout_5->addWidget(comboBoxViewMode);

        comboBoxEditMode = new QComboBox(centralwidget);
        QIcon icon4;
        icon4.addPixmap(QPixmap(QString::fromUtf8(":/icons/icons/eye.png")), QIcon::Normal, QIcon::Off);
        comboBoxEditMode->addItem(icon4, QString());
        QIcon icon5;
        icon5.addPixmap(QPixmap(QString::fromUtf8(":/icons/icons/pencil.png")), QIcon::Normal, QIcon::Off);
        comboBoxEditMode->addItem(icon5, QString());
        comboBoxEditMode->setObjectName(QString::fromUtf8("comboBoxEditMode"));
        sizePolicy2.setHeightForWidth(comboBoxEditMode->sizePolicy().hasHeightForWidth());
        comboBoxEditMode->setSizePolicy(sizePolicy2);

        horizontalLayout_5->addWidget(comboBoxEditMode);

        toolButtonFitToWindow = new QToolButton(centralwidget);
        toolButtonFitToWindow->setObjectName(QString::fromUtf8("toolButtonFitToWindow"));
        QIcon icon6;
        icon6.addPixmap(QPixmap(QString::fromUtf8(":/icons/icons/magnifier.png")), QIcon::Normal, QIcon::Off);
        toolButtonFitToWindow->setIcon(icon6);

        horizontalLayout_5->addWidget(toolButtonFitToWindow);

        comboBoxSelectMode = new QComboBox(centralwidget);
        comboBoxSelectMode->setObjectName(QString::fromUtf8("comboBoxSelectMode"));

        horizontalLayout_5->addWidget(comboBoxSelectMode);

        pushButtonTetTopograph = new QPushButton(centralwidget);
        pushButtonTetTopograph->setObjectName(QString::fromUtf8("pushButtonTetTopograph"));

        horizontalLayout_5->addWidget(pushButtonTetTopograph);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_2);


        verticalLayout_2->addLayout(horizontalLayout_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        horizontalLayout_6->setContentsMargins(-1, 4, -1, 4);
        lineEdit = new QLineEdit(centralwidget);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));

        horizontalLayout_6->addWidget(lineEdit);


        verticalLayout_2->addLayout(horizontalLayout_6);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(1);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        toolButtonRewind = new QToolButton(centralwidget);
        toolButtonRewind->setObjectName(QString::fromUtf8("toolButtonRewind"));
        QIcon icon7;
        icon7.addPixmap(QPixmap(QString::fromUtf8(":/icons/icons/control_start.png")), QIcon::Normal, QIcon::Off);
        toolButtonRewind->setIcon(icon7);

        horizontalLayout_4->addWidget(toolButtonRewind);

        toolButtonStop = new QToolButton(centralwidget);
        toolButtonStop->setObjectName(QString::fromUtf8("toolButtonStop"));
        QIcon icon8;
        icon8.addPixmap(QPixmap(QString::fromUtf8(":/icons/icons/control_stop.png")), QIcon::Normal, QIcon::Off);
        toolButtonStop->setIcon(icon8);

        horizontalLayout_4->addWidget(toolButtonStop);

        toolButtonPlay = new QToolButton(centralwidget);
        toolButtonPlay->setObjectName(QString::fromUtf8("toolButtonPlay"));
        QIcon icon9;
        icon9.addPixmap(QPixmap(QString::fromUtf8(":/icons/icons/control_play.png")), QIcon::Normal, QIcon::Off);
        toolButtonPlay->setIcon(icon9);

        horizontalLayout_4->addWidget(toolButtonPlay);


        verticalLayout->addLayout(horizontalLayout_4);

        sliderPlaybackRate = new QSlider(centralwidget);
        sliderPlaybackRate->setObjectName(QString::fromUtf8("sliderPlaybackRate"));
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(sliderPlaybackRate->sizePolicy().hasHeightForWidth());
        sliderPlaybackRate->setSizePolicy(sizePolicy3);
        sliderPlaybackRate->setMinimumSize(QSize(0, 10));
        sliderPlaybackRate->setMaximumSize(QSize(16777215, 15));
        sliderPlaybackRate->setValue(50);
        sliderPlaybackRate->setOrientation(Qt::Horizontal);
        sliderPlaybackRate->setInvertedAppearance(false);

        verticalLayout->addWidget(sliderPlaybackRate);


        horizontalLayout_2->addLayout(verticalLayout);

        timeline = new TimeLineWidget(centralwidget);
        timeline->setObjectName(QString::fromUtf8("timeline"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(timeline->sizePolicy().hasHeightForWidth());
        timeline->setSizePolicy(sizePolicy4);

        horizontalLayout_2->addWidget(timeline);


        horizontalLayout->addLayout(horizontalLayout_2);


        verticalLayout_2->addLayout(horizontalLayout);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(8);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        labelFilename = new QLabel(centralwidget);
        labelFilename->setObjectName(QString::fromUtf8("labelFilename"));
        labelFilename->setMaximumSize(QSize(16777215, 20));
        QFont font;
        font.setPointSize(7);
        labelFilename->setFont(font);

        horizontalLayout_3->addWidget(labelFilename);

        labelVersion = new QLabel(centralwidget);
        labelVersion->setObjectName(QString::fromUtf8("labelVersion"));
        labelVersion->setMaximumSize(QSize(16777215, 20));
        labelVersion->setFont(font);

        horizontalLayout_3->addWidget(labelVersion);

        labelDate = new QLabel(centralwidget);
        labelDate->setObjectName(QString::fromUtf8("labelDate"));
        labelDate->setMaximumSize(QSize(16777215, 20));
        labelDate->setFont(font);
        labelDate->setWordWrap(false);

        horizontalLayout_3->addWidget(labelDate);

        horizontalSpacer = new QSpacerItem(40, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);


        verticalLayout_2->addLayout(horizontalLayout_3);

        SDSPlayerClass->setCentralWidget(centralwidget);
        menubar = new QMenuBar(SDSPlayerClass);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 790, 22));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        SDSPlayerClass->setMenuBar(menubar);

        menubar->addAction(menuFile->menuAction());
        menuFile->addAction(action_Open);
        menuFile->addAction(actionQuit);

        retranslateUi(SDSPlayerClass);
        QObject::connect(toolButtonPlay, SIGNAL(clicked()), timeline, SLOT(play()));
        QObject::connect(toolButtonStop, SIGNAL(clicked()), timeline, SLOT(stop()));
        QObject::connect(toolButtonRewind, SIGNAL(clicked()), timeline, SLOT(rewind()));
        QObject::connect(sliderPlaybackRate, SIGNAL(valueChanged(int)), SDSPlayerClass, SLOT(playbackRateChanged(int)));
        QObject::connect(comboBoxViewMode, SIGNAL(activated(QString)), viewer, SLOT(setViewMode(QString)));
        QObject::connect(toolButtonFitToWindow, SIGNAL(clicked()), viewer, SLOT(showEntireScene()));
        QObject::connect(comboBoxSelectMode, SIGNAL(activated(QString)), SDSPlayerClass, SLOT(selectMode(QString)));

        comboBoxViewMode->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(SDSPlayerClass);
    } // setupUi

    void retranslateUi(QMainWindow *SDSPlayerClass)
    {
        SDSPlayerClass->setWindowTitle(QApplication::translate("SDSPlayerClass", "SDSPlayer", 0, QApplication::UnicodeUTF8));
        SDSPlayerClass->setStyleSheet(QString());
        action_Open->setText(QApplication::translate("SDSPlayerClass", "&Open...", 0, QApplication::UnicodeUTF8));
        actionQuit->setText(QApplication::translate("SDSPlayerClass", "&Quit", 0, QApplication::UnicodeUTF8));
        comboBoxViewMode->setItemText(0, QApplication::translate("SDSPlayerClass", "wireframe", 0, QApplication::UnicodeUTF8));
        comboBoxViewMode->setItemText(1, QApplication::translate("SDSPlayerClass", "surface", 0, QApplication::UnicodeUTF8));
        comboBoxViewMode->setItemText(2, QApplication::translate("SDSPlayerClass", "smooth", 0, QApplication::UnicodeUTF8));
        comboBoxViewMode->setItemText(3, QApplication::translate("SDSPlayerClass", "cells", 0, QApplication::UnicodeUTF8));

#ifndef QT_NO_TOOLTIP
        comboBoxViewMode->setToolTip(QApplication::translate("SDSPlayerClass", "View mode", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        comboBoxEditMode->setItemText(0, QApplication::translate("SDSPlayerClass", "view", 0, QApplication::UnicodeUTF8));
        comboBoxEditMode->setItemText(1, QApplication::translate("SDSPlayerClass", "inspect", 0, QApplication::UnicodeUTF8));

#ifndef QT_NO_TOOLTIP
        comboBoxEditMode->setToolTip(QApplication::translate("SDSPlayerClass", "Interaction mode", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        toolButtonFitToWindow->setToolTip(QApplication::translate("SDSPlayerClass", "Fit to window", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonFitToWindow->setText(QApplication::translate("SDSPlayerClass", "...", 0, QApplication::UnicodeUTF8));
        comboBoxSelectMode->clear();
        comboBoxSelectMode->insertItems(0, QStringList()
         << QApplication::translate("SDSPlayerClass", "cells", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SDSPlayerClass", "edges", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SDSPlayerClass", "tetrahedra", 0, QApplication::UnicodeUTF8)
        );
        pushButtonTetTopograph->setText(QApplication::translate("SDSPlayerClass", "tet topograph", 0, QApplication::UnicodeUTF8));
        lineEdit->setText(QApplication::translate("SDSPlayerClass", "shift-click to select an element", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        toolButtonRewind->setToolTip(QApplication::translate("SDSPlayerClass", "Rewind", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonRewind->setText(QApplication::translate("SDSPlayerClass", "...", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        toolButtonStop->setToolTip(QApplication::translate("SDSPlayerClass", "Stop", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonStop->setText(QApplication::translate("SDSPlayerClass", "...", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        toolButtonPlay->setToolTip(QApplication::translate("SDSPlayerClass", "Play", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonPlay->setText(QApplication::translate("SDSPlayerClass", "...", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        sliderPlaybackRate->setToolTip(QApplication::translate("SDSPlayerClass", "Playback rate", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        timeline->setToolTip(QApplication::translate("SDSPlayerClass", "Timeline", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        labelFilename->setToolTip(QApplication::translate("SDSPlayerClass", "Filename", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        labelFilename->setText(QApplication::translate("SDSPlayerClass", "filename", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        labelVersion->setToolTip(QApplication::translate("SDSPlayerClass", "Version/format of the file.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        labelVersion->setText(QApplication::translate("SDSPlayerClass", "version", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        labelDate->setToolTip(QApplication::translate("SDSPlayerClass", "Date", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        labelDate->setText(QApplication::translate("SDSPlayerClass", "date", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("SDSPlayerClass", "File", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class SDSPlayerClass: public Ui_SDSPlayerClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SDSPLAYER_H
