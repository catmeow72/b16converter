#pragma once
#include <string>
class ConsoleOutput;
class ConsoleOutput {
	public:
	enum FormatType {
		Text,
		LogStyle,
		Json
	}
	enum Category {
		None,
		Debug,
		Info,
		Warning,
		Error,
	};
	static FormatType format_type;
	static std::string CategoryPrefix(Category category);
	static std::string InsertCategoryPrefix(Category category, std::string input);
	static void Write(Category category, const char *fmt, ...);
	static void WriteLn(Category category, const char *fmt, ...);
	static std::string Format(const char *fmt, ...);
	static std::string VFormat(const char *fmt, va_list args);
};
#define Debug(...) ConsoleOutput::WriteLn(ConsoleOutput::Debug, __VA_ARGS__)
#define Info(...) ConsoleOutput::WriteLn(ConsoleOutput::Info, __VA_ARGS__)
#define Warning(...) ConsoleOutput::WriteLn(ConsoleOutput::Warning, __VA_ARGS__)
#define Error(...) ConsoleOutput::WriteLn(ConsoleOutput::Error, __VA_ARGS__)
#define Print(...) ConsoleOutput::Write(ConsoleOutput::None, __VA_ARGS__)
#define PrintLn(...) ConsoleOutput::WriteLn(ConsoleOutput::None, __VA_ARGS__)