
#pragma once


#if defined(_WIN32)
#ifdef _MSC_VER
#pragma warning(disable : 4251)
#endif // _MSC_VER
#define ALIBABACLOUD_OSS_DECL_EXPORT __declspec(dllexport)
#define ALIBABACLOUD_OSS_DECL_IMPORT __declspec(dllimport)
#else
#define ALIBABACLOUD_OSS_DECL_EXPORT
#define ALIBABACLOUD_OSS_DECL_IMPORT
#endif


#if defined(ALIBABACLOUD_OSS_SHARED)
#if defined(ALIBABACLOUD_OSS_EXPORTS)
#define ALIBABACLOUD_OSS_API ALIBABACLOUD_OSS_DECL_EXPORT
#else
#define ALIBABACLOUD_OSS_API ALIBABACLOUD_OSS_DECL_IMPORT
#endif
#else
#define ALIBABACLOUD_OSS_API
#endif
