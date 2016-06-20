/****************************************************************************
** Meta object code from reading C++ file 'MetaDataView.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "MetaDataView.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MetaDataView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_teaching__MetaDataViewImpl[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      28,   27,   27,   27, 0x08,
      52,   27,   27,   27, 0x08,
      65,   27,   27,   27, 0x08,
      83,   27,   27,   27, 0x08,
     104,   27,   27,   27, 0x08,
     127,   27,   27,   27, 0x08,
     147,   27,   27,   27, 0x08,
     165,   27,   27,   27, 0x08,
     185,   27,   27,   27, 0x08,
     206,   27,   27,   27, 0x08,
     225,   27,   27,   27, 0x08,
     246,   27,   27,   27, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_teaching__MetaDataViewImpl[] = {
    "teaching::MetaDataViewImpl\0\0"
    "modelSelectionChanged()\0refClicked()\0"
    "addModelClicked()\0deleteModelClicked()\0"
    "modelPositionChanged()\0fileOutputClicked()\0"
    "fileShowClicked()\0fileDeleteClicked()\0"
    "imageOutputClicked()\0imageShowClicked()\0"
    "imageDeleteClicked()\0processFinished()\0"
};

void teaching::MetaDataViewImpl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MetaDataViewImpl *_t = static_cast<MetaDataViewImpl *>(_o);
        switch (_id) {
        case 0: _t->modelSelectionChanged(); break;
        case 1: _t->refClicked(); break;
        case 2: _t->addModelClicked(); break;
        case 3: _t->deleteModelClicked(); break;
        case 4: _t->modelPositionChanged(); break;
        case 5: _t->fileOutputClicked(); break;
        case 6: _t->fileShowClicked(); break;
        case 7: _t->fileDeleteClicked(); break;
        case 8: _t->imageOutputClicked(); break;
        case 9: _t->imageShowClicked(); break;
        case 10: _t->imageDeleteClicked(); break;
        case 11: _t->processFinished(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData teaching::MetaDataViewImpl::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject teaching::MetaDataViewImpl::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_teaching__MetaDataViewImpl,
      qt_meta_data_teaching__MetaDataViewImpl, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &teaching::MetaDataViewImpl::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *teaching::MetaDataViewImpl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *teaching::MetaDataViewImpl::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_teaching__MetaDataViewImpl))
        return static_cast<void*>(const_cast< MetaDataViewImpl*>(this));
    return QWidget::qt_metacast(_clname);
}

int teaching::MetaDataViewImpl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
