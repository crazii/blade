/****************************************************************************
** Meta object code from reading C++ file 'BladeExtension.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "QtPluginPCH.h"
#include "BladeExtension.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'BladeExtension.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QtDoubleEditSliderFactory_t {
    QByteArrayData data[10];
    char stringdata0[171];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtDoubleEditSliderFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtDoubleEditSliderFactory_t qt_meta_stringdata_QtDoubleEditSliderFactory = {
    {
QT_MOC_LITERAL(0, 0, 25), // "QtDoubleEditSliderFactory"
QT_MOC_LITERAL(1, 26, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 46, 0), // ""
QT_MOC_LITERAL(3, 47, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 59, 16), // "slotRangeChanged"
QT_MOC_LITERAL(5, 76, 21), // "slotSingleStepChanged"
QT_MOC_LITERAL(6, 98, 19), // "slotDecimalsChanged"
QT_MOC_LITERAL(7, 118, 19), // "slotReadOnlyChanged"
QT_MOC_LITERAL(8, 138, 12), // "slotSetValue"
QT_MOC_LITERAL(9, 151, 19) // "slotEditorDestroyed"

    },
    "QtDoubleEditSliderFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotRangeChanged\0"
    "slotSingleStepChanged\0slotDecimalsChanged\0"
    "slotReadOnlyChanged\0slotSetValue\0"
    "slotEditorDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtDoubleEditSliderFactory[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   49,    2, 0x08 /* Private */,
       4,    3,   54,    2, 0x08 /* Private */,
       5,    2,   61,    2, 0x08 /* Private */,
       6,    2,   66,    2, 0x08 /* Private */,
       7,    2,   71,    2, 0x08 /* Private */,
       8,    1,   76,    2, 0x08 /* Private */,
       9,    1,   79,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double, QMetaType::Double,    2,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    2,    2,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,

       0        // eod
};

void QtDoubleEditSliderFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtDoubleEditSliderFactory *_t = static_cast<QtDoubleEditSliderFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 2: _t->d_func()->slotSingleStepChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotDecimalsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->d_func()->slotReadOnlyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 5: _t->d_func()->slotSetValue((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtDoubleEditSliderFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtDoublePropertyManager>::staticMetaObject, qt_meta_stringdata_QtDoubleEditSliderFactory.data,
      qt_meta_data_QtDoubleEditSliderFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtDoubleEditSliderFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtDoubleEditSliderFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtDoubleEditSliderFactory.stringdata0))
        return static_cast<void*>(const_cast< QtDoubleEditSliderFactory*>(this));
    return QtAbstractEditorFactory<QtDoublePropertyManager>::qt_metacast(_clname);
}

int QtDoubleEditSliderFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtDoublePropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
struct qt_meta_stringdata_QtIntEditSliderFactory_t {
    QByteArrayData data[9];
    char stringdata0[148];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtIntEditSliderFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtIntEditSliderFactory_t qt_meta_stringdata_QtIntEditSliderFactory = {
    {
QT_MOC_LITERAL(0, 0, 22), // "QtIntEditSliderFactory"
QT_MOC_LITERAL(1, 23, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 43, 0), // ""
QT_MOC_LITERAL(3, 44, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 56, 16), // "slotRangeChanged"
QT_MOC_LITERAL(5, 73, 21), // "slotSingleStepChanged"
QT_MOC_LITERAL(6, 95, 19), // "slotReadOnlyChanged"
QT_MOC_LITERAL(7, 115, 12), // "slotSetValue"
QT_MOC_LITERAL(8, 128, 19) // "slotEditorDestroyed"

    },
    "QtIntEditSliderFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotRangeChanged\0"
    "slotSingleStepChanged\0slotReadOnlyChanged\0"
    "slotSetValue\0slotEditorDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtIntEditSliderFactory[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   44,    2, 0x08 /* Private */,
       4,    3,   49,    2, 0x08 /* Private */,
       5,    2,   56,    2, 0x08 /* Private */,
       6,    2,   61,    2, 0x08 /* Private */,
       7,    1,   66,    2, 0x08 /* Private */,
       8,    1,   69,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int, QMetaType::Int,    2,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    2,    2,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,

       0        // eod
};

void QtIntEditSliderFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtIntEditSliderFactory *_t = static_cast<QtIntEditSliderFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->d_func()->slotSingleStepChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotReadOnlyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: _t->d_func()->slotSetValue((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 5: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtIntEditSliderFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtIntPropertyManager>::staticMetaObject, qt_meta_stringdata_QtIntEditSliderFactory.data,
      qt_meta_data_QtIntEditSliderFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtIntEditSliderFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtIntEditSliderFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtIntEditSliderFactory.stringdata0))
        return static_cast<void*>(const_cast< QtIntEditSliderFactory*>(this));
    return QtAbstractEditorFactory<QtIntPropertyManager>::qt_metacast(_clname);
}

int QtIntEditSliderFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtIntPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
struct qt_meta_stringdata_QtEnumSliderFactory_t {
    QByteArrayData data[9];
    char stringdata0[144];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtEnumSliderFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtEnumSliderFactory_t qt_meta_stringdata_QtEnumSliderFactory = {
    {
QT_MOC_LITERAL(0, 0, 19), // "QtEnumSliderFactory"
QT_MOC_LITERAL(1, 20, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 53, 20), // "slotEnumNamesChanged"
QT_MOC_LITERAL(5, 74, 20), // "slotEnumIconsChanged"
QT_MOC_LITERAL(6, 95, 15), // "QMap<int,QIcon>"
QT_MOC_LITERAL(7, 111, 12), // "slotSetValue"
QT_MOC_LITERAL(8, 124, 19) // "slotEditorDestroyed"

    },
    "QtEnumSliderFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotEnumNamesChanged\0"
    "slotEnumIconsChanged\0QMap<int,QIcon>\0"
    "slotSetValue\0slotEditorDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtEnumSliderFactory[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   39,    2, 0x08 /* Private */,
       4,    2,   44,    2, 0x08 /* Private */,
       5,    2,   49,    2, 0x08 /* Private */,
       7,    1,   54,    2, 0x08 /* Private */,
       8,    1,   57,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QStringList,    2,    2,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 6,    2,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,

       0        // eod
};

void QtEnumSliderFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtEnumSliderFactory *_t = static_cast<QtEnumSliderFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotEnumNamesChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotEnumIconsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QMap<int,QIcon>(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotSetValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtEnumSliderFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtEnumPropertyManager>::staticMetaObject, qt_meta_stringdata_QtEnumSliderFactory.data,
      qt_meta_data_QtEnumSliderFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtEnumSliderFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtEnumSliderFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtEnumSliderFactory.stringdata0))
        return static_cast<void*>(const_cast< QtEnumSliderFactory*>(this));
    return QtAbstractEditorFactory<QtEnumPropertyManager>::qt_metacast(_clname);
}

int QtEnumSliderFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtEnumPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
