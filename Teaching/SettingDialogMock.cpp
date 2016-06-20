/****************************************************************************
** Meta object code from reading C++ file 'SettingDialog.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "SettingDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SettingDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_teaching__SettingDialog[] = {

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
      25,   24,   24,   24, 0x08,
      40,   24,   24,   24, 0x08,
      62,   24,   24,   24, 0x08,
      78,   24,   24,   24, 0x08,
      90,   24,   24,   24, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_teaching__SettingDialog[] = {
    "teaching::SettingDialog\0\0refDBClicked()\0"
    "appSelectionChanged()\0refAppClicked()\0"
    "oKClicked()\0cancelClicked()\0"
};

void teaching::SettingDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SettingDialog *_t = static_cast<SettingDialog *>(_o);
        switch (_id) {
        case 0: _t->refDBClicked(); break;
        case 1: _t->appSelectionChanged(); break;
        case 2: _t->refAppClicked(); break;
        case 3: _t->oKClicked(); break;
        case 4: _t->cancelClicked(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData teaching::SettingDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject teaching::SettingDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_teaching__SettingDialog,
      qt_meta_data_teaching__SettingDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &teaching::SettingDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *teaching::SettingDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *teaching::SettingDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_teaching__SettingDialog))
        return static_cast<void*>(const_cast< SettingDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int teaching::SettingDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
