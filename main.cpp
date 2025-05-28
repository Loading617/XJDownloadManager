#include <CtrlLib/CtrlLib.h>
#include <Core/Core.h>
#include <plugin/ftp/ftp.h>

using namespace Upp;

class XJDownloadManager : public TopWindow {
    EditString urlEdit;
    ProgressIndicator progress;
    Button startBtn;
    Button cancelBtn;
    Label status;
    Atomic progressValue;
    Thread downloader;
    bool downloading = false;
    bool cancel = false;

    void StartDownload() {
        if (downloading) return;

        cancel = false;
        downloading = true;
        progressValue = 0;
        status.SetLabel("Starting download...");
        progress.Set(0);

        downloader = Thread([=] {
            String url = urlEdit.GetData();
            String fileName = GetFileName(url);
            String tempFile = AppendFileName(GetTempDirectory(), fileName);

            HttpRequest req(url);
            req.Timeout(30000);
            req.WhenContent = [&](void *ptr, int size) -> bool {
                FileAppend out(tempFile, File::SHARE_READ | File::SHARE_WRITE);
                out.Put(ptr, size);

                progressValue += size;
                return !cancel;
            };

            if (!req.Execute()) {
                PostCallback([=] {
                    status.SetLabel("Download failed.");
                    downloading = false;
                });
                return;
            }

            PostCallback([=] {
                progress.Set(100);
                status.SetLabel("Download complete: " + fileName);
                downloading = false;
            });
        });

        SetTimeCallback(100, THISBACK(UpdateProgress));
    }

    void UpdateProgress() {
        if (!downloading) return;
        int simulated = min(100, int(progressValue / 1000)); // Simulate %
        progress.Set(simulated);
        if (simulated < 100)
            SetTimeCallback(100, THISBACK(UpdateProgress));
    }

    void CancelDownload() {
        if (!downloading) return;
        cancel = true;
        status.SetLabel("Cancelling...");
        downloading = false;
    }

public:
    XJDownloadManager() {
        Title("XJDownloadManager").Sizeable().Zoomable();
        Add(urlEdit.TopPos(10, 24).HSizePos(10, 10));
        Add(startBtn.SetLabel("Start").TopPos(44, 24).LeftPos(10, 100));
        Add(cancelBtn.SetLabel("Cancel").TopPos(44, 24).LeftPos(120, 100));
        Add(progress.TopPos(80, 24).HSizePos(10, 10));
        Add(status.TopPos(110, 24).HSizePos(10, 10));

        startBtn << [=] { StartDownload(); };
        cancelBtn << [=] { CancelDownload(); };

        urlEdit.SetText("https://example.com/file.zip"); 
    }

    ~XJDownloadManager() {
        cancel = true;
        if (downloader.IsRunning())
            downloader.Wait();
    }
};

GUI_APP_MAIN {
    XJDownloadManager().Run();
}

