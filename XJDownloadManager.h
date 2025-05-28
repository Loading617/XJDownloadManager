#ifndef _XJDownloadManager_XJDownloadManager_h
#define _XJDownloadManager_XJDownloadManager_h

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define LAYOUTFILE <XJDownloadManager/XJDownloadManager.lay>
#include <CtrlCore/lay.h>

class XJDownloadManager : public WithXJDownloadManagerLayout<TopWindow> {
public:
	XJDownloadManager();
};

#endif
