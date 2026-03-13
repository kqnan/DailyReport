/****************************************************************************
** Meta object code from reading C++ file 'apimanager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/apimanager.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'apimanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.6.2. It"
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

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSApiManagerENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSApiManagerENDCLASS = QtMocHelpers::stringData(
    "ApiManager",
    "verificationCodeReceived",
    "",
    "code",
    "verificationCodeFailed",
    "error",
    "loginSuccess",
    "token",
    "loginFailed",
    "message",
    "dailyReportListReceived",
    "reports",
    "dailyReportListFailed",
    "dailyReportDetailsReceived",
    "tasks",
    "date",
    "dailyReportDetailsFailed",
    "dailyReportCreated",
    "uuid",
    "dailyReportCreateFailed",
    "syncSuccess",
    "syncFailed"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSApiManagerENDCLASS_t {
    uint offsetsAndSizes[44];
    char stringdata0[11];
    char stringdata1[25];
    char stringdata2[1];
    char stringdata3[5];
    char stringdata4[23];
    char stringdata5[6];
    char stringdata6[13];
    char stringdata7[6];
    char stringdata8[12];
    char stringdata9[8];
    char stringdata10[24];
    char stringdata11[8];
    char stringdata12[22];
    char stringdata13[27];
    char stringdata14[6];
    char stringdata15[5];
    char stringdata16[25];
    char stringdata17[19];
    char stringdata18[5];
    char stringdata19[24];
    char stringdata20[12];
    char stringdata21[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSApiManagerENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSApiManagerENDCLASS_t qt_meta_stringdata_CLASSApiManagerENDCLASS = {
    {
        QT_MOC_LITERAL(0, 10),  // "ApiManager"
        QT_MOC_LITERAL(11, 24),  // "verificationCodeReceived"
        QT_MOC_LITERAL(36, 0),  // ""
        QT_MOC_LITERAL(37, 4),  // "code"
        QT_MOC_LITERAL(42, 22),  // "verificationCodeFailed"
        QT_MOC_LITERAL(65, 5),  // "error"
        QT_MOC_LITERAL(71, 12),  // "loginSuccess"
        QT_MOC_LITERAL(84, 5),  // "token"
        QT_MOC_LITERAL(90, 11),  // "loginFailed"
        QT_MOC_LITERAL(102, 7),  // "message"
        QT_MOC_LITERAL(110, 23),  // "dailyReportListReceived"
        QT_MOC_LITERAL(134, 7),  // "reports"
        QT_MOC_LITERAL(142, 21),  // "dailyReportListFailed"
        QT_MOC_LITERAL(164, 26),  // "dailyReportDetailsReceived"
        QT_MOC_LITERAL(191, 5),  // "tasks"
        QT_MOC_LITERAL(197, 4),  // "date"
        QT_MOC_LITERAL(202, 24),  // "dailyReportDetailsFailed"
        QT_MOC_LITERAL(227, 18),  // "dailyReportCreated"
        QT_MOC_LITERAL(246, 4),  // "uuid"
        QT_MOC_LITERAL(251, 23),  // "dailyReportCreateFailed"
        QT_MOC_LITERAL(275, 11),  // "syncSuccess"
        QT_MOC_LITERAL(287, 10)   // "syncFailed"
    },
    "ApiManager",
    "verificationCodeReceived",
    "",
    "code",
    "verificationCodeFailed",
    "error",
    "loginSuccess",
    "token",
    "loginFailed",
    "message",
    "dailyReportListReceived",
    "reports",
    "dailyReportListFailed",
    "dailyReportDetailsReceived",
    "tasks",
    "date",
    "dailyReportDetailsFailed",
    "dailyReportCreated",
    "uuid",
    "dailyReportCreateFailed",
    "syncSuccess",
    "syncFailed"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSApiManagerENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      12,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   86,    2, 0x06,    1 /* Public */,
       4,    1,   89,    2, 0x06,    3 /* Public */,
       6,    1,   92,    2, 0x06,    5 /* Public */,
       8,    1,   95,    2, 0x06,    7 /* Public */,
      10,    1,   98,    2, 0x06,    9 /* Public */,
      12,    1,  101,    2, 0x06,   11 /* Public */,
      13,    2,  104,    2, 0x06,   13 /* Public */,
      16,    1,  109,    2, 0x06,   16 /* Public */,
      17,    2,  112,    2, 0x06,   18 /* Public */,
      19,    1,  117,    2, 0x06,   21 /* Public */,
      20,    1,  120,    2, 0x06,   23 /* Public */,
      21,    1,  123,    2, 0x06,   25 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void, QMetaType::QJsonArray,   11,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::QJsonArray, QMetaType::QString,   14,   15,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   18,    9,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void, QMetaType::QString,    5,

       0        // eod
};

Q_CONSTINIT const QMetaObject ApiManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSApiManagerENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSApiManagerENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSApiManagerENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ApiManager, std::true_type>,
        // method 'verificationCodeReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'verificationCodeFailed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'loginSuccess'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'loginFailed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'dailyReportListReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QJsonArray &, std::false_type>,
        // method 'dailyReportListFailed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'dailyReportDetailsReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QJsonArray &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'dailyReportDetailsFailed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'dailyReportCreated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'dailyReportCreateFailed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'syncSuccess'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'syncFailed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void ApiManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ApiManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->verificationCodeReceived((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->verificationCodeFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->loginSuccess((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->loginFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->dailyReportListReceived((*reinterpret_cast< std::add_pointer_t<QJsonArray>>(_a[1]))); break;
        case 5: _t->dailyReportListFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->dailyReportDetailsReceived((*reinterpret_cast< std::add_pointer_t<QJsonArray>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 7: _t->dailyReportDetailsFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->dailyReportCreated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 9: _t->dailyReportCreateFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->syncSuccess((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->syncFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ApiManager::*)(int );
            if (_t _q_method = &ApiManager::verificationCodeReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ApiManager::*)(const QString & );
            if (_t _q_method = &ApiManager::verificationCodeFailed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ApiManager::*)(const QString & );
            if (_t _q_method = &ApiManager::loginSuccess; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ApiManager::*)(const QString & );
            if (_t _q_method = &ApiManager::loginFailed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (ApiManager::*)(const QJsonArray & );
            if (_t _q_method = &ApiManager::dailyReportListReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (ApiManager::*)(const QString & );
            if (_t _q_method = &ApiManager::dailyReportListFailed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (ApiManager::*)(const QJsonArray & , const QString & );
            if (_t _q_method = &ApiManager::dailyReportDetailsReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (ApiManager::*)(const QString & );
            if (_t _q_method = &ApiManager::dailyReportDetailsFailed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (ApiManager::*)(const QString & , const QString & );
            if (_t _q_method = &ApiManager::dailyReportCreated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (ApiManager::*)(const QString & );
            if (_t _q_method = &ApiManager::dailyReportCreateFailed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (ApiManager::*)(const QString & );
            if (_t _q_method = &ApiManager::syncSuccess; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (ApiManager::*)(const QString & );
            if (_t _q_method = &ApiManager::syncFailed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
    }
}

const QMetaObject *ApiManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ApiManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSApiManagerENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ApiManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void ApiManager::verificationCodeReceived(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ApiManager::verificationCodeFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ApiManager::loginSuccess(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void ApiManager::loginFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void ApiManager::dailyReportListReceived(const QJsonArray & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void ApiManager::dailyReportListFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void ApiManager::dailyReportDetailsReceived(const QJsonArray & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void ApiManager::dailyReportDetailsFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void ApiManager::dailyReportCreated(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void ApiManager::dailyReportCreateFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void ApiManager::syncSuccess(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void ApiManager::syncFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}
QT_WARNING_POP
