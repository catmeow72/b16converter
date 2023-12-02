#pragma once

#ifdef _WIN32
#include <stdlib.h>
#include <string.h>
inline void setenv(const char *variable, const char *value, char replace) {
	if (replace || getenv(variable) == NULL || !strcmp(getenv(variable), "")) {
		if (value == NULL) {
			_putenv_s(variable, "");
		} else {
			_putenv_s(variable, value);
		}
	}
}
#else
#include <unistd.h>
#endif