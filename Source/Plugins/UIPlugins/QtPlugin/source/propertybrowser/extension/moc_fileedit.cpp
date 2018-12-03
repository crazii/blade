/****************************************************************************
** Meta object code from reading C++ file 'fileedit.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <QtPluginPCH.h>
#include "fileedit.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'fileedit.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_FileEdit_t {
    QByteArrayData data[11];
    char stringdata0[100];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FileEdit_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FileEdit_t qt_meta_stringdata_FileEdit = {
    {
QT_MOC_LITERAL(0, 0, 8), // "FileEdit"
QT_MOC_LITERAL(1, 9, 15), // "filePathChanged"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 8), // "filePath"
QT_MOC_LITERAL(4, 35, 15), // "callbackInvoked"
QT_MOC_LITERAL(5, 51, 9), // "FileEdit*"
QT_MOC_LITERAL(6, 61, 4), // "edit"
QT_MOC_LITERAL(7, 66, 8), // "QString&"
QT_MOC_LITERAL(8, 75, 5), // "bool&"
QT_MOC_LITERAL(9, 81, 4), // "done"
QT_MOC_LITERAL(10, 86, 13) // "buttonClicked"

    },
    "FileEdit\0filePathChanged\0\0filePath\0"
    "callbackInvoked\0FileEdit*\0edit\0QString&\0"
    "bool&\0done\0buttonClicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FileEdit[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       4,    3,   32,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    0,   39,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 7, 0x80000000 | 8,    6,    3,    9,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void FileEdit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        FileEdit *_t = static_cast<FileEdit *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->filePathChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->callbackInvoked((*reinterpret_cast< FileEdit*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 2: _t->buttonClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< FileEdit* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (FileEdit::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&FileEdit::filePathChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (FileEdit::*_t)(FileEdit * , QString & , bool & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&FileEdit::callbackInvoked)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject FileEdit::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_FileEdit.data,
      qt_meta_data_FileEdit,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *FileEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FileEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_FileEdit.stringdata0))
        return static_cast<void*>(const_cast< FileEdit*>(this));
    return QWidget::qt_metacast(_clname);
}

int FileEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void FileEdit::filePathChanged(const QString & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void FileEdit::callbackInvoked(FileEdit * _t1, QString & _t2, bool & _t3)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
