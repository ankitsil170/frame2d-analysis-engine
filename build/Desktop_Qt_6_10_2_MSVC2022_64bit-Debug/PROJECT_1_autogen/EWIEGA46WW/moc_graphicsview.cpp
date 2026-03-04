/****************************************************************************
** Meta object code from reading C++ file 'graphicsview.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../graphicsview.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'graphicsview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN12GraphicsViewE_t {};
} // unnamed namespace

template <> constexpr inline auto GraphicsView::qt_create_metaobjectdata<qt_meta_tag_ZN12GraphicsViewE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GraphicsView",
        "mouseClicked",
        "",
        "QPointF",
        "scenePos",
        "leftClicked",
        "rightClicked",
        "leftPressed",
        "mouseMoved",
        "mouseReleased"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'mouseClicked'
        QtMocHelpers::SignalData<void(const QPointF &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'leftClicked'
        QtMocHelpers::SignalData<void(const QPointF &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'rightClicked'
        QtMocHelpers::SignalData<void(const QPointF &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'leftPressed'
        QtMocHelpers::SignalData<void(const QPointF &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'mouseMoved'
        QtMocHelpers::SignalData<void(const QPointF &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'mouseReleased'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<GraphicsView, qt_meta_tag_ZN12GraphicsViewE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GraphicsView::staticMetaObject = { {
    QMetaObject::SuperData::link<QGraphicsView::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12GraphicsViewE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12GraphicsViewE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12GraphicsViewE_t>.metaTypes,
    nullptr
} };

void GraphicsView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<GraphicsView *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->mouseClicked((*reinterpret_cast<std::add_pointer_t<QPointF>>(_a[1]))); break;
        case 1: _t->leftClicked((*reinterpret_cast<std::add_pointer_t<QPointF>>(_a[1]))); break;
        case 2: _t->rightClicked((*reinterpret_cast<std::add_pointer_t<QPointF>>(_a[1]))); break;
        case 3: _t->leftPressed((*reinterpret_cast<std::add_pointer_t<QPointF>>(_a[1]))); break;
        case 4: _t->mouseMoved((*reinterpret_cast<std::add_pointer_t<QPointF>>(_a[1]))); break;
        case 5: _t->mouseReleased(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (GraphicsView::*)(const QPointF & )>(_a, &GraphicsView::mouseClicked, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (GraphicsView::*)(const QPointF & )>(_a, &GraphicsView::leftClicked, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (GraphicsView::*)(const QPointF & )>(_a, &GraphicsView::rightClicked, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (GraphicsView::*)(const QPointF & )>(_a, &GraphicsView::leftPressed, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (GraphicsView::*)(const QPointF & )>(_a, &GraphicsView::mouseMoved, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (GraphicsView::*)()>(_a, &GraphicsView::mouseReleased, 5))
            return;
    }
}

const QMetaObject *GraphicsView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GraphicsView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12GraphicsViewE_t>.strings))
        return static_cast<void*>(this);
    return QGraphicsView::qt_metacast(_clname);
}

int GraphicsView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void GraphicsView::mouseClicked(const QPointF & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void GraphicsView::leftClicked(const QPointF & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void GraphicsView::rightClicked(const QPointF & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void GraphicsView::leftPressed(const QPointF & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void GraphicsView::mouseMoved(const QPointF & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void GraphicsView::mouseReleased()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
