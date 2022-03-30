#pragma once

#include "APlayer.h"
#include "Utils.h"
#include <functional>

extern "C"
{
#include <vlc/vlc.h>
}

class VPlayer :public APlayer
{
public:
	enum Event
	{
		Init,
		Wait,
		Free,
		Fail
	};

	explicit VPlayer(QObject *parent = 0);
    virtual ~VPlayer();

private:
	struct Track
	{
		QString name;
		std::function<void()> set;
	};

	struct TrackSlot
	{
		QList<Track> list;
		int current;
	};

	int state;
	libvlc_instance_t *vlc;
	libvlc_media_player_t *mp;
	TrackSlot tracks[3];
    QQueue<QString> pendingSubtitleFiles;//待加载的字幕文件列表

	void    init();
	void    wait();
	void    free();
	void    parseTracks(Utils::Type type);
    void    addSubtitle_internal(QString file);//不会根据state是否等于Stop来判断应该入列队还是直接加载，避免写出死循环的bug

public slots:
	void    play();
	void    stop(bool manually = true);
	int     getState(){ return state; }

	void    setTime(qint64 time);
	qint64  getTime();

	void    setMedia(QString file);
	QString getMedia();

	qint64  getDuration();

	void    setVolume(int volume);
	int     getVolume();

	void    setRate(double rate);
	double  getRate();

	qint64  getDelay(int type);
	void    setDelay(int type, qint64 delay);

	int     getTrack(int type);
	void    setTrack(int type, int index);
	QStringList getTracks(int type);


    void    addSubtitle(QString file);

	void    event(int type);
};
