/****************************************************************************
** Meta object code from reading C++ file 'ArgumentDialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ArgumentDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ArgumentDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_teaching__ArgumentDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      26,   25,   25,   25, 0x08,
      51,   25,   25,   25, 0x08,
      73,   25,   25,   25, 0x08,
      86,   25,   25,   25, 0x08,
     102,   25,   25,   25, 0x08,
     114,   25,   25,   25, 0x08,
     128,   25,   25,   25, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_teaching__ArgumentDialog[] = {
    "teaching::ArgumentDialog\0\0"
    "actionSelectionChanged()\0argSelectionChanged()\0"
    "addClicked()\0deleteClicked()\0upClicked()\0"
    "downClicked()\0oKClicked()\0"
};

void teaching::ArgumentDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ArgumentDialog *_t = static_cast<ArgumentDialog *>(_o);
        switch (_id) {
        case 0: _t->actionSelectionChanged(); break;
        case 1: _t->argSelectionChanged(); break;
        case 2: _t->addClicked(); break;
        case 3: _t->deleteClicked(); break;
        case 4: _t->upClicked(); break;
        case 5: _t->downClicked(); break;
        case 6: _t->oKClicked(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData teaching::ArgumentDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject teaching::ArgumentDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_teaching__ArgumentDialog,
      qt_meta_data_teaching__ArgumentDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &teaching::ArgumentDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *teaching::ArgumentDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *teaching::ArgumentDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_teaching__ArgumentDialog))
        return static_cast<void*>(const_cast< ArgumentDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int teaching::ArgumentDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
