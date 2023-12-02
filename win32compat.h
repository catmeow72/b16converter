#pragma once

#ifdef _WIN32
#include <stdlib.h>
inline void setenv(char const *variable, char const *value, char replace) {
	if (replace || getenv(variable) == NULL || getenv(variable) == "") {
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