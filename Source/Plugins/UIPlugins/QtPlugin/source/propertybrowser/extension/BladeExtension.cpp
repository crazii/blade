/********************************************************************
	created:	2016/08/20
	filename: 	BladeExtension.cpp
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include "BladeExtension.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/
class EditSlider : public QWidget
{
public:
	Q_OBJECT
public:
	EditSlider(QWidget* parent = Q_NULLPTR)
		:QWidget(parent)
		,mEdit(this)
		,mSlider(this)
	{
		QHBoxLayout* layout = QT_NEW QHBoxLayout();
		this->setLayout(layout);
		layout->setMargin(0);
		layout->setSpacing(0);
		layout->addWidget(&mEdit);
		layout->addWidget(&mSlider);

		mVal = 0;
		mStep = 1;

		mValidator.setBottom(-1e2);
		mValidator.setTop(1e2);
		mValidator.setDecimals(3);
		mValidator.setNotation(QDoubleValidator::StandardNotation);
		mEdit.setValidator(&mValidator);
		mEdit.setText("0");
		mEdit.setFrame(false);
		mSlider.setOrientation(Qt::Horizontal);

		connect(&mEdit, SIGNAL(returnPressed()), this, SLOT(editFinish()));
		connect(&mSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderMoved(int)));
		mEdit.installEventFilter(this);
	}

	~EditSlider()
	{

	}

	/** @brief  */
	double value()
	{
		return mVal;
	}

	/** @brief  */
	void setValue(double val)
	{
		if(val > mValidator.top() || val < mValidator.bottom() )
			return;
		if(mVal != val)
		{
			mVal = val;
			mEdit.setText( QString("%0").arg(mVal, 0, 'f', mValidator.decimals()) );
			this->updateSlider(false);
		}
	}

	/** @brief  */
	void setRange(double min, double max)
	{
		Q_ASSERT(max >= min);
		mValidator.setBottom(min);
		mValidator.setTop(max);

		this->updateEditWidth();
		this->updateSlider(true);
	}

	/** @brief  */
	void setSingleStep(double step)
	{
		mStep = step;
		this->updateSlider(true);
	}

	/** @brief  */
	void setDecimals(int prec)
	{
		mValidator.setDecimals(prec);
		this->updateEditWidth();
	}

	/** @brief  */
	bool blockSignals(bool b)
	{
		return QWidget::blockSignals(b) 
			&& mEdit.blockSignals(b) 
			&& mSlider.blockSignals(b);
	}

	/** @brief  */
	void setReadOnly(bool readOnly)
	{
		mEdit.setReadOnly(readOnly);
		//mSlider.setEnabled(!readOnly);
	}

protected:
	/** @brief  */
	void updateSlider(bool updateRange)
	{
		double min = mValidator.bottom();
		double max = mValidator.top();

		if(updateRange)
		{
			double range = std::max(max - min, 0.0) / std::abs(mStep);
			range = std::min(range, ((double)std::numeric_limits<int>::max()));
			mSlider.setRange(0, (int)std::round(range));
		}

		double val = std::max(mVal - min, 0.0) / std::abs(mStep);
		bool prevb = mSlider.blockSignals(true);
		mSlider.setValue((int)val);
		mSlider.blockSignals(prevb);
	}
	/** @brief  */
	void updateEditWidth()
	{
		//note: minimal width of edit makes max of slider
		//but different property control will have different width
#if 0
		QString maxS("%0");
		maxS = maxS.arg(mValidator.top(), 0, 'f', mValidator.decimals());
		int p = 0;
		mValidator.validate(maxS, p);
		mEdit.setMaximumWidth(mEdit.fontMetrics().width(maxS) + mEdit.style()->pixelMetric(QStyle::PM_DefaultFrameWidth)*2 );
#endif
	}

	/** @brief  */
	virtual bool eventFilter(QObject* watched, QEvent *evt)
	{
		if(evt->type() == QEvent::FocusOut)
			this->editFinish();
		else if (evt->type() == QEvent::KeyPress)
		{
			QKeyEvent* ke = static_cast<QKeyEvent*>(evt);
			if (ke->key() == Qt::Key_Escape)
			{
				//rollback
				mEdit.setText(QString("%0").arg(mVal, 0, 'f', mValidator.decimals()));
				mEdit.selectAll();
			}
		}
		return false;
	}

	/** @brief  */
	virtual void focusInEvent(QFocusEvent* evt)
	{
		Q_UNUSED(evt);
		mEdit.selectAll();
		mEdit.setFocus();
	}

signals:
	void valueChanged(double);

	private slots:
		void editFinish()
		{
			//on return pressed, whether it is validated already?
			QString s = mEdit.text();
			int pos = 0;
			mValidator.validate(s, pos);
			double val = s.toDouble();
			if(mVal != val )
			{
				mVal = val;
				this->updateSlider(false);
				emit valueChanged(mVal);
			}
		}

		void sliderMoved(int pos)
		{
			double range = mValidator.top() - mValidator.bottom();
			double val = mValidator.bottom() + mStep * (double)pos;
			if(mVal != val)
			{
				mVal = val;
				mEdit.setText( QString("%0").arg(mVal, 0, 'f', mValidator.decimals()));
				emit valueChanged(mVal);
			}
		}
protected:
	QLineEdit	mEdit;;
	QSlider		mSlider;
	QDoubleValidator	mValidator;
	double		mVal;
	double		mStep;
};

class EnumSlider : public QWidget
{
public:
	Q_OBJECT
public:
	EnumSlider(QWidget* parent = Q_NULLPTR)
		:QWidget(parent)
		,mEdit(this)
		,mSlider(this)
	{
		QHBoxLayout* layout = QT_NEW QHBoxLayout();
		this->setLayout(layout);
		layout->setMargin(0);
		layout->setSpacing(0);

		layout->addWidget(&mEdit);
		layout->addWidget(&mSlider);

		mEdit.setReadOnly(true);
		mEdit.setFrame(false);
		mSlider.setOrientation(Qt::Horizontal);

		connect(&mSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderMoved(int)));
	}

	~EnumSlider()
	{

	}

	/** @brief  */
	bool blockSignals(bool b)
	{
		return QWidget::blockSignals(b) 
			&& mEdit.blockSignals(b) 
			&& mSlider.blockSignals(b);
	}

	/** @brief  */
	void setCurrentIndex(int index)
	{
		Q_ASSERT(index < mEnums.count());
		mEdit.setText(mEnums[index]);
		bool prevb = mSlider.blockSignals(true);
		mSlider.setValue(index);
		mSlider.blockSignals(prevb);
	}

	/** @brief  */
	void clear()
	{
		mEnums.clear();
		mEdit.setText("");
		bool prevb = mSlider.blockSignals(true);
		mSlider.setValue(0);
		mSlider.blockSignals(prevb);
	}

	/** @brief  */
	void addItems(const QStringList& items)
	{
		mEnums.append(items);
		bool prevb = mSlider.blockSignals(true);
		mSlider.setRange(0, mEnums.count() - 1);
		mSlider.blockSignals(prevb);
	}

	/** @brief  */
	void setItemIcon(int index, const QIcon& icon)
	{
		//TODO:
	}

	private slots:
		void sliderMoved(int pos)
		{
			Q_ASSERT(pos < mEnums.count());
			mEdit.setText(mEnums[pos]);
			emit currentIndexChanged(pos);
		}

public:
signals:
	void currentIndexChanged(int value);

protected:
	QLineEdit	mEdit;;
	QSlider		mSlider;
	QStringList mEnums;
};


/************************************************************************/
/*                                                                      */
/************************************************************************/
template <class Editor>
class EditorFactoryPrivate
{
public:

	typedef QList<Editor *> EditorList;
	typedef QMap<QtProperty *, EditorList> PropertyToEditorListMap;
	typedef QMap<Editor *, QtProperty *> EditorToPropertyMap;

	Editor *createEditor(QtProperty *property, QWidget *parent);
	void initializeEditor(QtProperty *property, Editor *e);
	void slotEditorDestroyed(QObject *object);

	PropertyToEditorListMap  m_createdEditors;
	EditorToPropertyMap m_editorToProperty;
};

template <class Editor>
Editor *EditorFactoryPrivate<Editor>::createEditor(QtProperty *property, QWidget *parent)
{
	Editor *editor = QT_NEW Editor(parent);
	initializeEditor(property, editor);
	return editor;
}

template <class Editor>
void EditorFactoryPrivate<Editor>::initializeEditor(QtProperty *property, Editor *editor)
{
	typename PropertyToEditorListMap::iterator it = m_createdEditors.find(property);
	if (it == m_createdEditors.end())
		it = m_createdEditors.insert(property, EditorList());
	it.value().append(editor);
	m_editorToProperty.insert(editor, property);
}

template <class Editor>
void EditorFactoryPrivate<Editor>::slotEditorDestroyed(QObject *object)
{
	const typename EditorToPropertyMap::iterator ecend = m_editorToProperty.end();
	for (typename EditorToPropertyMap::iterator itEditor = m_editorToProperty.begin(); itEditor !=  ecend; ++itEditor) {
		if (itEditor.key() == object) {
			Editor *editor = itEditor.key();
			QtProperty *property = itEditor.value();
			const typename PropertyToEditorListMap::iterator pit = m_createdEditors.find(property);
			if (pit != m_createdEditors.end()) {
				pit.value().removeAll(editor);
				if (pit.value().empty())
					m_createdEditors.erase(pit);
			}
			m_editorToProperty.erase(itEditor);
			return;
		}
	}
}


/************************************************************************/
/*                                                                      */
/************************************************************************/
class QtDoubleEditSliderFactoryPrivate : public EditorFactoryPrivate<EditSlider>
{
	QtDoubleEditSliderFactory *q_ptr;
	Q_DECLARE_PUBLIC(QtDoubleEditSliderFactory)
public:

	void slotPropertyChanged(QtProperty *property, double value);
	void slotRangeChanged(QtProperty *property, double min, double max);
	void slotSingleStepChanged(QtProperty *property, double step);
	void slotDecimalsChanged(QtProperty *property, int prec);
	void slotReadOnlyChanged(QtProperty *property, bool readOnly);
	void slotSetValue(double value);
};

void QtDoubleEditSliderFactoryPrivate::slotPropertyChanged(QtProperty *property, double value)
{
	QList<EditSlider *> editors = m_createdEditors[property];
	QListIterator<EditSlider *> itEditor(m_createdEditors[property]);
	while (itEditor.hasNext()) {
		EditSlider *editor = itEditor.next();
		if (editor->value() != value) {
			editor->blockSignals(true);
			editor->setValue(value);
			editor->blockSignals(false);
		}
	}
}

void QtDoubleEditSliderFactoryPrivate::slotRangeChanged(QtProperty *property,
													 double min, double max)
{
	if (!m_createdEditors.contains(property))
		return;

	QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
	if (!manager)
		return;

	QList<EditSlider *> editors = m_createdEditors[property];
	QListIterator<EditSlider *> itEditor(editors);
	while (itEditor.hasNext()) {
		EditSlider *editor = itEditor.next();
		editor->blockSignals(true);
		editor->setRange(min, max);
		editor->setValue(manager->value(property));
		editor->blockSignals(false);
	}
}

void QtDoubleEditSliderFactoryPrivate::slotSingleStepChanged(QtProperty *property, double step)
{
	if (!m_createdEditors.contains(property))
		return;

	QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
	if (!manager)
		return;

	QList<EditSlider *> editors = m_createdEditors[property];
	QListIterator<EditSlider *> itEditor(editors);
	while (itEditor.hasNext()) {
		EditSlider *editor = itEditor.next();
		editor->blockSignals(true);
		editor->setSingleStep(step);
		editor->blockSignals(false);
	}
}

void QtDoubleEditSliderFactoryPrivate::slotReadOnlyChanged( QtProperty *property, bool readOnly)
{
	if (!m_createdEditors.contains(property))
		return;

	QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
	if (!manager)
		return;

	QListIterator<EditSlider *> itEditor(m_createdEditors[property]);
	while (itEditor.hasNext()) {
		EditSlider *editor = itEditor.next();
		editor->blockSignals(true);
		editor->setReadOnly(readOnly);
		editor->blockSignals(false);
	}
}

void QtDoubleEditSliderFactoryPrivate::slotDecimalsChanged(QtProperty *property, int prec)
{
	if (!m_createdEditors.contains(property))
		return;

	QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
	if (!manager)
		return;

	QList<EditSlider *> editors = m_createdEditors[property];
	QListIterator<EditSlider *> itEditor(editors);
	while (itEditor.hasNext()) {
		EditSlider *editor = itEditor.next();
		editor->blockSignals(true);
		editor->setDecimals(prec);
		editor->setValue(manager->value(property));
		editor->blockSignals(false);
	}
}


void QtDoubleEditSliderFactoryPrivate::slotSetValue(double value)
{
	QObject *object = q_ptr->sender();
	const QMap<EditSlider *, QtProperty *>::ConstIterator itcend = m_editorToProperty.constEnd();
	for (QMap<EditSlider *, QtProperty *>::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != itcend; ++itEditor) {
		if (itEditor.key() == object) {
			QtProperty *property = itEditor.value();
			QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
			if (!manager)
				return;
			manager->setValue(property, value);
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
QtDoubleEditSliderFactory::QtDoubleEditSliderFactory(QObject *parent/* = Q_NULLPTR*/)
	:QtAbstractEditorFactory<QtDoublePropertyManager>(parent)
{
	d_ptr = QT_NEW QtDoubleEditSliderFactoryPrivate();
	d_ptr->q_ptr = this;
}

//////////////////////////////////////////////////////////////////////////
QtDoubleEditSliderFactory::~QtDoubleEditSliderFactory()
{
	qDeleteAll(d_ptr->m_editorToProperty.keys());
	delete d_ptr;
}

//////////////////////////////////////////////////////////////////////////
void QtDoubleEditSliderFactory::connectPropertyManager(QtDoublePropertyManager *manager)
{
	connect(manager, SIGNAL(valueChanged(QtProperty *, double)),
		this, SLOT(slotPropertyChanged(QtProperty *, double)));
	connect(manager, SIGNAL(rangeChanged(QtProperty *, double, double)),
		this, SLOT(slotRangeChanged(QtProperty *, double, double)));
	connect(manager, SIGNAL(singleStepChanged(QtProperty *, double)),
		this, SLOT(slotSingleStepChanged(QtProperty *, double)));
	connect(manager, SIGNAL(decimalsChanged(QtProperty *, int)),
		this, SLOT(slotDecimalsChanged(QtProperty *, int)));
	connect(manager, SIGNAL(readOnlyChanged(QtProperty *, bool)),
		this, SLOT(slotReadOnlyChanged(QtProperty *, bool)));
}

//////////////////////////////////////////////////////////////////////////
QWidget *QtDoubleEditSliderFactory::createEditor(QtDoublePropertyManager *manager,
											  QtProperty *property, QWidget *parent)
{
	EditSlider *editor = d_ptr->createEditor(property, parent);
	editor->setSingleStep(manager->singleStep(property));
	editor->setDecimals(manager->decimals(property));
	editor->setRange(manager->minimum(property), manager->maximum(property));
	editor->setValue(manager->value(property));
	editor->setReadOnly(manager->isReadOnly(property));

	connect(editor, SIGNAL(valueChanged(double)), this, SLOT(slotSetValue(double)));
	connect(editor, SIGNAL(destroyed(QObject *)),
		this, SLOT(slotEditorDestroyed(QObject *)));
	return editor;
}

//////////////////////////////////////////////////////////////////////////
void QtDoubleEditSliderFactory::disconnectPropertyManager(QtDoublePropertyManager *manager)
{
	disconnect(manager, SIGNAL(valueChanged(QtProperty *, double)),
		this, SLOT(slotPropertyChanged(QtProperty *, double)));
	disconnect(manager, SIGNAL(rangeChanged(QtProperty *, double, double)),
		this, SLOT(slotRangeChanged(QtProperty *, double, double)));
	disconnect(manager, SIGNAL(singleStepChanged(QtProperty *, double)),
		this, SLOT(slotSingleStepChanged(QtProperty *, double)));
	disconnect(manager, SIGNAL(decimalsChanged(QtProperty *, int)),
		this, SLOT(slotDecimalsChanged(QtProperty *, int)));
	disconnect(manager, SIGNAL(readOnlyChanged(QtProperty *, bool)),
		this, SLOT(slotReadOnlyChanged(QtProperty *, bool)));
}

//////////////////////////////////////////////////////////////////////////
bool QtDoubleEditSliderFactory::setValue(QWidget* editor, value_type val) const
{
	EditSlider* slider = qobject_cast<EditSlider*>(editor);
	if (slider != NULL)
	{
		slider->setValue(val);
		return true;
	}
	return false;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
class QtIntEditSliderFactoryPrivate : public EditorFactoryPrivate<EditSlider>
{
	QtIntEditSliderFactory *q_ptr;
	Q_DECLARE_PUBLIC(QtIntEditSliderFactory)
public:

	void slotPropertyChanged(QtProperty *property, int value);
	void slotRangeChanged(QtProperty *property, int min, int max);
	void slotSingleStepChanged(QtProperty *property, int step);
	void slotReadOnlyChanged(QtProperty *property, bool readOnly);
	void slotSetValue(double value);
};

void QtIntEditSliderFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
	QList<EditSlider *> editors = m_createdEditors[property];
	QListIterator<EditSlider *> itEditor(m_createdEditors[property]);
	while (itEditor.hasNext()) {
		EditSlider *editor = itEditor.next();
		if (editor->value() != value) {
			editor->blockSignals(true);
			editor->setValue(value);
			editor->blockSignals(false);
		}
	}
}

void QtIntEditSliderFactoryPrivate::slotRangeChanged(QtProperty *property,
													 int min, int max)
{
	if (!m_createdEditors.contains(property))
		return;

	QtIntPropertyManager *manager = q_ptr->propertyManager(property);
	if (!manager)
		return;

	QList<EditSlider *> editors = m_createdEditors[property];
	QListIterator<EditSlider *> itEditor(editors);
	while (itEditor.hasNext()) {
		EditSlider *editor = itEditor.next();
		editor->blockSignals(true);
		editor->setRange(min, max);
		editor->setValue(manager->value(property));
		editor->blockSignals(false);
	}
}

void QtIntEditSliderFactoryPrivate::slotSingleStepChanged(QtProperty *property, int step)
{
	if (!m_createdEditors.contains(property))
		return;

	QtIntPropertyManager *manager = q_ptr->propertyManager(property);
	if (!manager)
		return;

	QList<EditSlider *> editors = m_createdEditors[property];
	QListIterator<EditSlider *> itEditor(editors);
	while (itEditor.hasNext()) {
		EditSlider *editor = itEditor.next();
		editor->blockSignals(true);
		editor->setSingleStep(step);
		editor->blockSignals(false);
	}
}

void QtIntEditSliderFactoryPrivate::slotReadOnlyChanged( QtProperty *property, bool readOnly)
{
	if (!m_createdEditors.contains(property))
		return;

	QtIntPropertyManager *manager = q_ptr->propertyManager(property);
	if (!manager)
		return;

	QListIterator<EditSlider *> itEditor(m_createdEditors[property]);
	while (itEditor.hasNext()) {
		EditSlider *editor = itEditor.next();
		editor->blockSignals(true);
		editor->setReadOnly(readOnly);
		editor->blockSignals(false);
	}
}

void QtIntEditSliderFactoryPrivate::slotSetValue(double dvalue)
{
	int value = (int)dvalue;
	QObject *object = q_ptr->sender();
	const QMap<EditSlider *, QtProperty *>::ConstIterator itcend = m_editorToProperty.constEnd();
	for (QMap<EditSlider *, QtProperty *>::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != itcend; ++itEditor) {
		if (itEditor.key() == object) {
			QtProperty *property = itEditor.value();
			QtIntPropertyManager *manager = q_ptr->propertyManager(property);
			if (!manager)
				return;
			manager->setValue(property, value);
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
QtIntEditSliderFactory::QtIntEditSliderFactory(QObject *parent/* = Q_NULLPTR*/)
	:QtAbstractEditorFactory<QtIntPropertyManager>(parent)
{
	d_ptr = QT_NEW QtIntEditSliderFactoryPrivate();
	d_ptr->q_ptr = this;
}

//////////////////////////////////////////////////////////////////////////
QtIntEditSliderFactory::~QtIntEditSliderFactory()
{
	qDeleteAll(d_ptr->m_editorToProperty.keys());
	delete d_ptr;
}

//////////////////////////////////////////////////////////////////////////
void QtIntEditSliderFactory::connectPropertyManager(QtIntPropertyManager *manager)
{
	connect(manager, SIGNAL(valueChanged(QtProperty *, int)),
		this, SLOT(slotPropertyChanged(QtProperty *, int)));
	connect(manager, SIGNAL(rangeChanged(QtProperty *, int, int)),
		this, SLOT(slotRangeChanged(QtProperty *, int, int)));
	connect(manager, SIGNAL(singleStepChanged(QtProperty *, int)),
		this, SLOT(slotSingleStepChanged(QtProperty *, int)));
	connect(manager, SIGNAL(readOnlyChanged(QtProperty *, bool)),
		this, SLOT(slotReadOnlyChanged(QtProperty *, bool)));
}

//////////////////////////////////////////////////////////////////////////
QWidget *QtIntEditSliderFactory::createEditor(QtIntPropertyManager *manager,
											  QtProperty *property, QWidget *parent)
{
	EditSlider *editor = d_ptr->createEditor(property, parent);
	editor->setSingleStep(manager->singleStep(property));
	editor->setDecimals(0);
	editor->setRange(manager->minimum(property), manager->maximum(property));
	editor->setValue(manager->value(property));
	editor->setReadOnly(manager->isReadOnly(property));

	connect(editor, SIGNAL(valueChanged(double)), this, SLOT(slotSetValue(double)));
	connect(editor, SIGNAL(destroyed(QObject *)),
		this, SLOT(slotEditorDestroyed(QObject *)));
	return editor;
}

//////////////////////////////////////////////////////////////////////////
void QtIntEditSliderFactory::disconnectPropertyManager(QtIntPropertyManager *manager)
{
	disconnect(manager, SIGNAL(valueChanged(QtProperty *, int)),
		this, SLOT(slotPropertyChanged(QtProperty *, int)));
	disconnect(manager, SIGNAL(rangeChanged(QtProperty *, int, int)),
		this, SLOT(slotRangeChanged(QtProperty *, int, int)));
	disconnect(manager, SIGNAL(singleStepChanged(QtProperty *, int)),
		this, SLOT(slotSingleStepChanged(QtProperty *, int)));
	disconnect(manager, SIGNAL(readOnlyChanged(QtProperty *, bool)),
		this, SLOT(slotReadOnlyChanged(QtProperty *, bool)));
}

//////////////////////////////////////////////////////////////////////////
bool QtIntEditSliderFactory::setValue(QWidget* editor, value_type val) const
{
	EditSlider* slider = qobject_cast<EditSlider*>(editor);
	if (slider != NULL)
	{
		slider->setValue((double)val);
		return true;
	}
	return false;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
class QtEnumSliderFactoryPrivate : public EditorFactoryPrivate<EnumSlider>
{
	QtEnumSliderFactory *q_ptr;
	Q_DECLARE_PUBLIC(QtEnumSliderFactory)
public:

	void slotPropertyChanged(QtProperty *property, int value);
	void slotEnumNamesChanged(QtProperty *property, const QStringList &);
	void slotEnumIconsChanged(QtProperty *property, const QMap<int, QIcon> &);
	void slotSetValue(int value);
};

void QtEnumSliderFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
	if (!m_createdEditors.contains(property))
		return;

	QListIterator<EnumSlider *> itEditor(m_createdEditors[property]);
	while (itEditor.hasNext()) {
		EnumSlider *editor = itEditor.next();
		editor->blockSignals(true);
		editor->setCurrentIndex(value);
		editor->blockSignals(false);
	}
}

void QtEnumSliderFactoryPrivate::slotEnumNamesChanged(QtProperty *property,
													  const QStringList &enumNames)
{
	if (!m_createdEditors.contains(property))
		return;

	QtEnumPropertyManager *manager = q_ptr->propertyManager(property);
	if (!manager)
		return;

	QMap<int, QIcon> enumIcons = manager->enumIcons(property);

	QListIterator<EnumSlider *> itEditor(m_createdEditors[property]);
	while (itEditor.hasNext()) {
		EnumSlider *editor = itEditor.next();
		editor->blockSignals(true);
		editor->clear();
		editor->addItems(enumNames);
		const int nameCount = enumNames.count();
		for (int i = 0; i < nameCount; i++)
			editor->setItemIcon(i, enumIcons.value(i));
		editor->setCurrentIndex(manager->value(property));
		editor->blockSignals(false);
	}
}

void QtEnumSliderFactoryPrivate::slotEnumIconsChanged(QtProperty *property,
													  const QMap<int, QIcon> &enumIcons)
{
	if (!m_createdEditors.contains(property))
		return;

	QtEnumPropertyManager *manager = q_ptr->propertyManager(property);
	if (!manager)
		return;

	const QStringList enumNames = manager->enumNames(property);
	QListIterator<EnumSlider *> itEditor(m_createdEditors[property]);
	while (itEditor.hasNext()) {
		EnumSlider *editor = itEditor.next();
		editor->blockSignals(true);
		const int nameCount = enumNames.count();
		for (int i = 0; i < nameCount; i++)
			editor->setItemIcon(i, enumIcons.value(i));
		editor->setCurrentIndex(manager->value(property));
		editor->blockSignals(false);
	}
}

void QtEnumSliderFactoryPrivate::slotSetValue(int value)
{
	QObject *object = q_ptr->sender();
	const  QMap<EnumSlider *, QtProperty *>::ConstIterator ecend = m_editorToProperty.constEnd();
	for (QMap<EnumSlider *, QtProperty *>::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
		if (itEditor.key() == object) {
			QtProperty *property = itEditor.value();
			QtEnumPropertyManager *manager = q_ptr->propertyManager(property);
			if (!manager)
				return;
			manager->setValue(property, value);
			return;
		}
}

//////////////////////////////////////////////////////////////////////////
QtEnumSliderFactory::QtEnumSliderFactory(QObject *parent)
	: QtAbstractEditorFactory<QtEnumPropertyManager>(parent)
{
	d_ptr = QT_NEW QtEnumSliderFactoryPrivate();
	d_ptr->q_ptr = this;

}

//////////////////////////////////////////////////////////////////////////
QtEnumSliderFactory::~QtEnumSliderFactory()
{
	qDeleteAll(d_ptr->m_editorToProperty.keys());
	delete d_ptr;
}

//////////////////////////////////////////////////////////////////////////
void QtEnumSliderFactory::connectPropertyManager(QtEnumPropertyManager *manager)
{
	connect(manager, SIGNAL(valueChanged(QtProperty *, int)),
		this, SLOT(slotPropertyChanged(QtProperty *, int)));
	connect(manager, SIGNAL(enumNamesChanged(QtProperty *, const QStringList &)),
		this, SLOT(slotEnumNamesChanged(QtProperty *, const QStringList &)));
}

//////////////////////////////////////////////////////////////////////////
QWidget *QtEnumSliderFactory::createEditor(QtEnumPropertyManager *manager, QtProperty *property,
										   QWidget *parent)
{
	EnumSlider *editor = d_ptr->createEditor(property, parent);
	//editor->setSizeAdjustPolicy(EnumSlider::AdjustToMinimumContentsLengthWithIcon);
	//editor->setMinimumContentsLength(1);
	//editor->view()->setTextElideMode(Qt::ElideRight);
	QStringList enumNames = manager->enumNames(property);
	editor->addItems(enumNames);
	QMap<int, QIcon> enumIcons = manager->enumIcons(property);
	const int enumNamesCount = enumNames.count();
	for (int i = 0; i < enumNamesCount; i++)
		editor->setItemIcon(i, enumIcons.value(i));
	editor->setCurrentIndex(manager->value(property));

	connect(editor, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetValue(int)));
	connect(editor, SIGNAL(destroyed(QObject *)),
		this, SLOT(slotEditorDestroyed(QObject *)));
	return editor;
}

//////////////////////////////////////////////////////////////////////////
void QtEnumSliderFactory::disconnectPropertyManager(QtEnumPropertyManager *manager)
{
	disconnect(manager, SIGNAL(valueChanged(QtProperty *, int)),
		this, SLOT(slotPropertyChanged(QtProperty *, int)));
	disconnect(manager, SIGNAL(enumNamesChanged(QtProperty *, const QStringList &)),
		this, SLOT(slotEnumNamesChanged(QtProperty *, const QStringList &)));
}


#include "moc_BladeExtension.cpp"
#include "BladeExtension.moc"