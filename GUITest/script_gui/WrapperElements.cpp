#include <WrapperUtils.hpp>
#include "Elements.h"
#include "WrapperElements.h"

///////////////////// Element ///////////////////

void WrapperElement::define(ClassDefinition& object)
{
	object.name = "Element";
	object.dtor = dtor;
	object.properties = {
		{"name", GetName, SetName},
	};
}

void WrapperElement::dtor(void* ptr, GameContext* ctx)
{
	delete (Element*)ptr;
}

void WrapperElement::GetName(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Element* self = lctx.self<Element>();
	info.GetReturnValue().Set(lctx.str_to_jstr(self->name.c_str()));
}

void WrapperElement::SetName(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Element* self = lctx.self<Element>();
	self->name = lctx.jstr_to_str(value);
}

///////////////////// Text ///////////////////

void WrapperText::define(ClassDefinition& cls)
{
	WrapperElement::define(cls);
	cls.name = "Text";
	cls.ctor = ctor;
}

void* WrapperText::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);

	std::string text;
	if (info.Length() > 0)
	{
		text = lctx.jstr_to_str(info[0]);
	}

	return new Text(text.c_str());
}

///////////////////// SameLine ///////////////////

void WrapperSameLine::define(ClassDefinition& cls)
{
	WrapperElement::define(cls);
	cls.name = "SameLine";
	cls.ctor = ctor;
}

void* WrapperSameLine::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{	
	return new SameLine;
}

///////////////////// InputText ///////////////////


void WrapperInputText::define(ClassDefinition& cls)
{
	WrapperElement::define(cls);
	cls.name = "InputText";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "text", GetText, SetText },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());
}

void* WrapperInputText::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);

	std::string name;
	if (info.Length() > 0)
	{
		name = lctx.jstr_to_str(info[0]);
	}

	int size = 256;
	std::string str;
	if (info.Length() > 1)
	{
		lctx.jnum_to_num(info[1], size);
		if (info.Length() > 2)
		{
			str = lctx.jstr_to_str(info[2]);
		}
	}
	return new InputText(name.c_str(), size, str.c_str());
}


void WrapperInputText::GetText(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	InputText* self = lctx.self<InputText>();
	info.GetReturnValue().Set(lctx.str_to_jstr(self->text.data()));
}

void WrapperInputText::SetText(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	InputText* self = lctx.self<InputText>();
	std::string str = lctx.jstr_to_str(value);
	int len = str.length();
	if (len > self->text.size() - 1)
	{
		len = self->text.size() - 1;
	}
	memcpy(self->text.data(), str.c_str(), len);
	self->text[len] = 0;
}

///////////////////// Button ///////////////////

void WrapperButton::define(ClassDefinition& cls)
{
	WrapperElement::define(cls);
	cls.name = "Button";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "onClick", GetOnClick, SetOnClick },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());
}


void* WrapperButton::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);

	std::string name;
	if (info.Length() > 0)
	{
		name = lctx.jstr_to_str(info[0]);
	}

	return new Button(name.c_str());
}


typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> CallbackT;

struct ButtonClickData
{
	GameContext* ctx;
	CallbackT callback;
};

static void ButtonClickCallback(void* ptr)
{
	ButtonClickData* data = (ButtonClickData*)ptr;
	GameContext* ctx = data->ctx;
	v8::Isolate* isolate = ctx->m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = ctx->m_context.Get(isolate);
	v8::Context::Scope context_scope(context);
	v8::Local<v8::Function> callback = data->callback.Get(isolate);
	std::vector<v8::Local<v8::Value>> args;
	ctx->InvokeCallback(*callback, args);
}


void WrapperButton::GetOnClick(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> onClick = lctx.get_property(lctx.holder, "_onClick");
	info.GetReturnValue().Set(onClick);
}

void WrapperButton::SetOnClick(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onClick", value);

	Button* self = lctx.self<Button>();
	self->click_callback = ButtonClickCallback;

	if (self->click_callback_data != nullptr)
	{
		ButtonClickData* data = (ButtonClickData*)self->click_callback_data;
		delete data;
	}

	ButtonClickData* data = new ButtonClickData;
	data->ctx = lctx.ctx();

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(lctx.isolate, callback);

	self->click_callback_data = data;
}

