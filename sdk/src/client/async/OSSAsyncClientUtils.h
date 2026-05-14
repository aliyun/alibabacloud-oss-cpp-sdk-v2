#pragma once

#include "src/client/sync/OSSClientFieldCheck.h"

#define requiredFieldAsync(field)                                                                  \
    do {                                                                                           \
        if (isFieldMissing(request.get##field())) {                                                \
            callback(makeUnexpected(OperationError(SdkErrorCode::ARGUMENT_REQUIRED,                               \
                                    {{"Code", "ArgumentRequired"}, {"Message", "Missing field " #field ""}}))); \
            return;                                                                                \
        }                                                                                          \
    } while (false)

#define requiredFieldsOrAsync2_(f1, f2)                                                                \
    do {                                                                                               \
        if (isFieldMissing(request.get##f1()) && isFieldMissing(request.get##f2())) {                  \
            callback(makeUnexpected(OperationError(                                                                   \
                    SdkErrorCode::ARGUMENT_REQUIRED,                                                   \
                    {{"Code", "ArgumentRequired"}, {"Message", "Missing field " #f1 " or " #f2 ""}}))); \
            return;                                                                                    \
        }                                                                                              \
    } while (false)

#define requiredFieldsOrAsync3_(f1, f2, f3)                                                              \
    do {                                                                                                 \
        if (isFieldMissing(request.get##f1()) && isFieldMissing(request.get##f2())                       \
            && isFieldMissing(request.get##f3())) {                                                      \
            callback(makeUnexpected(OperationError(                                                                     \
                    SdkErrorCode::ARGUMENT_REQUIRED,                                                     \
                    {{"Code", "ArgumentRequired"},                                                       \
                     {"Message", "Missing field " #f1 " or " #f2 " or " #f3 ""}})));                     \
            return;                                                                                      \
        }                                                                                                \
    } while (false)

#define requiredFieldsOrAsync4_(f1, f2, f3, f4)                                                          \
    do {                                                                                                 \
        if (isFieldMissing(request.get##f1()) && isFieldMissing(request.get##f2())                       \
            && isFieldMissing(request.get##f3()) && isFieldMissing(request.get##f4())) {                 \
            callback(makeUnexpected(OperationError(                                                                     \
                    SdkErrorCode::ARGUMENT_REQUIRED,                                                     \
                    {{"Code", "ArgumentRequired"},                                                       \
                     {"Message", "Missing field " #f1 " or " #f2 " or " #f3 " or " #f4 ""}})));         \
            return;                                                                                      \
        }                                                                                                \
    } while (false)

#define requiredFieldsOrAsyncN_(_1, _2, _3, _4, NAME, ...) NAME
#define requiredFieldsOrAsync(...) \
    EXPAND_ASYNC_(requiredFieldsOrAsyncN_(__VA_ARGS__, requiredFieldsOrAsync4_, requiredFieldsOrAsync3_, requiredFieldsOrAsync2_, _)(__VA_ARGS__))
#define EXPAND_ASYNC_(x) x

#define requiredHasFieldAsync(field)                                                                  \
    do {                                                                                              \
        if (!request.has##field()) {                                                                  \
            callback(makeUnexpected(OperationError(SdkErrorCode::ARGUMENT_REQUIRED,                                  \
                                    {{"Code", "ArgumentRequired"}, {"Message", "Missing field " #field ""}}))); \
            return;                                                                                   \
        }                                                                                             \
    } while (false)
