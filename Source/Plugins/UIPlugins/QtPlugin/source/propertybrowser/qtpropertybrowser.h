/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/


#ifndef QTPROPERTYBROWSER_H
#define QTPROPERTYBROWSER_H

#include <QWidget>
#include <QSet>
#include <QLineEdit>
#include <QDateTime>
#include <QKeyEvent>

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

#if defined(Q_OS_WIN)
#  if !defined(QT_QTPROPERTYBROWSER_EXPORT) && !defined(QT_QTPROPERTYBROWSER_IMPORT)
#    define QT_QTPROPERTYBROWSER_EXPORT
#  elif defined(QT_QTPROPERTYBROWSER_IMPORT)
#    if defined(QT_QTPROPERTYBROWSER_EXPORT)
#      undef QT_QTPROPERTYBROWSER_EXPORT
#    endif
#    define QT_QTPROPERTYBROWSER_EXPORT __declspec(dllimport)
#  elif defined(QT_QTPROPERTYBROWSER_EXPORT)
#    undef QT_QTPROPERTYBROWSER_EXPORT
#    define QT_QTPROPERTYBROWSER_EXPORT __declspec(dllexport)
#  endif
#else
#  define QT_QTPROPERTYBROWSER_EXPORT
#endif

typedef QLineEdit::EchoMode EchoMode;

class QtAbstractPropertyManager;
class QtPropertyPrivate;

class QT_QTPROPERTYBROWSER_EXPORT QtProperty
{
public:
	enum BackupMode
	{
		NoBackup,
		BackupToInitial,	//back to initial value when editor created
		BackupToLast,		//back to last value when editor activate/gaining focus
		BackupToCurrent,	//back to current (latest committed) property value, useful for focus editing mode only (i.e. line edit, spin)
	};
    virtual ~QtProperty();

    QList<QtProperty *> subProperties() const;

    QtAbstractPropertyManager *propertyManager() const;

    QString toolTip() const;
    QString statusTip() const;
    QString whatsThis() const;
    QString propertyName() const;
    bool isEnabled() const;
    bool isModified() const;
	BackupMode backupMode() const;

	bool isDefaultExpanded() const;
	void setDefaultExpanded(bool expanded);

    bool hasValue() const;
    QIcon valueIcon() const;
    QString valueText() const;
    QString displayText() const;

    void setToolTip(const QString &text);
    void setStatusTip(const QString &text);
    void setWhatsThis(const QString &text);
    void setPropertyName(const QString &text);
    void setEnabled(bool enable);
    void setModified(bool modified);
	void setBackupMode(BackupMode mode);

    void addSubProperty(QtProperty *property);
    void insertSubProperty(QtProperty *property, QtProperty *afterProperty);
    void removeSubProperty(QtProperty *property);
protected:
    explicit QtProperty(QtAbstractPropertyManager *manager);
    void propertyChanged();
private:
    friend class QtAbstractPropertyManager;
    QtPropertyPrivate *d_ptr;
};

class QtAbstractPropertyManagerPrivate;

class QT_QTPROPERTYBROWSER_EXPORT QtAbstractPropertyManager : public QObject
{
    Q_OBJECT
public:

    explicit QtAbstractPropertyManager(QObject *parent = 0);
    ~QtAbstractPropertyManager();

    QSet<QtProperty *> properties() const;
    void clear() const;

    QtProperty *addProperty(const QString &name = QString());
Q_SIGNALS:

    void propertyInserted(QtProperty *property,
                QtProperty *parent, QtProperty *after);
    void propertyChanged(QtProperty *property);
    void propertyRemoved(QtProperty *property, QtProperty *parent);
    void propertyDestroyed(QtProperty *property);
protected:
    virtual bool hasValue(const QtProperty *property) const;
    virtual QIcon valueIcon(const QtProperty *property) const;
    virtual QString valueText(const QtProperty *property) const;
    virtual QString displayText(const QtProperty *property) const;
    virtual EchoMode echoMode(const QtProperty *) const;
    virtual void initializeProperty(QtProperty *property) = 0;
    virtual void uninitializeProperty(QtProperty *property);
    virtual QtProperty *createProperty();
private:
    friend class QtProperty;
    QtAbstractPropertyManagerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtAbstractPropertyManager)
    Q_DISABLE_COPY(QtAbstractPropertyManager)
};

class QT_QTPROPERTYBROWSER_EXPORT QtAbstractEditorFactoryBase : public QObject
{
    Q_OBJECT
public:
    virtual QWidget *createEditor(QtProperty *property, QWidget *parent) = 0;
protected:
    explicit QtAbstractEditorFactoryBase(QObject *parent = 0)
        : QObject(parent) {}

    virtual void breakConnection(QtAbstractPropertyManager *manager) = 0;
protected Q_SLOTS:
    virtual void managerDestroyed(QObject *manager) = 0;

    friend class QtAbstractPropertyBrowser;
};

template <class PropertyManager>
class QtBackupMananger
{
public:
	typedef typename PropertyManager::value_type value_type;
	///note: one property can have multiple editors, so create per editor backup
	///this is also easy and won't change implementations of QtAbstractPropertyManager
	void backup(PropertyManager* manger, QtProperty* prop, QWidget* editor)
	{
		value_type val = manger->value(prop);
		QtBakcupItem bi = {prop, val};
		mBackupMap[editor] = bi;
	}
	void backup(QWidget* editor)
	{
		EditorToBackup::iterator it = mBackupMap.find(editor);
		if(it != mBackupMap.end())
		{
			QtProperty* p = it.value().prop;
			if(p->backupMode() != QtProperty::BackupToInitial)
			{
				PropertyManager* pm = static_cast<PropertyManager*>(p->propertyManager());
				it.value().val = pm->value(p);
			}
		}
	}
	bool restore(QWidget* editor, value_type& outVal)
	{
		EditorToBackup::iterator it = mBackupMap.find(editor);
		if(it != mBackupMap.end())
		{
			QtProperty* p = it.value().prop;
			PropertyManager* pm = static_cast<PropertyManager*>(p->propertyManager());
			if (p->backupMode() == QtProperty::BackupToCurrent)
				it.value().val = pm->value(p);
			else
				pm->setValue(p, it.value().val);
			outVal = it.value().val;
			return true;
		}
		return false;
	}
	void remove(QWidget* editor)
	{
		EditorToBackup::iterator it = mBackupMap.find(editor);
		if(it != mBackupMap.end())
			mBackupMap.erase(it);
	}
protected:
	struct QtBakcupItem
	{
		QtProperty*	prop;
		value_type	val;
	};
	typedef QMap<QWidget*, QtBakcupItem> EditorToBackup;
	EditorToBackup mBackupMap;
};

template <class PropertyManager>
class QtAbstractEditorFactory : public QtAbstractEditorFactoryBase
{
public:
	typedef typename QtBackupMananger<PropertyManager>::value_type value_type;

    explicit QtAbstractEditorFactory(QObject *parent) : QtAbstractEditorFactoryBase(parent) {}
    QWidget *createEditor(QtProperty *property, QWidget *parent)
    {
        QSetIterator<PropertyManager *> it(m_managers);
        while (it.hasNext()) {
            PropertyManager *manager = it.next();
            if (manager == property->propertyManager()) {
                 QWidget* widget = createEditor(manager, property, parent);
				///note: change backup mode in the middle of editing won't change 
				///its behavior
				if(widget != Q_NULLPTR && property->backupMode() != QtProperty::NoBackup)
				{
					widget->installEventFilter(this);
					mBackupManager.backup(manager, property, widget);
				}
				return widget;
            }
        }
        return 0;
    }
    void addPropertyManager(PropertyManager *manager)
    {
        if (m_managers.contains(manager))
            return;
        m_managers.insert(manager);
        connectPropertyManager(manager);
        connect(manager, SIGNAL(destroyed(QObject *)),
                    this, SLOT(managerDestroyed(QObject *)));
    }
    void removePropertyManager(PropertyManager *manager)
    {
        if (!m_managers.contains(manager))
            return;
        disconnect(manager, SIGNAL(destroyed(QObject *)),
                    this, SLOT(managerDestroyed(QObject *)));
        disconnectPropertyManager(manager);
        m_managers.remove(manager);
    }
    QSet<PropertyManager *> propertyManagers() const
    {
        return m_managers;
    }
    PropertyManager *propertyManager(QtProperty *property) const
    {
        QtAbstractPropertyManager *manager = property->propertyManager();
        QSetIterator<PropertyManager *> itManager(m_managers);
        while (itManager.hasNext()) {
            PropertyManager *m = itManager.next();
            if (m == manager) {
                return m;
            }
        }
        return 0;
    }
protected:
    virtual void connectPropertyManager(PropertyManager *manager) = 0;
    virtual QWidget *createEditor(PropertyManager *manager, QtProperty *property,
                QWidget *parent) = 0;
    virtual void disconnectPropertyManager(PropertyManager *manager) = 0;
    void managerDestroyed(QObject *manager)
    {
        QSetIterator<PropertyManager *> it(m_managers);
        while (it.hasNext()) {
            PropertyManager *m = it.next();
            if (m == manager) {
                m_managers.remove(m);
                return;
            }
        }
    }
	
	//TODO: this is much better, but need modify each implementation
	virtual value_type value(QWidget* editor) const { Q_UNUSED(editor); return value_type(); }
	virtual bool setValue(QWidget* editor, value_type val) const { Q_UNUSED(editor); Q_UNUSED(val); return false; }

	virtual bool eventFilter(QObject *watched, QEvent *evt)
	{
		//destruction safety guard
		if (this->propertyManagers().size() == 0)
			return false;

		QWidget* editor = static_cast<QWidget*>(watched);//unsafe cast but safe use in map
		if(evt->type() == QEvent::FocusIn)
		{
			mBackupManager.backup(editor);
			//BLADE_LOG(DebugOutput, BTString("focus in"));
		}
		else if (evt->type() == QEvent::KeyPress && static_cast<QKeyEvent*>(evt)->key() == Qt::Key_Escape)
		{
			value_type val;
			if (mBackupManager.restore(editor, val))
				this->setValue(editor, val);
		}
		else if(evt->type() == QEvent::Destroy)
			mBackupManager.remove(editor);
		return false;
	}
	QtBackupMananger<PropertyManager> mBackupManager;
private:
    void breakConnection(QtAbstractPropertyManager *manager)
    {
        QSetIterator<PropertyManager *> it(m_managers);
        while (it.hasNext()) {
            PropertyManager *m = it.next();
            if (m == manager) {
                removePropertyManager(m);
                return;
            }
        }
    }
private:
    QSet<PropertyManager *> m_managers;
    friend class QtAbstractPropertyEditor;
};

class QtAbstractPropertyBrowser;
class QtBrowserItemPrivate;

class QT_QTPROPERTYBROWSER_EXPORT QtBrowserItem
{
public:
    QtProperty *property() const;
    QtBrowserItem *parent() const;
    QList<QtBrowserItem *> children() const;
    QtAbstractPropertyBrowser *browser() const;
private:
    explicit QtBrowserItem(QtAbstractPropertyBrowser *browser, QtProperty *property, QtBrowserItem *parent);
    ~QtBrowserItem();
    QtBrowserItemPrivate *d_ptr;
    friend class QtAbstractPropertyBrowserPrivate;
};

class QtAbstractPropertyBrowserPrivate;

class QT_QTPROPERTYBROWSER_EXPORT QtAbstractPropertyBrowser : public QWidget
{
    Q_OBJECT
public:

    explicit QtAbstractPropertyBrowser(QWidget *parent = 0);
    ~QtAbstractPropertyBrowser();

    QList<QtProperty *> properties() const;
    QList<QtBrowserItem *> items(QtProperty *property) const;
    QtBrowserItem *topLevelItem(QtProperty *property) const;
    QList<QtBrowserItem *> topLevelItems() const;
    void clear();

    template <class PropertyManager>
    void setFactoryForManager(PropertyManager *manager,
                    QtAbstractEditorFactory<PropertyManager> *factory) {
        QtAbstractPropertyManager *abstractManager = manager;
        QtAbstractEditorFactoryBase *abstractFactory = factory;

        if (addFactory(abstractManager, abstractFactory))
            factory->addPropertyManager(manager);
    }

    void unsetFactoryForManager(QtAbstractPropertyManager *manager);

    QtBrowserItem *currentItem() const;
    void setCurrentItem(QtBrowserItem *);

Q_SIGNALS:
    void currentItemChanged(QtBrowserItem *);

public Q_SLOTS:

    QtBrowserItem *addProperty(QtProperty *property);
    QtBrowserItem *insertProperty(QtProperty *property, QtProperty *afterProperty);
    void removeProperty(QtProperty *property);

	QtAbstractEditorFactoryBase* matchFactory(QtProperty* prop);
protected:

    virtual void itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem) = 0;
    virtual void itemRemoved(QtBrowserItem *item) = 0;
    // can be tooltip, statustip, whatsthis, name, icon, text.
    virtual void itemChanged(QtBrowserItem *item) = 0;

    virtual QWidget *createEditor(QtProperty *property, QWidget *parent);
private:

    bool addFactory(QtAbstractPropertyManager *abstractManager,
                QtAbstractEditorFactoryBase *abstractFactory);

    QtAbstractPropertyBrowserPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtAbstractPropertyBrowser)
    Q_DISABLE_COPY(QtAbstractPropertyBrowser)
    Q_PRIVATE_SLOT(d_func(), void slotPropertyInserted(QtProperty *,
                            QtProperty *, QtProperty *))
    Q_PRIVATE_SLOT(d_func(), void slotPropertyRemoved(QtProperty *,
                            QtProperty *))
    Q_PRIVATE_SLOT(d_func(), void slotPropertyDestroyed(QtProperty *))
    Q_PRIVATE_SLOT(d_func(), void slotPropertyDataChanged(QtProperty *))

};

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif

#endif // QTPROPERTYBROWSER_H
