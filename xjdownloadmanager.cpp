#include <gtkmm.h>
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <thread>

class XJDownloadManager : public Gtk::Window {
public:
    XJDownloadManager() : button_download("Start Download") {
        set_title("XJ Download Manager");
        set_default_size(400, 200);

        vbox.set_margin(10);
        vbox.set_spacing(10);
        set_child(vbox);

        vbox.append(progress_bar);
        vbox.append(button_download);

        button_download.signal_clicked().connect(sigc::mem_fun(*this, &XJDownloadManager::start_download));

        progress_bar.set_show_text(true);
    }

private:
    Gtk::Box vbox{Gtk::Orientation::VERTICAL};
    Gtk::ProgressBar progress_bar;
    Gtk::Button button_download;

    static size_t write_data(void* ptr, size_t size, size_t nmemb, std::ofstream* stream) {
        stream->write(static_cast<char*>(ptr), size * nmemb);
        return size * nmemb;
    }

    static int progress_callback(void* ptr, curl_off_t total, curl_off_t now, curl_off_t, curl_off_t) {
        if (total > 0) {
            auto* manager = static_cast<XJDownloadManager*>(ptr);
            double progress = static_cast<double>(now) / static_cast<double>(total);
            Glib::signal_idle().connect_once([manager, progress]() {
                manager->progress_bar.set_fraction(progress);
            });
        }
        return 0;
    }

    void start_download() {
        std::thread([this]() {
            CURL* curl = curl_easy_init();
            if (curl) {
                const std::string url = "https://speed.hetzner.de/100MB.bin";
                const std::string filename = "downloaded_file.bin";

                std::ofstream file(filename, std::ios::binary);
                if (!file) {
                    std::cerr << "Failed to open file for writing.\n";
                    return;
                }

                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
                curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
                curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
                curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

                CURLcode res = curl_easy_perform(curl);
                curl_easy_cleanup(curl);

                file.close();

                Glib::signal_idle().connect_once([this, res]() {
                    if (res == CURLE_OK) {
                        progress_bar.set_fraction(1.0);
                        progress_bar.set_text("Download Complete");
                    } else {
                        progress_bar.set_text("Download Failed");
                    }
                });
            }
        }).detach();
    }
};

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create("com.example.xjdownloadmanager");
    return app->make_window_and_run<XJDownloadManager>(argc, argv);
}
