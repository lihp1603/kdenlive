/*
Copyright (C) 2012  Till Theato <root@ttill.de>
This file is part of kdenlive. See www.kdenlive.org.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "timelinetrackitem.h"
#include "timelinescene.h"
#include "timelineclipitem.h"
#include "project/timelinetrack.h"
#include "project/abstracttimelineclip.h"
#include "project/abstractprojectclip.h"
#include "project/abstractclipplugin.h"
#include "project/producerwrapper.h"


TimelineTrackItem::TimelineTrackItem(TimelineTrack* track, QObject* parent) :
    QObject(parent),
    m_track(track)
{
    // the height should be adjustable
    setRect(0, 0, 1, 50);
    // red only for testing purposes
    setBrush(Qt::red);

    loadClips();
    updateGeometry();

    connect(m_track, SIGNAL(durationChanged(int)), this, SLOT(updateGeometry()));
}

TimelineTrackItem::~TimelineTrackItem()
{

}

int TimelineTrackItem::type() const
{
    return Type;
}

TimelineTrack* TimelineTrackItem::track()
{
    return m_track;
}

void TimelineTrackItem::updateGeometry()
{
    QRectF r = rect();
    r.setWidth(m_track->producer()->get_playtime());
    setRect(r);
}

void TimelineTrackItem::loadClips()
{
    QList <AbstractTimelineClip*> clips = m_track->clips();
    foreach(AbstractTimelineClip *clip, clips) {
        TimelineClipItem *item = clip->projectClip()->plugin()->timelineClipView(clip, this);
        m_clipItems.append(item);
    }
}


#include "timelinetrackitem.moc"