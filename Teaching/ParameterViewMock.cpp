/****************************************************************************
** Meta object code from reading C++ file 'ParameterView.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ParameterView.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ParameterView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_teaching__ModelParameterGroup[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      31,   30,   30,   30, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_teaching__ModelParameterGroup[] = {
    "teaching::ModelParameterGroup\0\0"
    "modelPositionChanged()\0"
};

void teaching::ModelParameterGroup::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ModelParameterGroup *_t = static_cast<ModelParameterGroup *>(_o);
        switch (_id) {
        case 0: _t->modelPositionChanged(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData teaching::ModelParameterGroup::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject teaching::ModelParameterGroup::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_teaching__ModelParameterGroup,
      qt_meta_data_teaching__ModelParameterGroup, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &teaching::ModelParameterGroup::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *teaching::ModelParameterGroup::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *teaching::ModelParameterGroup::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_teaching__ModelParameterGroup))
        return static_cast<void*>(const_cast< ModelParameterGroup*>(this));
    return QWidget::qt_metacast(_clname);
}

int teaching::ModelParameterGroup::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_teaching__ParameterViewImpl[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      29,   28,   28,   28, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_teaching__ParameterViewImpl[] = {
    "teaching::ParameterViewImpl\0\0editClicked()\0"
};

void teaching::ParameterViewImpl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ParameterViewImpl *_t = static_cast<ParameterViewImpl *>(_o);
        switch (_id) {
        case 0: _t->editClicked(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData teaching::ParameterViewImpl::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject teaching::ParameterViewImpl::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_teaching__ParameterViewImpl,
      qt_meta_data_teaching__ParameterViewImpl, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &teaching::ParameterViewImpl::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *teaching::ParameterViewImpl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *teaching::ParameterViewImpl::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_teaching__ParameterViewImpl))
        return static_cast<void*>(const_cast< ParameterViewImpl*>(this));
    return QWidget::qt_metacast(_clname);
}

int teaching::ParameterViewImpl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
