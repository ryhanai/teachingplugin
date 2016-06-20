/****************************************************************************
** Meta object code from reading C++ file 'FlowView.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "FlowView.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FlowView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_teaching__FlowViewImpl[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   23,   23,   23, 0x08,
      47,   23,   23,   23, 0x08,
      63,   23,   23,   23, 0x08,
      80,   23,   23,   23, 0x08,
     100,   23,   23,   23, 0x08,
     120,   23,   23,   23, 0x08,
     136,   23,   23,   23, 0x08,
     154,   23,   23,   23, 0x08,
     171,   23,   23,   23, 0x08,
     188,   23,   23,   23, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_teaching__FlowViewImpl[] = {
    "teaching::FlowViewImpl\0\0flowSelectionChanged()\0"
    "searchClicked()\0newFlowClicked()\0"
    "registFlowClicked()\0deleteTaskClicked()\0"
    "upTaskClicked()\0downTaskClicked()\0"
    "runFlowClicked()\0runTaskClicked()\0"
    "initPosClicked()\0"
};

void teaching::FlowViewImpl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        FlowViewImpl *_t = static_cast<FlowViewImpl *>(_o);
        switch (_id) {
        case 0: _t->flowSelectionChanged(); break;
        case 1: _t->searchClicked(); break;
        case 2: _t->newFlowClicked(); break;
        case 3: _t->registFlowClicked(); break;
        case 4: _t->deleteTaskClicked(); break;
        case 5: _t->upTaskClicked(); break;
        case 6: _t->downTaskClicked(); break;
        case 7: _t->runFlowClicked(); break;
        case 8: _t->runTaskClicked(); break;
        case 9: _t->initPosClicked(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData teaching::FlowViewImpl::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject teaching::FlowViewImpl::staticMetaObject = {
    { &TaskExecutionView::staticMetaObject, qt_meta_stringdata_teaching__FlowViewImpl,
      qt_meta_data_teaching__FlowViewImpl, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &teaching::FlowViewImpl::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *teaching::FlowViewImpl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *teaching::FlowViewImpl::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_teaching__FlowViewImpl))
        return static_cast<void*>(const_cast< FlowViewImpl*>(this));
    return TaskExecutionView::qt_metacast(_clname);
}

int teaching::FlowViewImpl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = TaskExecutionView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
