#pragma once

#include "OSSClientFieldCheck.h"
#include "alibabacloud/oss2/Error.h"

#define requiredField(field)                                                                              \
    do {                                                                                                  \
        if (isFieldMissing(request.get##field())) {                                                       \
            return makeUnexpected(OperationError(ClientErrorCode::ArgumentRequired,                                        \
                                  {{"Code", "ArgumentRequired"}, {"Message", "Missing field " #field ""}})); \
        }                                                                                                 \
    } while (false)

#define requiredFieldsOr2_(f1, f2)                                                                             \
    do {                                                                                                     \
        if (isFieldMissing(request.get##f1()) && isFieldMissing(request.get##f2())) {                        \
            return makeUnexpected(OperationError(                                                                           \
                    ClientErrorCode::ArgumentRequired,                                                         \
                    {{"Code", "ArgumentRequired"}, {"Message", "Missing field " #f1 " or " #f2 ""}}));       \
        }                                                                                                    \
    } while (false)

#define requiredFieldsOr3_(f1, f2, f3)                                                                         \
    do {                                                                                                     \
        if (isFieldMissing(request.get##f1()) && isFieldMissing(request.get##f2())                           \
            && isFieldMissing(request.get##f3())) {                                                          \
            return makeUnexpected(OperationError(                                                                           \
                    ClientErrorCode::ArgumentRequired,                                                         \
                    {{"Code", "ArgumentRequired"},                                                           \
                     {"Message", "Missing field " #f1 " or " #f2 " or " #f3 ""}}));                          \
        }                                                                                                    \
    } while (false)

#define requiredFieldsOr4_(f1, f2, f3, f4)                                                                     \
    do {                                                                                                     \
        if (isFieldMissing(request.get##f1()) && isFieldMissing(request.get##f2())                           \
            && isFieldMissing(request.get##f3()) && isFieldMissing(request.get##f4())) {                     \
            return makeUnexpected(OperationError(                                                                           \
                    ClientErrorCode::ArgumentRequired,                                                         \
                    {{"Code", "ArgumentRequired"},                                                           \
                     {"Message", "Missing field " #f1 " or " #f2 " or " #f3 " or " #f4 ""}}));              \
        }                                                                                                    \
    } while (false)

#define requiredFieldsOrN_(_1, _2, _3, _4, NAME, ...) NAME
#define requiredFieldsOr(...) \
    EXPAND_(requiredFieldsOrN_(__VA_ARGS__, requiredFieldsOr4_, requiredFieldsOr3_, requiredFieldsOr2_, _)(__VA_ARGS__))
#define EXPAND_(x) x

#define requiredHasField(field)                                                                           \
    do {                                                                                                  \
        if (!request.has##field()) {                                                                      \
            return makeUnexpected(OperationError(ClientErrorCode::ArgumentRequired,                                        \
                                  {{"Code", "ArgumentRequired"}, {"Message", "Missing field " #field ""}})); \
        }                                                                                                 \
    } while (false)
