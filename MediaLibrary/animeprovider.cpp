#include "animeprovider.h"
#include "Common/threadtask.h"
#include "Script/scriptmanager.h"
#include "Script/libraryscript.h"
#include "globalobjects.h"
namespace
{
    const char *setting_MatchScriptId = "Script/DefaultMatchScript";
}

AnimeProvider::AnimeProvider(QObject *parent) : QObject(parent)
{
    QObject::connect(GlobalObjects::scriptManager, &ScriptManager::scriptChanged, this, [=](ScriptType type, const QString &id){
        if(type==ScriptType::LIBRARY && matchProviderIds.contains(id))
        {
            setMacthProviders();
            if(!matchProviderIds.contains(defaultMatchScriptId))
            {
                if(matchProviders.size()>0)
                {
                    setDefaultMatchScript(matchProviders.first().second);
                }
                else
                {
                    defaultMatchScriptId = "";
                    GlobalObjects::appSetting->setValue(setting_MatchScriptId, defaultMatchScriptId);
                    emit defaultMacthProviderChanged("", "");
                }
            }
            emit matchProviderChanged();
        }
    });
    setMacthProviders();
    defaultMatchScriptId = GlobalObjects::appSetting->value(setting_MatchScriptId).toString();
    if(defaultMatchScript().isEmpty() && !matchProviderIds.isEmpty())
    {
        setDefaultMatchScript(matchProviders.first().second);
    }
}

QList<QPair<QString, QString> > AnimeProvider::getSearchProviders()
{
    QList<QPair<QString, QString>> searchProviders;
    for(auto &script : GlobalObjects::scriptManager->scripts(ScriptType::LIBRARY))
    {
        LibraryScript *libScript = static_cast<LibraryScript *>(script.data());
        searchProviders.append({libScript->name(), libScript->id()});
    }
    return searchProviders;
}

void AnimeProvider::setDefaultMatchScript(const QString &scriptId)
{
    if(!matchProviderIds.contains(scriptId)) return;
    auto script = GlobalObjects::scriptManager->getScript(scriptId).staticCast<LibraryScript>();
    if(!script) return;
    defaultMatchScriptId = scriptId;
    GlobalObjects::appSetting->setValue(setting_MatchScriptId, scriptId);
    emit defaultMacthProviderChanged(script->name(), scriptId);
}

ScriptState AnimeProvider::animeSearch(const QString &scriptId, const QString &keyword, QList<AnimeLite> &results)
{
    auto script = GlobalObjects::scriptManager->getScript(scriptId).staticCast<LibraryScript>();
    if(!script) return "Script invalid";
    ThreadTask task(GlobalObjects::workThread);
    return task.Run([&](){
        return QVariant::fromValue(script->search(keyword, results));
    }).value<ScriptState>();
}

ScriptState AnimeProvider::getDetail(const AnimeLite &base, Anime *anime)
{
    auto script = GlobalObjects::scriptManager->getScript(base.scriptId).staticCast<LibraryScript>();
    if(!script) return "Script invalid";
    ThreadTask task(GlobalObjects::workThread);
    return task.Run([&](){
        return QVariant::fromValue(script->getDetail(base, anime));
    }).value<ScriptState>();
}

ScriptState AnimeProvider::getEp(Anime *anime, QList<EpInfo> &results)
{
    auto script = GlobalObjects::scriptManager->getScript(anime->scriptId()).staticCast<LibraryScript>();
    if(!script) return "Script invalid";
    ThreadTask task(GlobalObjects::workThread);
    return task.Run([&](){
        return QVariant::fromValue(script->getEp(anime, results));
    }).value<ScriptState>();
}

ScriptState AnimeProvider::getTags(Anime *anime, QStringList &results)
{
    auto script = GlobalObjects::scriptManager->getScript(anime->scriptId()).staticCast<LibraryScript>();
    if(!script) return "Script invalid";
    ThreadTask task(GlobalObjects::workThread);
    return task.Run([&](){
        return QVariant::fromValue(script->getTags(anime, results));
    }).value<ScriptState>();
}

ScriptState AnimeProvider::match(const QString &scriptId, const QString &path, MatchResult &result)
{
    auto script = GlobalObjects::scriptManager->getScript(scriptId).staticCast<LibraryScript>();
    if(!script) return "Script invalid";
    ThreadTask task(GlobalObjects::workThread);
    return task.Run([&](){
        return QVariant::fromValue(script->match(path, result));
    }).value<ScriptState>();
}

ScriptState AnimeProvider::menuClick(const QString &mid, Anime *anime)
{
    auto script = GlobalObjects::scriptManager->getScript(anime->scriptId()).staticCast<LibraryScript>();
    if(!script) return "Script invalid";
    ThreadTask task(GlobalObjects::workThread);
    return task.Run([&](){
        return QVariant::fromValue(script->menuClick(mid, anime));
    }).value<ScriptState>();
}

void AnimeProvider::setMacthProviders()
{
    matchProviderIds.clear();
    matchProviders.clear();
    for(auto &script : GlobalObjects::scriptManager->scripts(ScriptType::LIBRARY))
    {
        LibraryScript *libScript = static_cast<LibraryScript *>(script.data());
        if(libScript->supportMatch())
        {
            matchProviders.append({libScript->name(), libScript->id()});
            matchProviderIds.insert(libScript->id());
        }
    }
}