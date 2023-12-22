#pragma once

#include <definitions.hpp>

class GameContext;
class WrapperScriptWindow
{
public:
	static void define(ObjectDefinition& object);

private:
	static void* ctor();
	static void dtor(void* ptr, GameContext* ctx);

	static void GetShow(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetShow(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetTitle(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetTitle(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetElements(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void Add(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Remove(const v8::FunctionCallbackInfo<v8::Value>& info);	
	static void Clear(const v8::FunctionCallbackInfo<v8::Value>& info);

	
};