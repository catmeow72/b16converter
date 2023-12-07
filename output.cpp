#include "output.h"
#include <stdarg.h>
#include <stdio.h>
ConsoleOutput::FormatType ConsoleOutput::format_type = ConsoleOutput::Text;
std::string ConsoleOutput::CategoryPrefix(Category category) {
	if (format_type == Text) {
	switch (category) {
		case Debug:
			return "Debug: ";
		case Warning:
			return "WARNING: ";
		case Error:
			return "ERROR: ";
		case Info:
		case None:
		default:
			return "";
	}
	} else {
		std::string output;
		switch (category) {
			case Debug:
				output = "DEBUG";
				break;
			case Info:
				output = "INFO";
				break;
			case Warning:
				output = "WARNING";
				break;
			case Error:
				output = "ERROR";
				break;
			default:
				if (format_type == Json) {
					output = "NONE";
				} else {
					output = "";
				}
		}
		if (format_type == LogStyle) {
			output = "[" + output + "] ";
		}
		return output;
	}
}
std::string ConsoleOutput::InsertCategoryPrefix(Category category, std::string input) {
	std::string output = "";
	std::string categoryPrefix = CategoryPrefix(category);
	if (format_type == Json) {
		output = "{\"category\": \"";
		output += categoryPrefix;
		output += "\", \"text\": \"";
		for (size_t i = 0; i < input.length(); i++) {
			char inChar = input[i];
			switch (inChar) {
				case '\"':
					output += "\\\"";
					break;
				case '\n':
					output += "\\n";
					break;
				case '\r':
					output += "\\r";
					break;
				case '\\':
					output += '\\\\';
					break;
				case '/':
					output += "\\/";
					break;
				case '\b':
					output += "\\b";
					break;
				case '\t':
					output += '\\t';
					break;
				default:
					output += inChar;
			}
		}
		output += "\"}";
	} else {
		for (size_t i = 0; i < input.length(); i++) {
			char inChar = input[i];
			if (inChar == '\n' || (inChar == '\r' && (i + 1 < input.length() && input[i] != '\n'))) {
				output += "\n";
				output += categoryPrefix;
			} else {
				output += inChar;
			}
		}
	}
	return output;
}
void ConsoleOutput::Write(Category category, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	std::string output = VFormat(fmt, args);
	va_end(args);
	output = InsertCategoryPrefix(category, output);
	printf("%s", output.c_str());
}
void ConsoleOutput::WriteLn(Category category, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	std::string output = VFormat(fmt, args);
	va_end(args);
	output = InsertCategoryPrefix(category, output);
	printf("%s", output.c_str());
}
std::string ConsoleOutput::Format(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	std::string output = VFormat(fmt, args);
	va_end(args);
	return output;
}
std::string ConsoleOutput::VFormat(const char *fmt, va_list args) {
	size_t len = 0;
	char *buf = NULL;
	va_list args_copy;
	va_copy(args_copy, args);
	len = vsnprintf(NULL, 0, fmt, args_copy);
	va_end(args_copy);
	buf = (char*)malloc(len);
	va_copy(args_copy, args);
	len = vsnprintf(buf, len, fmt, args_copy);
	va_end(args_copy);
	std::string output(buf);
	free(buf);
	return output;
}