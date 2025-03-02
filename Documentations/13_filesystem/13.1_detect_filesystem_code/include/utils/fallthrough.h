#ifndef _FALLT_H
#define _FALLT_H
#if __has_attribute(fallthrough)
#define FALLTHROUGH __attribute__((fallthrough))
// Note, there could be more branches here, like using `[[gsl::suppress]]` for MSVC
#else
#define FALLTHROUGH
#endif

#endif