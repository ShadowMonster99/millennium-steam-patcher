#include <window/api/installer.hpp>
#include <stdafx.h>

#include <regex>
#include <utils/clr/platform.hpp>
#include <utils/io/input-output.hpp>

extern bool is_installing = false;

namespace Community
{
	void installer::installUpdate(const nlohmann::json& skinData)
	{
#ifdef _WIN32
		bool success = clr_interop::clr_base::instance().start_update(nlohmann::json({
			{"owner", skinData["github"]["owner"]},
			{"repo", skinData["github"]["repo_name"]}
		}).dump(4));

		if (success) {
			//g_fileDropStatus = "Updated Successfully!";
		}
		else {
			//g_fileDropStatus = "Failed to update theme...";
		}
#elif __linux__
        console.err("installer::installUpdate HAS NO IMPLEMENTATION");
#endif
        std::this_thread::sleep_for(std::chrono::seconds(2));
	}

    const void installer::handleFileDrop(const char* _filePath)
    {
        std::cout << "Dropped file: " << _filePath << std::endl;
        try {
            std::filesystem::path filePath(_filePath);

            if (!std::filesystem::exists(filePath) || !std::filesystem::exists(filePath / "skin.json") || !std::filesystem::is_directory(filePath))
            {
                //MsgBox("The dropped skin either doesn't exist, isn't a folder, or doesn't have a skin.json inside. "
                //    "Make sure the skin isn't archived, and it exists on your disk", "Can't Add Skin", MB_ICONERROR);
                //
                //MsgBox("Can't Add Skin", [&](auto open) {

                //    ImGui::TextWrapped("The dropped skin either doesn't exist, isn't a folder, or doesn't have a skin.json inside. "
                //        "Make sure the skin isn't archived, and it exists on your disk");

                //    if (ImGui::Button("Close")) {

                //    }
                //});

                auto selection = msg::show("The dropped skin either doesn't exist, isn't a folder, or doesn't have a skin.json inside. "
                    "Make sure the skin isn't archived, and it exists on your disk", "Bootstrap Error", Buttons::OK);

                return;
            }

            std::filesystem::rename(filePath, std::filesystem::path(config.getSkinDir()) / filePath.filename().string());
        }
        catch (const std::filesystem::filesystem_error& error) {
            //MsgBox(fmt::format("An error occured while adding the dropped skin to your library.\nError:\n{}", error.what()).c_str(), "Fatal Error", MB_ICONERROR);

            //MsgBox("Can't Add Skin", [&](auto open) {

            //    ImGui::TextWrapped(fmt::format("An error occured while adding the dropped skin to your library.\nError:\n{}", error.what()).c_str());
            //});

            auto selection = msg::show(fmt::format("An error occured while adding the dropped skin to your library.\nError:\n{}", error.what()).c_str(), "Can't Add Skin", Buttons::OK);
        }
    }

#ifdef _WIN32
    bool unzip(std::string zipFileName, std::string targetDirectory) {

        std::string powershellCommand = fmt::format("powershell.exe -Command \"Expand-Archive '{}' -DestinationPath '{}' -Force\"", zipFileName, targetDirectory);

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        si.dwFlags |= STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        if (CreateProcess(NULL, const_cast<char*>(powershellCommand.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            return true;
        }
        return false;
    }
#elif __linux__
    bool unzip(std::string zipFileName, std::string targetDirectory) {
        console.err("__linux__ unzip DOES NOT HAVE AN IMPLEMENTATION");
        return false;
    }
#endif

    const void installer::handleThemeInstall(std::string fileName, std::string downloadPath, std::function<void(std::string)> cb)
    {
        auto filePath = std::filesystem::path(config.getSkinDir()) / fileName;

        try 
        {
            http::to_disk(downloadPath.c_str(), filePath.string().c_str());
            if (unzip(filePath.string(), config.getSkinDir() + "/"))
            {
                cb("success");
                std::this_thread::sleep_for(std::chrono::seconds(2));

                m_Client.parseSkinData(false);
            }
            else {
                cb("Couldn't extract theme...");
            }
        }
        catch (const http_error&) {
            cb("Couldn't download bytes from the file");
            console.err("Couldn't download bytes from the file");
        }
        catch (const std::exception& err) {
            console.err(fmt::format("Exception form {}: {}", __func__, err.what()));
            std::string error = fmt::format("Exception form {}: {}", __func__, err.what());

            auto selection = msg::show(error.c_str(), "Can't Add Skin", Buttons::OK);
        }
    }
}