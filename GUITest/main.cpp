#include <filesystem>

#include "DXMain.h"
#include "DXContext.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <binding.h>
#include <Logging.h>
#include "AsyncCallbacks.h"
#include "DefaultModule.h"

#include <WrapperUtils.hpp>
#include "script_gui/ScriptWindow.h"

inline std::string WstrToUtf8Str(const std::wstring& wstr)
{
	std::string retStr;
	if (!wstr.empty())
	{
		int sizeRequired = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);

		if (sizeRequired > 0)
		{
			std::vector<char> utf8String(sizeRequired);
			int bytesConverted = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(),
				-1, &utf8String[0], utf8String.size(), NULL,
				NULL);
			if (bytesConverted != 0)
			{
				retStr = &utf8String[0];
			}
		}
	}
	return retStr;
}

inline std::string OpenFile(const wchar_t* filter_name, const wchar_t* filter_ext)
{
	wchar_t filter[1024];
	wsprintf(filter, L"%s", filter_name);

	int pos = lstrlenW(filter);
	wsprintf(filter + pos + 1, L"%s", filter_ext);

	wchar_t buffer[1024];
	OPENFILENAMEW ofn{ 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = buffer;
	ofn.lpstrFile[0] = 0;
	ofn.nMaxFile = 1024;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	BOOL res = ::GetOpenFileName(&ofn);
	if (res)
	{
		return WstrToUtf8Str(buffer);
	}
	else
	{
		return "";
	}
}

class Test : public DXMain
{
public:
	Test(const char* exec_path, const wchar_t* title, int width, int height)
		: DXMain(title, width, height)
		, m_v8vm(s_get_vm(exec_path))
	{		
		GetDefaultModule(m_world_definition.default_module);
		Logging::SetPrintCallbacks(this, print, print);

		this->SetFramerate(60.0f);
	}

	~Test()
	{
		UnloadScript();
	}

	void LoadScript(const char* dir, const char* filename)
	{
		UnloadScript();
		std::filesystem::current_path(dir);
		m_context = std::unique_ptr<GameContext>(new GameContext(&m_v8vm, m_world_definition, filename));

		v8::Isolate* isolate = m_v8vm.m_isolate;
		v8::HandleScope handle_scope(isolate);
		v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
		v8::Function* callback_init = m_context->GetCallback("init");
		if (callback_init != nullptr)
		{
			m_context->InvokeCallback(callback_init, {});
		}
	}

	void UnloadScript()
	{
		AsyncCallbacks::CheckPendings();
		if (m_context != nullptr)
		{
			v8::Isolate* isolate = m_v8vm.m_isolate;
			v8::HandleScope handle_scope(isolate);
			v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
			v8::Function* callback_dispose = m_context->GetCallback("dispose");
			if (callback_dispose != nullptr)
			{
				m_context->InvokeCallback(callback_dispose, {});
			}
			m_context = nullptr;
		}
	}


protected:
	void idle() override
	{
		AsyncCallbacks::CheckPendings();
		v8::platform::PumpMessageLoop(m_v8vm.m_platform.get(), m_v8vm.m_isolate);
	}

	void paint(int width, int height) override
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();		

		{
			ImGui::Begin("Launcher");

			if (ImGui::Button("Load Script"))
			{
				PostAction(s_load_script, this);
			}

			ImGui::SameLine();

			if (ImGui::Button("Clear Log"))
			{
				m_console_text = "";
			}

			ImGui::Text("Console log:");
			ImGui::BeginChild("ConsoleLog", { 0, 0 }, true, ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::Text(m_console_text.c_str());
			ImGui::EndChild();

			ImGui::End();
		}

		if (m_context!=nullptr)
		{
			v8::Isolate* isolate = m_v8vm.m_isolate;
			v8::HandleScope handle_scope(isolate);
			v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
			LocalContext lctx(isolate);

			v8::Local<v8::Value> value = lctx.get_global("scriptWindow");
			ScriptWindow* script_window = lctx.jobj_to_obj<ScriptWindow>(value);

			if (script_window->show)
			{
				script_window->Draw(this);
			}
		}

		ImGui::Render();

		DXContext* dxctx = get_context();
		const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
		dxctx->m_pContext->OMSetRenderTargets(1, &dxctx->m_pRenderTargetView, NULL);
		dxctx->m_pContext->ClearRenderTargetView(dxctx->m_pRenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	}

private:
	static V8VM& s_get_vm(const char* exec_path)
	{
		static thread_local V8VM vm(exec_path);
		return vm;
	}
	V8VM& m_v8vm;

	WorldDefinition m_world_definition;
	std::unique_ptr<GameContext> m_context;

	std::string m_console_text;

	static void print(void* ptr, const char* str)
	{
		Test* self = (Test*)ptr;
		self->m_console_text += str;
		self->m_console_text += "\n";
	}

	static void s_load_script(void* ptr)
	{
		Test* self = (Test*)ptr;
		std::string file_path = OpenFile(L"JavaScript", L"*.js");
		if (file_path != "")
		{
			std::filesystem::path path(file_path);
			std::string filename = path.filename().string();
			std::string directory = path.parent_path().string();
			self->LoadScript(directory.c_str(), filename.c_str());
		}
	}
};

int main(int argc, const char* argv[])
{	
	Test test(argv[0], L"GUITest", 1280, 720);
	test.MainLoop();

	return 0;
}