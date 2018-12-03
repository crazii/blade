/****************************************************************************
**
** Copyright (C) 2006 Trolltech ASA. All rights reserved.
**
** This file is part of the documentation of Qt. It was originally
** published as part of Qt Quarterly.
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation or under the
** terms of the Qt Commercial License Agreement. The respective license
** texts for these are provided with the open source and commercial
** editions of Qt.
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include <QtPluginPCH.h>
#include "filepathmanager.h"

QString FilePathManager::value(const QtProperty *property) const
{
    if (!theValues.contains(property))
        return QString();
    return theValues[property].value;
}

QString FilePathManager::filter(const QtProperty *property) const
{
    if (!theValues.contains(property))
        return QString();
    return theValues[property].filter;
}

QIcon FilePathManager::icon(const QtProperty* property) const
{
	if (!theValues.contains(property))
		return QIcon();
	return theValues[property].icon;
}

EFilePropertyType FilePathManager::type(const QtProperty* property) const
{
	if (!theValues.contains(property))
		return FT_FILE;
	return theValues[property].type;
}

bool	FilePathManager::readOnly(const QtProperty* property) const
{
	if (!theValues.contains(property))
		return true;
	return theValues[property].readOnly;
}

void FilePathManager::setValue(QtProperty *property, const QString &val)
{
	QMap<const QtProperty *, Data>::iterator i = theValues.find(property);
	if(i == theValues.end())
		return;

	Data& data = *i;

    if (data.value == val)
        return;

    data.value = val;

    theValues[property] = data;

    emit propertyChanged(property);
    emit valueChanged(property, data.value);
}

void FilePathManager::setFilter(QtProperty *property, const QString &fil)
{
	QMap<const QtProperty *, Data>::iterator i = theValues.find(property);
	if(i == theValues.end())
		return;

	Data& data = *i;

    if (data.filter == fil)
        return;

    data.filter = fil;

    emit filterChanged(property, data.filter);
}

//////////////////////////////////////////////////////////////////////////
void FilePathManager::setReadOnly(QtProperty* property, bool readOnly)
{
	QMap<const QtProperty *, Data>::iterator i = theValues.find(property);
	if(i == theValues.end())
		return;
	Data& data = *i;

	if (data.readOnly == readOnly)
		return;

	data.readOnly = readOnly;

	emit readOnlyChanged(property, data.readOnly);
}

//////////////////////////////////////////////////////////////////////////
void FilePathManager::setIcon(QtProperty* property, const QIcon& icon)
{
	QMap<const QtProperty *, Data>::iterator i = theValues.find(property);
	if(i == theValues.end())
		return;
	Data& data = *i;

	data.icon = icon;

	emit iconChanged(property, data.icon);
}

//////////////////////////////////////////////////////////////////////////
void FilePathManager::setType(QtProperty* property, EFilePropertyType type)
{
	QMap<const QtProperty *, Data>::iterator i = theValues.find(property);
	if(i == theValues.end())
		return;
	Data& data = *i;

	if(data.type == type)
		return;

	data.type = type;

	emit typeChanged(property, data.type);
}