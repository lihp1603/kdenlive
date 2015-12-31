/***************************************************************************
 *   Copyright (C) 2007 by Jean-Baptiste Mardelle (jb@kdenlive.org)        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/

#include "definitions.h"
#include <klocalizedstring.h>

QDebug operator << (QDebug qd, const ItemInfo &info)
{
    qd << "ItemInfo " << &info;
    qd << "\tTrack" << info.track;
    qd << "\tStart pos: " << info.startPos.toString();
    qd << "\tEnd pos: " << info.endPos.toString();
    qd << "\tCrop start: " << info.cropStart.toString();
    qd << "\tCrop duration: " << info.cropDuration.toString();
    return qd.maybeSpace();
}

QDebug operator << (QDebug qd, const MltVideoProfile &profile)
{
    qd << "Profile "<< &profile;
    qd << "\tProfile fps num " << profile.frame_rate_num;
    qd << "\tProfile fps den " << profile.frame_rate_den;
    qd << "\tProfile width " << profile.width,
    qd << "\tProfile height " << profile.height;
    qd << "\tProfile progressive " << profile.progressive;
    qd << "\tProfile sar num " << profile.sample_aspect_num;
    qd << "\tProfile sar den " << profile.sample_aspect_den;
    qd << "\tProfile dar num " << profile.display_aspect_num;
    qd << "\tProfile dar den " << profile.display_aspect_den;
    qd << "\tProfile colorspace " << profile.colorspace;
    qd << "\tProfile description " << profile.description;
    return qd.maybeSpace();
}


MltVideoProfile::MltVideoProfile() :
    frame_rate_num(0),
    frame_rate_den(0),
    width(0),
    height(0),
    progressive(0),
    sample_aspect_num(0),
    sample_aspect_den(0),
    display_aspect_num(0),
    display_aspect_den(0),
    colorspace(0)
{
}

bool MltVideoProfile::operator==(const MltVideoProfile &point) const
{
    if (!description.isEmpty() && point.description  == description) {
        return true;
    }
    int fps = frame_rate_num * 100 / frame_rate_den;
    int sar = sample_aspect_num * 100 / sample_aspect_den;
    int dar = display_aspect_num * 100 / display_aspect_den;
    return      point.frame_rate_num * 100 / point.frame_rate_den == fps &&
            point.width == width &&
            point.height == height &&
            point.progressive == progressive &&
            point.sample_aspect_num * 100 / point.sample_aspect_den == sar &&
            point.display_aspect_num * 100 / point.display_aspect_den == dar &&
            point.colorspace == colorspace;
}

bool MltVideoProfile::operator!=(const MltVideoProfile &other) const {
    return !(*this == other);
}

CommentedTime::CommentedTime(): t(GenTime(0)), type(0) {}


CommentedTime::CommentedTime(const GenTime &time, const QString &comment, int markerType)
    : t(time), c(comment), type(markerType) { }

CommentedTime::CommentedTime(const QString &hash, const GenTime &time)
    : t(time), c(hash.section(QStringLiteral(":"), 1)), type(hash.section(QStringLiteral(":"), 0, 0).toInt()) { }

QString CommentedTime::comment() const          {
    return (c.isEmpty() ? i18n("Marker") : c);
}

GenTime CommentedTime::time() const          {
    return t;
}

void CommentedTime::setComment(const QString &comm) {
    c = comm;
}

void CommentedTime::setMarkerType(int t) {
    type = t;
}

QString CommentedTime::hash() const
{
    return QString::number(type) + ":" + (c.isEmpty() ? i18n("Marker") : c);
}

int CommentedTime::markerType() const {
    return type;
}

QColor CommentedTime::markerColor(int type) {
    switch (type) {
    case 0:
        return Qt::red;
        break;
    case 1:
        return Qt::blue;
        break;
    case 2:
        return Qt::green;
        break;
    case 3:
        return Qt::yellow;
        break;
    default:
        return Qt::cyan;
        break;
    }
}

bool CommentedTime::operator>(CommentedTime op) const {
    return t > op.time();
}

bool CommentedTime::operator<(CommentedTime op) const {
    return t < op.time();
}

bool CommentedTime::operator>=(CommentedTime op) const {
    return t >= op.time();
}

bool CommentedTime::operator<=(CommentedTime op) const {
    return t <= op.time();
}

bool CommentedTime::operator==(CommentedTime op) const {
    return t == op.time();
}

bool CommentedTime::operator!=(CommentedTime op) const {
    return t != op.time();
}
