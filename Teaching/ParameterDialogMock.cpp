/****************************************************************************
** Meta object code from reading C++ file 'ParameterDialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ParameterDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ParameterDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_teaching__ParameterDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      27,   26,   26,   26, 0x08,
      57,   51,   26,   26, 0x08,
      83,   26,   26,   26, 0x08,
     101,   26,   26,   26, 0x08,
     122,   26,   26,   26, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_teaching__ParameterDialog[] = {
    "teaching::ParameterDialog\0\0"
    "paramSelectionChanged()\0index\0"
    "typeSelectionChanged(int)\0addParamClicked()\0"
    "deleteParamClicked()\0oKClicked()\0"
};

void teaching::ParameterDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ParameterDialog *_t = static_cast<ParameterDialog *>(_o);
        switch (_id) {
        case 0: _t->paramSelectionChanged(); break;
        case 1: _t->typeSelectionChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->addParamClicked(); break;
        case 3: _t->deleteParamClicked(); break;
        case 4: _t->oKClicked(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData teaching::ParameterDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject teaching::ParameterDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_teaching__ParameterDialog,
      qt_meta_data_teaching__ParameterDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &teaching::ParameterDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *teaching::ParameterDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *teaching::ParameterDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_teaching__ParameterDialog))
        return static_cast<void*>(const_cast< ParameterDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int teaching::ParameterDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
