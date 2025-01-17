#include "router.h"
#include "apihandler.h"
#include "filehandler.h"
#include <QCoreApplication>

Router::Router(QObject *parent) : stefanfrings::HttpRequestHandler(parent)
{
    apiHandler = new APIHandler(mediaHash, this);
    fileHandler = new FileHandler(this);
    fileHandler->setMediaHash(&mediaHash);
    const QString strApp(QCoreApplication::applicationDirPath()+"/web");
#ifdef CONFIG_UNIX_DATA
    const QString strHome(QDir::homePath()+"/.config/kikoplay/web");
    const QString strSys("/usr/share/kikoplay/web");

    const QFileInfo fileinfoHome(strHome);
    const QFileInfo fileinfoSys(strSys);
    const QFileInfo fileinfoApp(strApp);

    if (fileinfoHome.exists() || fileinfoHome.isDir()) {
        fileHandler->setRoot(strHome);
    } else if (fileinfoSys.exists() || fileinfoSys.isDir()) {
        fileHandler->setRoot(strSys);
    } else {
        fileHandler->setRoot(strApp);
    }
#else
    fileHandler->setRoot(strApp);
#endif
}

void Router::service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response)
{
    QByteArray path = request.getPath();
    if(path.startsWith("/api/"))
    {
        apiHandler->service(request, response);
    }
    else
    {
        fileHandler->service(request, response);
    }
}
