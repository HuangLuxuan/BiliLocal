/*=======================================================================
*
*   Copyright (C) 2013-2015 Lysine.
*
*   Filename:    Local.cpp
*   Time:        2013/03/18
*   Author:      Lysine
*
*   Lysine is a student majoring in Software Engineering
*   from the School of Software, SUN YAT-SEN UNIVERSITY.
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.

*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
=========================================================================*/

#include "Common.h"
#include "Local.h"
#include "Bundle.h"
#include "Config.h"
#include "Plugin.h"
#include "Utils.h"
#include "Access/Load.h"
#include "Access/Post.h"
#include "Access/Seek.h"
#include "Access/Sign.h"
#include "Model/Danmaku.h"
#include "Model/Running.h"
#include "Model/List.h"
#include "Model/Shield.h"
#include "Player/APlayer.h"
#include "Render/ARender.h"
#include "UI/Interface.h"
#include <type_traits>
#include <iostream>

Local *Local::ins = nullptr;

Local *Local::instance()
{
	return ins ? ins : new Local(qApp);
}

Local::Local(QObject *parent)
	: QObject(parent)
{
	ins = this;
	setObjectName("Local");
#define lIns(ModuleType) (this->objects[#ModuleType] = new ModuleType(this))
	lIns(Config);
	lIns(Shield);
	lIns(Danmaku);
	lIns(Running);
	lIns(Interface);
	objects["APlayer"] = APlayer::create(this);
	objects["ARender"] = ARender::create(this);
	lIns(Load);
	lIns(Post);
	lIns(Seek);
	lIns(Sign);
	lIns(List);
#define lSet(ModuleType) static_cast<ModuleType *>(this->objects[#ModuleType])->setup()
	lSet(Interface);
	lSet(ARender);
	lSet(Running);
#undef lIns
#undef lSet
	connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
		delete this;
	});
}

Local::~Local()
{
	Config::save();
	delete findObject<APlayer>();
	delete findObject<Running>();
	delete findObject<ARender>();
}

bool firstLoadDanmaku=false;

void Local::tryLocal(QString path)
{
	QFileInfo info(path);
	QString suffix = info.suffix().toLower();
	if (info.exists() == false) {
		return;
	}
	else if (Utils::getSuffix(Utils::Danmaku).contains(suffix)) {
        std::cout<<"command line load danmaku "<<path.toStdString()<<std::endl;
        findObject<Load>()->loadDanmaku(path,!firstLoadDanmaku);//如果不是这次命令行解析时第一次加载弹幕文件
                                                                //那么之前加载的肯定要保留，需要覆盖掉是否清除弹幕的标志
        firstLoadDanmaku=false;//标记一下已经解析了一次弹幕文件了
	}
    else if (/*findObject<APlayer>()->getState() != APlayer::Stop
        && */Utils::getSuffix(Utils::Subtitle).contains(suffix)) {//没在播放的时候APlayer::addSubtitle(QString)方法
                                                                    //不应该因为会被媒体文件加载过程覆盖字幕而不允许调用并丢弃这个字幕，
                                                                    //而是应该将该文件加入队列，等待媒体文件加载完成可以再去加载这个字幕
        std::cout<<"command line load subtitle "<<path.toStdString()<<std::endl;
        findObject<APlayer>()->addSubtitle(path);
        findObject<List>()->currentSubtitleFile=path;
	}
	else {
        std::cout<<"command line load media "<<path.toStdString()<<std::endl;
		switch (Config::getValue("/Interface/Single", 1)) {
		case 0:
		case 1:
			findObject<APlayer>()->stop();
			findObject<APlayer>()->setMedia(path);
			break;
		case 2:
			findObject<List>()->appendMedia(path);
			break;
		}
	}
}

namespace
{
	void setDefaultFont()
	{
		QString def = Utils::defaultFont();
		QFontInfo i(qApp->font());
		if (!QFontDatabase().families().contains(def)){
			def = i.family();
		}
		double p = i.pointSizeF();
		QFont f;
		f.setFamily(Config::getValue("/Interface/Font/Family", def));
		f.setPointSizeF(Config::getValue("/Interface/Font/Size", p));
		qApp->setFont(f);
	}

	void loadTranslator()
	{
		QString path = Utils::localPath(Utils::Locale);
		QString name = Config::getValue("/Interface/Locale", QLocale::system().name());
		QFileInfoList list;
		list += QDir(path + name).entryInfoList();
		list += QFileInfo(path + name + ".qm");
		name.resize(2);
		list += QDir(path + name).entryInfoList();
		list += QFileInfo(path + name + ".qm");
		for (QFileInfo info : list){
			if (!info.isFile()){
				continue;
			}
			QTranslator *trans = new QTranslator(qApp);
			if (trans->load(info.absoluteFilePath())){
				qApp->installTranslator(trans);
			}
			else{
				delete trans;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	std::remove_pointer<decltype(qApp)>::type a(argc, argv);
	Bundle::push();
	Config::load();
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
	int single = Config::getValue("/Interface/Single", 1);
	if (single){
		QLocalSocket socket;
        socket.connectToServer("BiliLocalInstance");
        if (socket.waitForConnected()){
            QByteArray byteArray;
            //QDataStream s(&socket);
            QDataStream s(&byteArray,QIODevice::WriteOnly);
            QStringList outputArgs=a.arguments().mid(1);
            std::cout<<"outputArgs="<<outputArgs.join(' ').toStdString()<<std::endl;
            s << outputArgs;
            std::cout<<"byteArray="<<byteArray.toHex(' ').toStdString()<<std::endl;
            socket.write(byteArray);
            if(!socket.waitForBytesWritten())
                MessageBoxW(GetForegroundWindow(),(std::wstring(L"QLocalSocket::waitForBytesWritten()返回了false\n")+socket.errorString().toStdWString()+L"("+std::to_wstring(socket.error())+L")").c_str(),L"参数发送失败",MB_OK|MB_ICONERROR);
			return 0;
        }
	}
	a.setAttribute(Qt::AA_UseOpenGLES);
#endif
	qThreadPool->setMaxThreadCount(Config::getValue("/Danmaku/Thread", QThread::idealThreadCount()));
	qsrand(QTime::currentTime().msec());
	loadTranslator();
	setDefaultFont();
    //new Local(&a);
	Plugin::load();
	lApp->findObject<Interface>()->show();
    firstLoadDanmaku=true;//标记一下一次命令解析刚开始
	for (const QString iter : a.arguments().mid(1)) {
        lApp->tryLocal(iter);//解析命令
	}
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
	QLocalServer *server = nullptr;
	if (single){
		server = new QLocalServer(&a);
		server->listen("BiliLocalInstance");
		QObject::connect(server, &QLocalServer::newConnection, [=](){
			QLocalSocket *r = server->nextPendingConnection();
            if(r->waitForReadyRead()){
                QByteArray byteArray=r->readAll();
                std::cout<<"byteArray="<<byteArray.toHex(' ').toStdString()<<std::endl;
                //QDataStream s(r);
                QDataStream s(&byteArray,QIODevice::ReadOnly);
                QStringList args;
                s >> args;
                std::cout<<"args="<<args.join(' ').toStdString()<<std::endl;
                delete r;
                firstLoadDanmaku=true;//标记一下一次命令解析刚开始
                for (const QString iter : args) {
                    lApp->tryLocal(iter);//解析命令
                }
            } else {
                MessageBoxW(GetForegroundWindow(),(std::wstring(L"QLocalSocket::waitForReadyRead()返回了false\n")+r->errorString().toStdWString()+L"("+std::to_wstring(r->error())+L")").c_str(),L"参数接收失败",MB_OK|MB_ICONERROR);
                delete r;
            }
		});
	}
	int r = a.exec();
	if (r == 12450){
		if (server){
			delete server;
		}
		QProcess::startDetached(a.applicationFilePath(), QStringList());
		return 0;
	}
	else{
		return r;
	}
#else
	return a.exec();
#endif
}
