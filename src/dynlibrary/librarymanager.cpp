#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include <boost/filesystem.hpp>

// TODO: Data transport should also be through plug-in
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "logicalaccess/readerproviders/tcpdatatransport.hpp"
#include "logicalaccess/readerproviders/udpdatatransport.hpp"
#include "logicalaccess/cards/keydiversification.hpp"

namespace logicalaccess
{
    const std::string LibraryManager::enumType[3] = { "readers", "cards", "unified" };

    bool LibraryManager::hasEnding(std::string const &fullString, std::string ending)
    {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
        }
        else {
            return false;
        }
    }

    LIBLOGICALACCESS_API LibraryManager *LibraryManager::_singleton = NULL;

    void* LibraryManager::getFctFromName(std::string fctname, LibraryType libraryType)
    {
        void *fct;
        boost::filesystem::directory_iterator end_iter;
        std::string extension = EXTENSION_LIB;

        if (libLoaded.empty())
        {
            scanPlugins();
        }

        for (std::map<std::string, IDynLibrary*>::iterator it = libLoaded.begin(); it != libLoaded.end(); ++it)
        {
            try
            {
                if ((*it).second != NULL && hasEnding((*it).first, enumType[libraryType] + extension) && (fct = (*it).second->getSymbol(fctname.c_str())) != NULL)
                    return fct;
            }
            catch (...) {}
        }

        return NULL;
    }

    boost::shared_ptr<ReaderProvider> LibraryManager::getReaderProvider(const std::string& readertype)
    {
        boost::shared_ptr<ReaderProvider> ret;
        std::string fctname = "get" + readertype + "Reader";

        getprovider getpcscsfct;
        *(void**)(&getpcscsfct) = getFctFromName(fctname, LibraryManager::READERS_TYPE);

        if (getpcscsfct)
        {
            getpcscsfct(&ret);
        }

        return ret;
    }

    boost::shared_ptr<Chip> LibraryManager::getCard(const std::string& cardtype)
    {
        boost::shared_ptr<Chip> ret;
        std::string fctname = "get" + cardtype + "Chip";

        getcard getcardfct;
        *(void**)(&getcardfct) = getFctFromName(fctname, LibraryManager::CARDS_TYPE);

        if (getcardfct)
        {
            getcardfct(&ret);
        }

        return ret;
    }

    boost::shared_ptr<KeyDiversification> LibraryManager::getKeyDiversification(const std::string& keydivtype)
    {
        boost::shared_ptr<KeyDiversification> ret;
        std::string fctname = "get" + keydivtype + "Diversification";

        getdiversification getdiversificationfct;
        *(void**)(&getdiversificationfct) = getFctFromName(fctname, LibraryManager::CARDS_TYPE);

        if (getdiversificationfct)
        {
            getdiversificationfct(&ret);
        }

        return ret;
    }

    boost::shared_ptr<Commands> LibraryManager::getCommands(const std::string& extendedtype)
    {
        boost::shared_ptr<Commands> ret;
        std::string fctname = "get" + extendedtype + "Commands";

        getcommands getcommandsfct;
        *(void**)(&getcommandsfct) = getFctFromName(fctname, LibraryManager::READERS_TYPE);
        if (getcommandsfct)
        {
            getcommandsfct(&ret);
        }

        return ret;
    }

    boost::shared_ptr<DataTransport> LibraryManager::getDataTransport(const std::string& transporttype)
    {
        boost::shared_ptr<DataTransport> ret;

        if (transporttype == TRANSPORT_SERIALPORT)
        {
            ret.reset(new SerialPortDataTransport());
        }
        else if (transporttype == TRANSPORT_UDP)
        {
            ret.reset(new UdpDataTransport());
        }
        else if (transporttype == TRANSPORT_TCP)
        {
            ret.reset(new TcpDataTransport());
        }

        return ret;
    }

    std::vector<std::string> LibraryManager::getAvailableCards()
    {
        return getAvailablePlugins(LibraryManager::CARDS_TYPE);
    }

    std::vector<std::string> LibraryManager::getAvailableReaders()
    {
        return getAvailablePlugins(LibraryManager::READERS_TYPE);
    }

    std::vector<std::string> LibraryManager::getAvailableDataTransports()
    {
        std::vector<std::string> list;

        list.push_back(TRANSPORT_SERIALPORT);
        list.push_back(TRANSPORT_UDP);
        list.push_back(TRANSPORT_TCP);

        return list;
    }

    std::vector<std::string> LibraryManager::getAvailablePlugins(LibraryType libraryType)
    {
        std::vector<std::string> plugins;
        void* fct = NULL;
        boost::filesystem::directory_iterator end_iter;
        std::string fctname;

        if (libraryType == LibraryManager::CARDS_TYPE)
        {
            fctname = "getChipInfoAt";
        }
        else if (libraryType == LibraryManager::READERS_TYPE)
        {
            fctname = "getReaderInfoAt";
        }

        if (libLoaded.empty())
        {
            scanPlugins();
        }

        for (std::map<std::string, IDynLibrary*>::iterator it = libLoaded.begin(); it != libLoaded.end(); ++it)
        {
            try
            {
                if ((*it).second != NULL && (fct = (*it).second->getSymbol(fctname.c_str())) != NULL)
                {
                    getobjectinfoat objectinfoptr;
                    *(void**)(&objectinfoptr) = fct;
                    getAvailablePlugins(plugins, objectinfoptr);
                }
            }
            catch (...) {}
        }

        return plugins;
    }

    void LibraryManager::getAvailablePlugins(std::vector<std::string>& plugins, getobjectinfoat objectinfoptr)
    {
        char objectname[PLUGINOBJECT_MAXLEN];
        memset(objectname, 0x00, sizeof(objectname));
        void* getobjectptr;

        unsigned int i = 0;
        while (objectinfoptr(i, objectname, sizeof(objectname), &getobjectptr))
        {
            plugins.push_back(std::string(objectname));
            memset(objectname, 0x00, sizeof(objectname));
            ++i;
        }
    }

    void LibraryManager::scanPlugins()
    {
        void* fct = NULL;
        boost::filesystem::directory_iterator end_iter;
        std::string extension = EXTENSION_LIB;
        Settings* setting = Settings::getInstance();
        std::string fctname = "getLibraryName";

        for (std::vector<std::string>::iterator it = setting->PluginFolders.begin(); it != setting->PluginFolders.end(); ++it)
        {
            boost::filesystem::path pluginDir(*it);
            if (boost::filesystem::exists(pluginDir) && boost::filesystem::is_directory(pluginDir))
            {
                LOG(LogLevel::PLUGINS) << "Scanning library folder " << pluginDir.string() << " ...";
                for (boost::filesystem::directory_iterator dir_iter(pluginDir); dir_iter != end_iter; ++dir_iter)
                {
                    LOG(LogLevel::PLUGINS) << "Checking library " << dir_iter->path().filename().string() << "...";
                    if ((boost::filesystem::is_regular_file(dir_iter->status()) || boost::filesystem::is_symlink(dir_iter->status()))
                        && dir_iter->path().extension() == extension
                        && (hasEnding(dir_iter->path().filename().string(), enumType[LibraryManager::CARDS_TYPE] + extension)
                        || hasEnding(dir_iter->path().filename().string(), enumType[LibraryManager::READERS_TYPE] + extension)
                        || hasEnding(dir_iter->path().filename().string(), enumType[LibraryManager::UNIFIED_TYPE] + extension)))
                    {
                        try
                        {
                            if (libLoaded.find(dir_iter->path().filename().string()) == libLoaded.end())
                            {
                                IDynLibrary* lib = newDynLibrary(dir_iter->path().string());
                                fct = lib->getSymbol(fctname.c_str());
                                if (fct != NULL)
                                {
                                    LOG(LogLevel::PLUGINS) << "Library " << dir_iter->path().filename().string() << " loaded.";
                                    libLoaded[dir_iter->path().filename().string()] = lib;
                                }
                                else
                                {
                                    LOG(LogLevel::PLUGINS) << "Cannot found library entry point in " << dir_iter->path().filename().string() << ". Skipped.";
                                    delete lib;
                                }
                            }
                            else
                            {
                                LOG(LogLevel::PLUGINS) << "Library " << dir_iter->path().filename().string() << " already loaded. Skipped.";
                            }
                        }
                        catch (...) {}
                    }
                    else
                    {
                        LOG(LogLevel::PLUGINS) << "File " << dir_iter->path().filename().string() << " does not match excepted filenames. Skipped.";
                    }
                }
            }
            else
            {
                LOG(LogLevel::WARNINGS) << "Cannot found plug-in folder " << (*it);
            }
        }
    }
}