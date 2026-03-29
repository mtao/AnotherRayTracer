#if !defined(ART_EXPORT_HPP)
#define ART_EXPORT_HPP

#ifdef _WIN32
#ifdef ART_EXPORTS
#define ART_API __declspec(dllexport)
#else
#define ART_API __declspec(dllimport)
#endif
#else
#define ART_API __attribute__((visibility("default")))
#endif
#endif
