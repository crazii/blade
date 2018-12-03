/****************************************************************************
** Meta object code from reading C++ file 'fileeditfactory.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <QtPluginPCH.h>
#include "fileeditfactory.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'fileeditfactory.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_FileEditFactory_t {
    QByteArrayData data[27];
    char stringdata0[305];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FileEditFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FileEditFactory_t qt_meta_stringdata_FileEditFactory = {
    {
QT_MOC_LITERAL(0, 0, 15), // "FileEditFactory"
QT_MOC_LITERAL(1, 16, 23), // "propertyCallbackInvoked"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 53, 8), // "property"
QT_MOC_LITERAL(5, 62, 8), // "QString&"
QT_MOC_LITERAL(6, 71, 4), // "path"
QT_MOC_LITERAL(7, 76, 5), // "bool&"
QT_MOC_LITERAL(8, 82, 4), // "done"
QT_MOC_LITERAL(9, 87, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(10, 107, 5), // "value"
QT_MOC_LITERAL(11, 113, 17), // "slotFilterChanged"
QT_MOC_LITERAL(12, 131, 6), // "filter"
QT_MOC_LITERAL(13, 138, 19), // "slotReadOnlyChanged"
QT_MOC_LITERAL(14, 158, 8), // "readOnly"
QT_MOC_LITERAL(15, 167, 15), // "slotIconChanged"
QT_MOC_LITERAL(16, 183, 4), // "icon"
QT_MOC_LITERAL(17, 188, 15), // "slotTypeChanged"
QT_MOC_LITERAL(18, 204, 17), // "EFilePropertyType"
QT_MOC_LITERAL(19, 222, 4), // "type"
QT_MOC_LITERAL(20, 227, 12), // "slotSetValue"
QT_MOC_LITERAL(21, 240, 19), // "slotEditorDestroyed"
QT_MOC_LITERAL(22, 260, 6), // "object"
QT_MOC_LITERAL(23, 267, 12), // "slotCallback"
QT_MOC_LITERAL(24, 280, 9), // "FileEdit*"
QT_MOC_LITERAL(25, 290, 4), // "edit"
QT_MOC_LITERAL(26, 295, 9) // "succeeded"

    },
    "FileEditFactory\0propertyCallbackInvoked\0"
    "\0QtProperty*\0property\0QString&\0path\0"
    "bool&\0done\0slotPropertyChanged\0value\0"
    "slotFilterChanged\0filter\0slotReadOnlyChanged\0"
    "readOnly\0slotIconChanged\0icon\0"
    "slotTypeChanged\0EFilePropertyType\0"
    "type\0slotSetValue\0slotEditorDestroyed\0"
    "object\0slotCallback\0FileEdit*\0edit\0"
    "succeeded"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FileEditFactory[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   59,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    2,   66,    2, 0x08 /* Private */,
      11,    2,   71,    2, 0x08 /* Private */,
      13,    2,   76,    2, 0x08 /* Private */,
      15,    2,   81,    2, 0x08 /* Private */,
      17,    2,   86,    2, 0x08 /* Private */,
      20,    1,   91,    2, 0x08 /* Private */,
      21,    1,   94,    2, 0x08 /* Private */,
      23,    3,   97,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, 0x80000000 | 7,    4,    6,    8,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,   10,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,   12,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    4,   14,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QIcon,    4,   16,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 18,    4,   19,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void, QMetaType::QObjectStar,   22,
    QMetaType::Void, 0x80000000 | 24, 0x80000000 | 5, 0x80000000 | 7,   25,    6,   26,

       0        // eod
};

void FileEditFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        FileEditFactory *_t = static_cast<FileEditFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->propertyCallbackInvoked((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 1: _t->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 2: _t->slotFilterChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 3: _t->slotReadOnlyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: _t->slotIconChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QIcon(*)>(_a[2]))); break;
        case 5: _t->slotTypeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< EFilePropertyType(*)>(_a[2]))); break;
        case 6: _t->slotSetValue((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        case 8: _t->slotCallback((*reinterpret_cast< FileEdit*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (FileEditFactory::*_t)(QtProperty * , QString & , bool & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&FileEditFactory::propertyCallbackInvoked)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject FileEditFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<FilePathManager>::staticMetaObject, qt_meta_stringdata_FileEditFactory.data,
      qt_meta_data_FileEditFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *FileEditFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FileEditFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_FileEditFactory.stringdata0))
        return static_cast<void*>(const_cast< FileEditFactory*>(this));
    return QtAbstractEditorFactory<FilePathManager>::qt_metacast(_clname);
}

int FileEditFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<FilePathManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void FileEditFactory::propertyCallbackInvoked(QtProperty * _t1, QString & _t2, bool & _t3)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
