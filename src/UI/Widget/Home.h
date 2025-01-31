﻿#pragma once

#include "Editor.h"
#include "Info.h"
#include "Jump.h"
#include "Menu.h"
#include "Prefer.h"
#include "Type.h"
#include "../Interface.h"
#include "../../Config.h"
#include "../../Local.h"
#include "../../Access/Load.h"
#include "../../Access/Post.h"
#include "../../Access/Seek.h"
#include "../../Access/Sign.h"
#include "../../Model/Danmaku.h"
#include "../../Model/Running.h"
#include "../../Model/List.h"
#include "../../Model/Shield.h"
#include "../../Player/APlayer.h"
#include "../../Render/ARender.h"
#include <QtCore>
#include <QtWidgets>
#include<HelpWindow.h>

namespace UI
{
	class Home : public QWidget
	{
		Q_OBJECT
	public:
		explicit Home(QWidget *parent = nullptr);

		void percent(double degree);
		void warning(QString title, QString text);

	private:
		QTimer *timer;
		QTimer *delay;

		QAction *quitA;
		QAction *fullA;
		QAction *confA;
		QAction *toggA;
		QAction *listA;
		QAction *postA;
        QAction *helpA;
		QMenu *rat;
		QMenu *sca;
		QMenu *spd;
		QPointer<QDialog> msg;

		UI::Menu *menu;
		UI::Info *info;
		UI::Jump *jump;
		UI::Type *type;
		List *list;
		Load *load;
		Post *post;
		Seek *seek;
		Sign *sign;
		APlayer *aplayer;
		Danmaku *danmaku;
		Running *running;
		ARender *arender;

		QPoint sta;
		QPoint wgd;
		QByteArray geo;

		bool showprg;
		bool sliding;

		virtual void closeEvent(QCloseEvent *e) override;
		virtual void dragEnterEvent(QDragEnterEvent *e) override;
		virtual void dropEvent(QDropEvent *e) override;
		virtual void mouseDoubleClickEvent(QMouseEvent *e) override;
		virtual void mouseMoveEvent(QMouseEvent *e) override;
		virtual void mousePressEvent(QMouseEvent *e) override;
		virtual void mouseReleaseEvent(QMouseEvent *e) override;
		virtual void resizeEvent(QResizeEvent *e) override;

		QSize parseSize(QString string);
		void setGeometry(QSize size, bool center);
		void setWindowFlags();
		void checkForUpdate();
		void showContextMenu(QPoint p);
	};
}
