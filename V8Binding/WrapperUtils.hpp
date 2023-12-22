#pragma once

#include <v8.h>
#include "binding.h"

class LocalContext
{
public:
	v8::Isolate* isolate;
	v8::HandleScope handle_scope;
	v8::Local<v8::Context> context;
	v8::Local<v8::Object> holder;

	template<class InfoType>
	LocalContext(const InfoType& info)
		: isolate(info.GetIsolate())
		, handle_scope(isolate)
		, context(isolate->GetCurrentContext())
		, holder(info.Holder())
	{

	}

	LocalContext(v8::Isolate* isolate)
		: isolate(isolate)
		, handle_scope(isolate)
		, context(isolate->GetCurrentContext())
	{

	}

	inline v8::Local<v8::Value> get_global(const char* path)
	{
		std::string s_path = path;
		v8::Local<v8::Value> value = context->Global();

		size_t start = 0;
		size_t end = s_path.find('.', start);
		if (end == std::string::npos)
		{
			end = s_path.length();
		}
		while (end > start)
		{
			std::string key = s_path.substr(start, end - start);
			value = get_property(value.As<v8::Object>(), key.c_str());
			if (end < s_path.length())
			{
				start = end + 1;
				end = s_path.find('.', start);
				if (end == std::string::npos)
				{
					end = s_path.length();
				}
			}
			else
			{
				start = end;
			}
		}
		return value;
	}

	inline GameContext* ctx()
	{
		v8::Local<v8::Object> global = context->Global();
		GameContext* ctx = (GameContext*)global->GetAlignedPointerFromInternalField(0);
		return ctx;
	}

	inline void* get_self()
	{
		return holder->GetAlignedPointerFromInternalField(0);
	}

	template<class T>
	inline T* self()
	{
		return (T*)get_self();
	}

	template<class T>
	inline T* jobj_to_obj(v8::Local<v8::Value> obj)
	{
		return (T*)obj.As<v8::Object>()->GetAlignedPointerFromInternalField(0);
	}

	v8::Local<v8::Object> instantiate(const char* cls_name)
	{
		v8::Local<v8::Value> value = get_global(cls_name);
		v8::Local<v8::Function> ctor = value.As<v8::Function>();
		return ctor->CallAsConstructor(context, 0, nullptr).ToLocalChecked().As<v8::Object>();
	}

	inline bool has_property(v8::Local<v8::Object> obj, const char* name)
	{
		return obj->Has(context, v8::String::NewFromUtf8(isolate, name).ToLocalChecked()).ToChecked();
	}

	inline v8::Local<v8::Value> get_property(v8::Local<v8::Object> obj, const char* name)
	{
		v8::Local<v8::Value> prop = v8::Null(isolate);
		if (has_property(obj, name))
		{
			prop = obj->Get(context, v8::String::NewFromUtf8(isolate, name).ToLocalChecked()).ToLocalChecked();
		}
		return prop;
	}

	inline void set_property(v8::Local<v8::Object> obj, const char* name, v8::Local<v8::Value> value)
	{
		obj->Set(context, v8::String::NewFromUtf8(isolate, name).ToLocalChecked(), value);
	}

	inline void del_property(v8::Local<v8::Object> obj, const char* name)
	{
		obj->Delete(context, v8::String::NewFromUtf8(isolate, name).ToLocalChecked());
	}

	template<typename T>
	inline v8::Local<v8::Number> num_to_jnum(T value)
	{
		return v8::Number::New(isolate, (double)value);
	}

	template<typename T>
	inline void jnum_to_num(v8::Local<v8::Value> jnum, T& value)
	{
		value = (T)jnum.As<v8::Number>()->Value();
	}

	inline v8::Local<v8::String> str_to_jstr(const char* str)
	{
		return v8::String::NewFromUtf8(isolate, str).ToLocalChecked();
	}

	inline std::string jstr_to_str(v8::Local<v8::Value> value)
	{
		v8::String::Utf8Value str(isolate, value);
		return *str;
	}

};

