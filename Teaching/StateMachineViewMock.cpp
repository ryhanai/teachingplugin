/****************************************************************************
** Meta object code from reading C++ file 'StateMachineView.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "StateMachineView.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'StateMachineView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_teaching__StateMachineViewImpl[] = {

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
      32,   31,   31,   31, 0x08,
      45,   31,   31,   31, 0x08,
      59,   31,   31,   31, 0x08,
      75,   31,   31,   31, 0x08,
      89,   31,   31,   31, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_teaching__StateMachineViewImpl[] = {
    "teaching::StateMachineViewImpl\0\0"
    "setClicked()\0modeChanged()\0deleteClicked()\0"
    "editClicked()\0runClicked()\0"
};

void teaching::StateMachineViewImpl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        StateMachineViewImpl *_t = static_cast<StateMachineViewImpl *>(_o);
        switch (_id) {
        case 0: _t->setClicked(); break;
        case 1: _t->modeChanged(); break;
        case 2: _t->deleteClicked(); break;
        case 3: _t->editClicked(); break;
        case 4: _t->runClicked(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData teaching::StateMachineViewImpl::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject teaching::StateMachineViewImpl::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_teaching__StateMachineViewImpl,
      qt_meta_data_teaching__StateMachineViewImpl, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &teaching::StateMachineViewImpl::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *teaching::StateMachineViewImpl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *teaching::StateMachineViewImpl::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_teaching__StateMachineViewImpl))
        return static_cast<void*>(const_cast< StateMachineViewImpl*>(this));
    return QWidget::qt_metacast(_clname);
}

int teaching::StateMachineViewImpl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
