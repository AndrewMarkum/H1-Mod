#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "game/dvars.hpp"

#include "component/scheduler.hpp"
#include "gui.hpp"
#include "fog_editor.hpp"
#include "component/console.hpp"

#include <utils/string.hpp>
#include <utils/hook.hpp>
#include <utils/concurrency.hpp>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace gui
{
	std::unordered_map<std::string, bool> enabled_menus;
	std::vector<std::tuple<std::string, std::string, std::function<void()>>> registered_menus;

	namespace
	{
		struct frame_callback
		{
			std::function<void()> callback;
			bool always;
		};

		struct event
		{
			HWND hWnd;
			UINT msg;
			WPARAM wParam;
			LPARAM lParam;
		};

		utils::concurrency::container<std::vector<frame_callback>> on_frame_callbacks;
		utils::concurrency::container<std::deque<notification_t>> notifications;
		utils::concurrency::container<std::vector<event>> event_queue;

		ID3D11Device* device;
		ID3D11DeviceContext* device_context;
		bool initialized = false;
		bool toggled = false;

		void initialize_gui_context()
		{
			ImGui::CreateContext();
			ImGui::StyleColorsDark();

			ImGui_ImplWin32_Init(*game::hWnd);
			ImGui_ImplDX11_Init(device, device_context);

			initialized = true;
		}

		void new_gui_frame()
		{
			ImGui::GetIO().MouseDrawCursor = toggled;
			if (toggled)
			{
				*game::keyCatchers |= 0x10;
			}
			else
			{
				*game::keyCatchers &= ~0x10;
			}

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();

			ImGui::NewFrame();
		}

		void end_gui_frame()
		{
			ImGui::EndFrame();
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		void toggle_menu(const std::string& name)
		{
			enabled_menus[name] = !enabled_menus[name];
		}

		bool find_menu(const char* menu_name)
		{
			for (int i = 0; i < registered_menus.size(); ++i)
			{
				if (std::get<0>(registered_menus[i]) == menu_name)
					return true;
			}

			return false;
		}

		std::string truncate(const std::string& text, const size_t length, const std::string& end)
		{
			return text.size() <= length
				? text
				: text.substr(0, length - end.size()) + end;
		}

		void show_notifications()
		{
			static const auto window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
				ImGuiWindowFlags_NoMove;

			notifications.access([](std::deque<notification_t>& notifications_)
				{
					auto index = 0;
					for (auto i = notifications_.begin(); i != notifications_.end();)
					{
						const auto now = std::chrono::high_resolution_clock::now();
						if (now - i->creation_time >= i->duration)
						{
							i = notifications_.erase(i);
							continue;
						}

						const auto title = truncate(i->title, 34, "...");
						const auto text = truncate(i->text, 34, "...");

						ImGui::SetNextWindowSizeConstraints(ImVec2(250, 50), ImVec2(250, 50));
						ImGui::SetNextWindowBgAlpha(0.6f);
						ImGui::Begin(utils::string::va("Notification #%i", index), nullptr, window_flags);

						ImGui::SetWindowPos(ImVec2(10, 30.f + static_cast<float>(index) * 60.f));
						ImGui::SetWindowSize(ImVec2(250, 0));
						ImGui::Text(title.data());
						ImGui::Text(text.data());

						ImGui::End();

						++i;
						++index;
					}
				});
		}

		void menu_checkbox(const std::string& name, const std::string& menu)
		{
			ImGui::Checkbox(name.data(), &enabled_menus[menu]);
			for (auto _ : registered_menus)
			{
				if (std::get<0>(_) == menu)
				{
					if(enabled_menus[menu])
						std::get<2>(_)();
				}
			}
		}

		void run_frame_callbacks()
		{
			on_frame_callbacks.access([](std::vector<frame_callback>& callbacks)
				{
					for (const auto& callback : callbacks)
					{
						if (callback.always || toggled)
						{
							callback.callback();
						}
					}
				});
		}

		void gui_on_frame()
		{
			if (!game::Sys_IsDatabaseReady2())
			{
				return;
			}

			if (!initialized)
			{
				console::info("[ImGui] Initializing\n");
				initialize_gui_context();
			}
			else
			{
				new_gui_frame();
				run_frame_callbacks();
				end_gui_frame();
			}
		}

		HRESULT d3d11_create_device_stub(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software,
			UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
			ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
		{
			const auto result = D3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels,
				FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

			if (ppDevice != nullptr && ppImmediateContext != nullptr)
			{
				device = *ppDevice;
				device_context = *ppImmediateContext;
			}

			return result;
		}

		void dxgi_swap_chain_present_stub(utils::hook::assembler& a)
		{
			a.pushad64();
			a.call_aligned(gui_on_frame);
			a.popad64();

			a.mov(r8d, esi);
			a.mov(edx, r15d);
			a.mov(rcx, rdi);
			a.call(rbx);
			a.mov(ecx, eax);

			a.jmp(0x6CB185_b);
		}

		utils::hook::detour wnd_proc_hook;
		LRESULT wnd_proc_stub(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			if (wParam != VK_ESCAPE && toggled)
			{
				event_queue.access([hWnd, uMsg, wParam, lParam](std::vector<event>& queue)
					{
						queue.push_back({ hWnd, uMsg, wParam, lParam });
					});
			}

			if (uMsg == WM_KEYUP && wParam == VK_F2)
			{
				toggled = !toggled;

				return 0;
			}
			
			if (gui::toggled)
			{
				LRESULT result = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
				return TRUE;
			}

			return wnd_proc_hook.invoke<LRESULT>(hWnd, uMsg, wParam, lParam);
		}

		utils::hook::detour set_cursor_pos;
		bool set_cursor_pos_stub(int x, int y)
		{
			if (gui::toggled)
			{
				return false;
			}

			return set_cursor_pos.invoke<bool>(x, y);
		}
	}

	bool gui_mouse_event(const int local_client_num, int x, int y)
	{
		return !toggled;
	}

	void register_menu(const char* menu_name, const char* menu_title, std::function<void()> callback)
	{
			if(!find_menu(menu_name))
				registered_menus.emplace_back(menu_name, menu_title, callback);
	}

	void on_frame(const std::function<void()>& callback, bool always)
	{
		on_frame_callbacks.access([always, callback](std::vector<frame_callback>& callbacks)
			{
				callbacks.push_back({ callback, always });
			});
	}

	bool is_menu_open(const std::string& name)
	{
		return enabled_menus[name];
	}

	void notification(const std::string& title, const std::string& text, const std::chrono::milliseconds duration)
	{
		notification_t notification{};
		notification.title = title;
		notification.text = text;
		notification.duration = duration;
		notification.creation_time = std::chrono::high_resolution_clock::now();

		notifications.access([notification](std::deque<notification_t>& notifications_)
			{
				notifications_.push_front(notification);
			});
	}

	class component final : public component_interface
	{
	public:
		void* load_import(const std::string& library, const std::string& function) override
		{
			if (function == "D3D11CreateDevice")
			{
				return d3d11_create_device_stub;
			}

			return nullptr;
		}

		void post_unpack() override
		{
			utils::hook::nop(SELECT_VALUE(0x5B3570_b, 0x6CB16D_b), 9);
			utils::hook::call(SELECT_VALUE(0x5B3573_b, 0x6CB170_b), gui_on_frame);
			wnd_proc_hook.create(SELECT_VALUE(0x4631D0_b, 0x5BFF60_b), wnd_proc_stub);
			set_cursor_pos.create(0x5BA990_b, set_cursor_pos_stub);

			register_menu("fog_editor", "Fog Editor", gui::fog_editor::render_window);
			on_frame([]
				{
					show_notifications();

					ImGui::Begin("IW5r Utils", &toggled);
					for (auto menu : registered_menus)
					{
						menu_checkbox(std::get<1>(menu), std::get<0>(menu));
					}
					ImGui::End();
				});
		}

		void pre_destroy() override
		{
			if (initialized)
			{
				ImGui_ImplWin32_Shutdown();
				ImGui::DestroyContext();
			}
		}
	};
}

REGISTER_COMPONENT(gui::component)
