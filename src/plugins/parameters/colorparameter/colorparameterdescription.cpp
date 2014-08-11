/*
Copyright (C) 2012  Till Theato <root@ttill.de>
This file is part of kdenlive. See www.kdenlive.org.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/


#include "colorparameterdescription.h"
#include "colorparameter.h"
#include <QDomElement>
#include <QLocale>
#include <KPluginFactory>
#include <mlt++/Mlt.h>

K_PLUGIN_FACTORY( ColorParameterFactory, registerPlugin<ColorParameterDescription>(); )
K_EXPORT_PLUGIN( ColorParameterFactory( "kdenlivecolorparameter" ) )


ColorParameterDescription::ColorParameterDescription(QObject* , const QVariantList& )
{
}

ColorParameterDescription::~ColorParameterDescription()
{
}

void ColorParameterDescription::init(const QDomElement &parameter, const QLocale &locale)
{
    AbstractParameterDescription::init(parameter, locale);

    m_prefix = parameter.attribute("paramprefix");
    m_default = ColorParameter::stringToColor(parameter.attribute("default").remove(m_prefix));
    m_supportsAlpha = parameter.attribute("alpha", "0").toInt();
}

void ColorParameterDescription::init(Mlt::Properties& properties, const QLocale &locale)
{
    AbstractParameterDescription::init(properties, locale);

    //QString itemsString = properties.get("paramlist");
    
    m_prefix = properties.get("paramprefix");
    m_default = ColorParameter::stringToColor(QString(properties.get("default")).remove(m_prefix));
    m_supportsAlpha = properties.get_int("alpha");
    m_valid = true;
}

AbstractParameter *ColorParameterDescription::createParameter(AbstractParameterList* parent, const QString &value) const
{
    ColorParameter *parameter = new ColorParameter(this, parent, value);
    return static_cast<AbstractParameter*>(parameter);
}

QColor ColorParameterDescription::defaultValue() const
{
    return m_default;
}

QString ColorParameterDescription::prefix() const
{
    return m_prefix;
}

bool ColorParameterDescription::supportsAlpha() const
{
    return m_supportsAlpha;
}

#include "colorparameterdescription.moc"