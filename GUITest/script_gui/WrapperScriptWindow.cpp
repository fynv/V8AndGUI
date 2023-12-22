#include <WrapperUtils.hpp>
#include "ScriptWindow.h"
#include "Elements.h"
#include "WrapperScriptWindow.h"

void WrapperScriptWindow::define(ObjectDefinition& object)
{
	object.name = "scriptWindow";
	object.ctor = ctor;
	object.dtor = dtor;
	object.properties = {
		{"show", GetShow, SetShow},
		{"title", GetTitle, SetTitle},
		{"elements", GetElements}
	};
	object.methods = {
		{ "add", Add },
		{ "remove", Remove },
		{ "clear", Clear },
	};
}

void* WrapperScriptWindow::ctor()
{
	return new ScriptWindow;
}

void WrapperScriptWindow::dtor(void* ptr, GameContext* ctx)
{
	delete (ScriptWindow*)ptr;
}

void WrapperScriptWindow::GetShow(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ScriptWindow* self = lctx.self<ScriptWindow>();
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, self->show));
}

void WrapperScriptWindow::SetShow(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	ScriptWindow* self = lctx.self<ScriptWindow>();
	self->show = value.As<v8::Boolean>()->Value();
}

void WrapperScriptWindow::GetTitle(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ScriptWindow* self = lctx.self<ScriptWindow>();
	info.GetReturnValue().Set(lctx.str_to_jstr(self->title.c_str()));
}

void WrapperScriptWindow::SetTitle(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	ScriptWindow* self = lctx.self<ScriptWindow>();
	self->title = lctx.jstr_to_str(value);
}

void WrapperScriptWindow::GetElements(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> elements = lctx.get_property(info.Holder(), "_elements");
	if (elements->IsNull())
	{
		elements = v8::Array::New(lctx.isolate);
		lctx.set_property(info.Holder(), "_elements", elements);
	}
	info.GetReturnValue().Set(elements);
}

void WrapperScriptWindow::Add(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ScriptWindow* self = lctx.self<ScriptWindow>();
	Element* element = lctx.jobj_to_obj<Element>(info[0]);
	self->add(element);

	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Object> holder_object = info[0].As<v8::Object>();

	v8::Local<v8::Array> elements = lctx.get_property(holder, "elements").As<v8::Array>();
	elements->Set(lctx.context, elements->Length(), holder_object);
}

void WrapperScriptWindow::Remove(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ScriptWindow* self = lctx.self<ScriptWindow>();
	Element* element = lctx.jobj_to_obj<Element>(info[0]);
	self->remove(element);

	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Object> holder_object = info[0].As<v8::Object>();

	v8::Local<v8::Array> elements = lctx.get_property(holder, "elements").As<v8::Array>();

	for (unsigned i = 0; i < elements->Length(); i++)
	{
		v8::Local<v8::Object> elem_i = elements->Get(lctx.context, i).ToLocalChecked().As<v8::Object>();
		if (elem_i == holder_object)
		{			
			for (unsigned j = i; j < elements->Length() - 1; j++)
			{
				elements->Set(lctx.context, j, elements->Get(lctx.context, j + 1).ToLocalChecked());
			}
			elements->Delete(lctx.context, elements->Length() - 1);
			lctx.set_property(elements, "length", lctx.num_to_jnum(elements->Length() - 1));
			break;
		}
	}
}

void WrapperScriptWindow::Clear(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ScriptWindow* self = lctx.self<ScriptWindow>();
	self->clear();

	v8::Local<v8::Value> elements = v8::Array::New(lctx.isolate);
	lctx.set_property(info.Holder(), "_elements", elements);
}

