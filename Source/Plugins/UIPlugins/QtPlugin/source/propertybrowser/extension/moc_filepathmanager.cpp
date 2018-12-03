/****************************************************************************
** Meta object code from reading C++ file 'filepathmanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <QtPluginPCH.h>
#include "filepathmanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'filepathmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_FilePathManager_t {
    QByteArrayData data[20];
    char stringdata0[197];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FilePathManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FilePathManager_t qt_meta_stringdata_FilePathManager = {
    {
QT_MOC_LITERAL(0, 0, 15), // "FilePathManager"
QT_MOC_LITERAL(1, 16, 12), // "valueChanged"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 42, 8), // "property"
QT_MOC_LITERAL(5, 51, 3), // "val"
QT_MOC_LITERAL(6, 55, 13), // "filterChanged"
QT_MOC_LITERAL(7, 69, 3), // "fil"
QT_MOC_LITERAL(8, 73, 15), // "readOnlyChanged"
QT_MOC_LITERAL(9, 89, 8), // "readOnly"
QT_MOC_LITERAL(10, 98, 11), // "iconChanged"
QT_MOC_LITERAL(11, 110, 4), // "icon"
QT_MOC_LITERAL(12, 115, 11), // "typeChanged"
QT_MOC_LITERAL(13, 127, 17), // "EFilePropertyType"
QT_MOC_LITERAL(14, 145, 4), // "type"
QT_MOC_LITERAL(15, 150, 8), // "setValue"
QT_MOC_LITERAL(16, 159, 9), // "setFilter"
QT_MOC_LITERAL(17, 169, 11), // "setReadOnly"
QT_MOC_LITERAL(18, 181, 7), // "setIcon"
QT_MOC_LITERAL(19, 189, 7) // "setType"

    },
    "FilePathManager\0valueChanged\0\0QtProperty*\0"
    "property\0val\0filterChanged\0fil\0"
    "readOnlyChanged\0readOnly\0iconChanged\0"
    "icon\0typeChanged\0EFilePropertyType\0"
    "type\0setValue\0setFilter\0setReadOnly\0"
    "setIcon\0setType"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FilePathManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   64,    2, 0x06 /* Public */,
       6,    2,   69,    2, 0x06 /* Public */,
       8,    2,   74,    2, 0x06 /* Public */,
      10,    2,   79,    2, 0x06 /* Public */,
      12,    2,   84,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      15,    2,   89,    2, 0x0a /* Public */,
      16,    2,   94,    2, 0x0a /* Public */,
      17,    2,   99,    2, 0x0a /* Public */,
      18,    2,  104,    2, 0x0a /* Public */,
      19,    2,  109,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,    7,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    4,    9,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QIcon,    4,   11,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 13,    4,   14,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,    7,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    4,    9,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QIcon,    4,   11,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 13,    4,   14,

       0        // eod
};

void FilePathManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        FilePathManager *_t = static_cast<FilePathManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 1: _t->filterChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 2: _t->readOnlyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 3: _t->iconChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QIcon(*)>(_a[2]))); break;
        case 4: _t->typeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< EFilePropertyType(*)>(_a[2]))); break;
        case 5: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 6: _t->setFilter((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 7: _t->setReadOnly((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 8: _t->setIcon((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QIcon(*)>(_a[2]))); break;
        case 9: _t->setType((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< EFilePropertyType(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (FilePathManager::*_t)(QtProperty * , const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&FilePathManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (FilePathManager::*_t)(QtProperty * , const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&FilePathManager::filterChanged)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (FilePathManager::*_t)(QtProperty * , bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&FilePathManager::readOnlyChanged)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (FilePathManager::*_t)(QtProperty * , const QIcon & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&FilePathManager::iconChanged)) {
                *result = 3;
                return;
            }
        }
        {
            typedef void (FilePathManager::*_t)(QtProperty * , EFilePropertyType );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&FilePathManager::typeChanged)) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject FilePathManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_FilePathManager.data,
      qt_meta_data_FilePathManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *FilePathManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FilePathManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_FilePathManager.stringdata0))
        return static_cast<void*>(const_cast< FilePathManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int FilePathManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void FilePathManager::valueChanged(QtProperty * _t1, const QString & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void FilePathManager::filterChanged(QtProperty * _t1, const QString & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void FilePathManager::readOnlyChanged(QtProperty * _t1, bool _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void FilePathManager::iconChanged(QtProperty * _t1, const QIcon & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void FilePathManager::typeChanged(QtProperty * _t1, EFilePropertyType _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_END_MOC_NAMESPACE
