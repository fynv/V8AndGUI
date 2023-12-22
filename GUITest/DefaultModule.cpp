#include <binding.h>
#include <Logging.h>
#include "DefaultModule.h"

inline const char* ToCString(const v8::String::Utf8Value& value) {
	return *value ? *value : "<string conversion failed>";
}


static void Print(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string line = "";
	bool first = true;
	for (int i = 0; i < args.Length(); i++) {
		v8::HandleScope handle_scope(args.GetIsolate());
		if (first) {
			first = false;
		}
		else
		{
			line += " ";
		}
		v8::String::Utf8Value str(args.GetIsolate(), args[i]);
		const char* cstr = ToCString(str);
		line += cstr;
	}

	Logging::print_std(line.c_str());

}

#include "network/WrapperHttpClient.h"
#include "script_gui/WrapperScriptWindow.h"
#include "script_gui/WrapperElements.h"

void GetDefaultModule(ModuleDefinition& module)
{
	module.functions = {
		{ "print", Print},
		{ "setCallback", GameContext::SetCallback}
	};

	module.objects = {
		WrapperHttpClient::define,
		WrapperScriptWindow::define,
	};

	module.classes = {
		WrapperText::define,
		WrapperSameLine::define,
		WrapperInputText::define,
		WrapperButton::define
	};
}

