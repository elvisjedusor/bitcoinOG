// Copyright (c) 2009-2010 Satoshi Nakamoto
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#include "headers.h"




void ExitTimeout(void* parg)
{
#ifdef __WXMSW__
    Sleep(5000);
    ExitProcess(0);
#endif
}

void Shutdown(void* parg)
{
    static CCriticalSection cs_Shutdown;
    static bool fTaken;
    bool fFirstThread;
    CRITICAL_BLOCK(cs_Shutdown)
    {
        fFirstThread = !fTaken;
        fTaken = true;
    }
    static bool fExit;
    if (fFirstThread)
    {
        fShutdown = true;
        nTransactionsUpdated++;
        DBFlush(false);
        StopNode();
        DBFlush(true);
        CreateThread(ExitTimeout, NULL);
        Sleep(50);
        printf("Bitok exiting\n\n");
        fExit = true;
        exit(0);
    }
    else
    {
        while (!fExit)
            Sleep(500);
        Sleep(100);
        ExitThread(0);
    }
}






//////////////////////////////////////////////////////////////////////////////
//
// Startup folder
//

#ifdef __WXMSW__
typedef WINSHELLAPI BOOL (WINAPI *PSHGETSPECIALFOLDERPATHA)(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate);

string MyGetSpecialFolderPath(int nFolder, bool fCreate)
{
    char pszPath[MAX_PATH+100] = "";

    // SHGetSpecialFolderPath is not usually available on NT 4.0
    HMODULE hShell32 = LoadLibraryA("shell32.dll");
    if (hShell32)
    {
        PSHGETSPECIALFOLDERPATHA pSHGetSpecialFolderPath =
            (PSHGETSPECIALFOLDERPATHA)GetProcAddress(hShell32, "SHGetSpecialFolderPathA");
        if (pSHGetSpecialFolderPath)
            (*pSHGetSpecialFolderPath)(NULL, pszPath, nFolder, fCreate);
        FreeModule(hShell32);
    }

    // Backup option
    if (pszPath[0] == '\0')
    {
        if (nFolder == CSIDL_STARTUP)
        {
            strcpy(pszPath, getenv("USERPROFILE"));
            strcat(pszPath, "\\Start Menu\\Programs\\Startup");
        }
        else if (nFolder == CSIDL_APPDATA)
        {
            strcpy(pszPath, getenv("APPDATA"));
        }
    }

    return pszPath;
}

string StartupShortcutPath()
{
    return MyGetSpecialFolderPath(CSIDL_STARTUP, true) + "\\Bitok.lnk";
}

bool GetStartOnSystemStartup()
{
    return wxFileExists(StartupShortcutPath());
}

void SetStartOnSystemStartup(bool fAutoStart)
{
    // If the shortcut exists already, remove it for updating
    remove(StartupShortcutPath().c_str());

    if (fAutoStart)
    {
        CoInitialize(NULL);

        // Get a pointer to the IShellLink interface.
        IShellLink* psl = NULL;
        HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL,
                                CLSCTX_INPROC_SERVER, IID_IShellLink,
                                reinterpret_cast<void**>(&psl));

        if (SUCCEEDED(hres))
        {
            // Get the current executable path
            TCHAR pszExePath[MAX_PATH];
            GetModuleFileName(NULL, pszExePath, sizeof(pszExePath));

            // Set the path to the shortcut target
            psl->SetPath(pszExePath);
            PathRemoveFileSpec(pszExePath);
            psl->SetWorkingDirectory(pszExePath);
            psl->SetShowCmd(SW_SHOWMINNOACTIVE);

            // Query IShellLink for the IPersistFile interface for
            // saving the shortcut in persistent storage.
            IPersistFile* ppf = NULL;
            hres = psl->QueryInterface(IID_IPersistFile,
                                       reinterpret_cast<void**>(&ppf));
            if (SUCCEEDED(hres))
            {
                WCHAR pwsz[MAX_PATH];
                // Ensure that the string is ANSI.
                MultiByteToWideChar(CP_ACP, 0, StartupShortcutPath().c_str(), -1, pwsz, MAX_PATH);
                // Save the link by calling IPersistFile::Save.
                hres = ppf->Save(pwsz, TRUE);
                ppf->Release();
            }
            psl->Release();
        }
        CoUninitialize();
    }
}
#else
bool GetStartOnSystemStartup() { return false; }
void SetStartOnSystemStartup(bool fAutoStart) { }
#endif







//////////////////////////////////////////////////////////////////////////////
//
// CMyApp
//

#if wxUSE_GUI
// Define a new application
class CMyApp: public wxApp
{
public:
    wxLocale m_locale;

    CMyApp(){};
    ~CMyApp(){};
    bool OnInit();
    bool OnInit2();
    int OnExit();

    // Hook Initialize so we can start without GUI
    virtual bool Initialize(int& argc, wxChar** argv);

    // 2nd-level exception handling: we get all the exceptions occurring in any
    // event handler here
    virtual bool OnExceptionInMainLoop();

    // 3rd, and final, level exception handling: whenever an unhandled
    // exception is caught, this function is called
    virtual void OnUnhandledException();

    // and now for something different: this function is called in case of a
    // crash (e.g. dereferencing null pointer, division by 0, ...)
    virtual void OnFatalException();
};

IMPLEMENT_APP(CMyApp)

bool CMyApp::Initialize(int& argc, wxChar** argv)
{
    if (argc > 1 && argv[1][0] != '-' && (!fWindows || argv[1][0] != '/') &&
        wxString(argv[1]) != "start")
    {
        fCommandLine = true;
    }
    else if (!fGUI)
    {
        fDaemon = true;
    }
    else
    {
        // wxApp::Initialize will remove environment-specific parameters,
        // so it's too early to call ParseParameters yet
        for (int i = 1; i < argc; i++)
        {
            wxString str = argv[i];
            #ifdef __WXMSW__
            if (str.size() >= 1 && str[0] == '/')
                str[0] = '-';
            str = str.MakeLower();
            #endif
            // haven't decided which argument to use for this yet
            if (str == "-daemon" || str == "-d" || str == "start")
                fDaemon = true;
        }
    }

#ifdef __WXGTK__
    if (fDaemon || fCommandLine)
    {
        // Call the original Initialize while suppressing error messages
        // and ignoring failure.  If unable to initialize GTK, it fails
        // near the end so hopefully the last few things don't matter.
        {
            wxLogNull logNo;
            wxApp::Initialize(argc, argv);
        }

        if (fDaemon)
        {
            // Daemonize
            pid_t pid = fork();
            if (pid < 0)
            {
                fprintf(stderr, "Error: fork() returned %d errno %d\n", pid, errno);
                return false;
            }
            if (pid > 0)
                pthread_exit((void*)0);
        }

        return true;
    }
#endif

    return wxApp::Initialize(argc, argv);
}

bool CMyApp::OnInit()
{
    bool fRet = false;
    try
    {
        fRet = OnInit2();
    }
    catch (std::exception& e) {
        PrintException(&e, "OnInit()");
    } catch (...) {
        PrintException(NULL, "OnInit()");
    }
    if (!fRet)
        Shutdown(NULL);
    return fRet;
}

extern int g_isPainting;

bool CMyApp::OnInit2()
{
#ifdef _MSC_VER
    // Turn off microsoft heap dump noise
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, CreateFileA("NUL", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0));
#endif
#if _MSC_VER >= 1400
    // Disable confusing "helpful" text message on abort, ctrl-c
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
#if defined(__WXMSW__) && defined(__WXDEBUG__) && wxUSE_GUI
    // Disable malfunctioning wxWidgets debug assertion
    g_isPainting = 10000;
#endif
#if wxUSE_GUI
    wxImage::AddHandler(new wxPNGHandler);
#endif
// Bitok: Updated app name for data directory
#if defined(__WXMSW__ ) || defined(__WXMAC__)
    SetAppName("Bitok");
#else
    SetAppName("bitokd");
#endif
#ifndef __WXMSW__
    umask(077);
#endif
#ifdef __WXMSW__
#if wxUSE_UNICODE
    // Hack to set wxConvLibc codepage to UTF-8 on Windows,
    // may break if wxMBConv_win32 implementation in strconv.cpp changes.
    class wxMBConv_win32 : public wxMBConv
    {
    public:
        long m_CodePage;
        size_t m_minMBCharWidth;
    };
    if (((wxMBConv_win32*)&wxConvLibc)->m_CodePage == CP_ACP)
        ((wxMBConv_win32*)&wxConvLibc)->m_CodePage = CP_UTF8;
#endif
#endif

    // Load locale/<lang>/LC_MESSAGES/bitok.mo language file
    m_locale.Init(wxLANGUAGE_DEFAULT, 0);
    m_locale.AddCatalogLookupPathPrefix("locale");
    if (!fWindows)
    {
        m_locale.AddCatalogLookupPathPrefix("/usr/share/locale");
        m_locale.AddCatalogLookupPathPrefix("/usr/local/share/locale");
    }
    m_locale.AddCatalog("wxstd"); // wxWidgets standard translations, if any
    m_locale.AddCatalog("bitok");

    //
    // Parameters
    //
    if (fCommandLine)
    {
        int ret = CommandLineRPC(argc, argv);
        exit(ret);
    }

    ParseParameters(argc, argv);
    if (mapArgs.count("-?") || mapArgs.count("--help"))
    {
        wxString strUsage = string() +
          _("Usage:") + "\t\t\t\t\t\t\t\t\t\t\n" +
            "  bitok [options]       \t" + "\n" +
            "  bitok [command]       \t" + _("Send command to bitokd running with -server or -daemon\n") +
            "  bitok [command] -?    \t" + _("Get help for a command\n") +
            "  bitok help            \t" + _("List commands\n") +
          _("Options:\n") +
            "  -gen            \t  " + _("Generate coins\n") +
            "  -gen=0          \t  " + _("Don't generate coins\n") +
            "  -min            \t  " + _("Start minimized\n") +
            "  -datadir=<dir>  \t  " + _("Specify data directory\n") +
            "  -proxy=<ip:port>\t  " + _("Connect through socks4 proxy\n") +
            "  -addnode=<ip>   \t  " + _("Add a node to connect to\n") +
            "  -connect=<ip>   \t  " + _("Connect only to the specified node\n") +
            "  -server         \t  " + _("Accept command line and JSON-RPC commands\n") +
            "  -daemon         \t  " + _("Run in the background as a daemon and accept commands\n") +
            "  -?              \t  " + _("This help message\n");


        if (fWindows && fGUI)
        {
            // Tabs make the columns line up in the message box
            wxMessageBox(strUsage, "Bitok", wxOK);
        }
        else
        {
            // Remove tabs
            strUsage.Replace("\t", "");
            fprintf(stderr, "%s", ((string)strUsage).c_str());
        }
        return false;
    }

    if (mapArgs.count("-datadir"))
        strlcpy(pszSetDataDir, mapArgs["-datadir"].c_str(), sizeof(pszSetDataDir));

    if (mapArgs.count("-debug"))
    {
        fDebug = true;
        fPrintToConsole = true;
    }

    if (mapArgs.count("-printtodebugger"))
        fPrintToDebugger = true;

    if (!fDebug && !pszSetDataDir[0])
        ShrinkDebugFile();
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("Bitok version %d.%d.%d%s, OS version %s\n", VERSION/10000, (VERSION/100)%100, VERSION%100, pszSubVer, ((string)wxGetOsDescription()).c_str());
    printf("System default language is %d %s\n", m_locale.GetSystemLanguage(), ((string)m_locale.GetSysName()).c_str());
    printf("Language file %s (%s)\n", (string("locale/") + (string)m_locale.GetCanonicalName() + "/LC_MESSAGES/bitok.mo").c_str(), ((string)m_locale.GetLocale()).c_str());

    if (mapArgs.count("-loadblockindextest"))
    {
        CTxDB txdb("r");
        txdb.LoadBlockIndex();
        PrintBlockTree();
        return false;
    }

    //
    // Limit to single instance per user
    // Required to protect the database files if we're going to keep deleting log.*
    //
#ifdef __WXMSW__
    // todo: wxSingleInstanceChecker wasn't working on Linux, never deleted its lock file
    //  maybe should go by whether successfully bind port 8333 instead
    wxString strMutexName = wxString("bitok_running.") + getenv("HOMEPATH");
    for (int i = 0; i < strMutexName.size(); i++)
        if (!isalnum(strMutexName[i]))
            strMutexName[i] = '.';
    wxSingleInstanceChecker* psingleinstancechecker = new wxSingleInstanceChecker(strMutexName);
    if (psingleinstancechecker->IsAnotherRunning())
    {
        printf("Existing instance found\n");
        unsigned int nStart = GetTime();
        loop
        {
            // TODO: find out how to do this in Linux, or replace with wxWidgets commands
            // Show the previous instance and exit
            HWND hwndPrev = FindWindowA("wxWindowClassNR", "Bitok");
            if (hwndPrev)
            {
                if (IsIconic(hwndPrev))
                    ShowWindow(hwndPrev, SW_RESTORE);
                SetForegroundWindow(hwndPrev);
                return false;
            }

            if (GetTime() > nStart + 60)
                return false;

            // Resume this instance if the other exits
            delete psingleinstancechecker;
            Sleep(1000);
            psingleinstancechecker = new wxSingleInstanceChecker(strMutexName);
            if (!psingleinstancechecker->IsAnotherRunning())
                break;
        }
    }
#endif

    // Bind to the port early so we can tell if another instance is already running.
    // This is a backup to wxSingleInstanceChecker, which doesn't work on Linux.
    string strErrors;
    if (!BindListenPort(strErrors))
    {
        wxMessageBox(strErrors, "Bitok");
        return false;
    }

    //
    // Load data files
    //
    if (fDaemon)
        fprintf(stdout, "Bitok server starting\n");
    strErrors = "";
    int64 nStart;

    printf("Loading addresses...\n");
    nStart = GetTimeMillis();
    if (!LoadAddresses())
        strErrors += _("Error loading addr.dat      \n");
    printf(" addresses   %15" PRI64d "ms\n", GetTimeMillis() - nStart);

    printf("Loading block index...\n");
    nStart = GetTimeMillis();
    if (!LoadBlockIndex())
        strErrors += _("Error loading blkindex.dat      \n");
    printf(" block index %15" PRI64d "ms\n", GetTimeMillis() - nStart);

    printf("Loading wallet...\n");
    nStart = GetTimeMillis();
    bool fFirstRun;
    if (!LoadWallet(fFirstRun))
        strErrors += _("Error loading wallet.dat      \n");
    printf(" wallet      %15" PRI64d "ms\n", GetTimeMillis() - nStart);

    // Initialize nLimitProcessors to CPU count - 1 if not set (leave 1 core for system)
    if (nLimitProcessors == 1 && fFirstRun)
    {
#if wxUSE_GUI
        int nProcessors = wxThread::GetCPUCount();
#else
        int nProcessors = sysconf(_SC_NPROCESSORS_ONLN);
#endif
        if (nProcessors > 1)
        {
            nLimitProcessors = nProcessors - 1;
            CWalletDB().WriteSetting("nLimitProcessors", nLimitProcessors);
            printf("Initialized nLimitProcessors to %d (leaving 1 core for system)\n", nLimitProcessors);
        }
    }

    printf("Done loading\n");

        //// debug print
        printf("mapBlockIndex.size() = %d\n",   mapBlockIndex.size());
        printf("nBestHeight = %d\n",            nBestHeight);
        printf("mapKeys.size() = %d\n",         mapKeys.size());
        printf("mapPubKeys.size() = %d\n",      mapPubKeys.size());
        printf("mapWallet.size() = %d\n",       mapWallet.size());
        printf("mapAddressBook.size() = %d\n",  mapAddressBook.size());

    if (!strErrors.empty())
    {
        wxMessageBox(strErrors, "Bitcoin");
        return false;
    }

    // Add wallet transactions that aren't already in a block to mapTransactions
    ReacceptWalletTransactions();

    //
    // Parameters
    //
    if (mapArgs.count("-printblockindex") || mapArgs.count("-printblocktree"))
    {
        PrintBlockTree();
        return false;
    }

    if (mapArgs.count("-printblock"))
    {
        string strMatch = mapArgs["-printblock"];
        int nFound = 0;
        for (map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.begin(); mi != mapBlockIndex.end(); ++mi)
        {
            uint256 hash = (*mi).first;
            if (strncmp(hash.ToString().c_str(), strMatch.c_str(), strMatch.size()) == 0)
            {
                CBlockIndex* pindex = (*mi).second;
                CBlock block;
                block.ReadFromDisk(pindex);
                block.BuildMerkleTree();
                block.print();
                printf("\n");
                nFound++;
            }
        }
        if (nFound == 0)
            printf("No blocks matching %s were found\n", strMatch.c_str());
        return false;
    }

    if (mapArgs.count("-gen"))
    {
        if (mapArgs["-gen"].empty())
            fGenerateBitcoins = true;
        else
            fGenerateBitcoins = (atoi(mapArgs["-gen"].c_str()) != 0);
    }

    if (mapArgs.count("-solo"))
    {
        fSoloMining = true;
        printf("Solo mining mode enabled - mining without peers\n");
    }

    if (mapArgs.count("-proxy"))
    {
        fUseProxy = true;
        addrProxy = CAddress(mapArgs["-proxy"]);
        if (!addrProxy.IsValid())
        {
            wxMessageBox(_("Invalid -proxy address"), "Bitok");
            return false;
        }
    }

    if (mapArgs.count("-addnode"))
    {
        foreach(string strAddr, mapMultiArgs["-addnode"])
        {
            CAddress addr(strAddr, NODE_NETWORK);
            addr.nTime = 0; // so it won't relay unless successfully connected
            if (addr.IsValid())
                AddAddress(addr);
        }
    }

    //
    // Create the main window and start the node
    //
    if (!fDaemon)
        CreateMainWindow();

    if (!CheckDiskSpace())
        return false;

    RandAddSeedPerfmon();

    if (!CreateThread(StartNode, NULL))
        wxMessageBox("Error: CreateThread(StartNode) failed", "Bitok");

    if (mapArgs.count("-server") || fDaemon)
        CreateThread(ThreadRPCServer, NULL);

    if (fFirstRun)
        SetStartOnSystemStartup(true);

    return true;
}

int CMyApp::OnExit()
{
    Shutdown(NULL);
    return wxApp::OnExit();
}

bool CMyApp::OnExceptionInMainLoop()
{
    try
    {
        throw;
    }
    catch (std::exception& e)
    {
        PrintException(&e, "CMyApp::OnExceptionInMainLoop()");
        wxLogWarning("Exception %s %s", typeid(e).name(), e.what());
        Sleep(1000);
        throw;
    }
    catch (...)
    {
        PrintException(NULL, "CMyApp::OnExceptionInMainLoop()");
        wxLogWarning("Unknown exception");
        Sleep(1000);
        throw;
    }
    return true;
}

void CMyApp::OnUnhandledException()
{
    // this shows how we may let some exception propagate uncaught
    try
    {
        throw;
    }
    catch (std::exception& e)
    {
        PrintException(&e, "CMyApp::OnUnhandledException()");
        wxLogWarning("Exception %s %s", typeid(e).name(), e.what());
        Sleep(1000);
        throw;
    }
    catch (...)
    {
        PrintException(NULL, "CMyApp::OnUnhandledException()");
        wxLogWarning("Unknown exception");
        Sleep(1000);
        throw;
    }
}

void CMyApp::OnFatalException()
{
    wxMessageBox(_("Program has crashed and will terminate.  "), "Bitok", wxOK | wxICON_ERROR);
}

#else

bool AppInit(int argc, char* argv[])
{
#ifdef _MSC_VER
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, CreateFileA("NUL", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0));
#endif
#if _MSC_VER >= 1400
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
#ifndef __WXMSW__
    umask(077);
#endif

    // Check if any argument is an RPC command (not starting with -)
    // RPC commands can appear after options like -datadir
    int rpcArgIndex = -1;
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-')
        {
            rpcArgIndex = i;
            fCommandLine = true;
            break;
        }
    }

    if (!fCommandLine)
    {
        fDaemon = true;
    }

    if (fCommandLine)
    {
        // Parse parameters first so -datadir is available for RPC
        ParseParameters(argc, argv);

        // Build new argv: program name + RPC command + RPC args
        // CommandLineRPC expects argv[1] to be the method name
        int rpcArgc = 1 + (argc - rpcArgIndex);  // prog + command + args
        char** rpcArgv = new char*[rpcArgc];
        rpcArgv[0] = argv[0];  // program name
        for (int i = rpcArgIndex; i < argc; i++)
            rpcArgv[1 + i - rpcArgIndex] = argv[i];

        int ret = CommandLineRPC(rpcArgc, rpcArgv);
        delete[] rpcArgv;
        exit(ret);
    }

    ParseParameters(argc, argv);

    if (mapArgs.count("-?") || mapArgs.count("--help"))
    {
        string strUsage = string() +
          _("Usage:") + "\n" +
            "  bitokd [options]       \n" +
            "  bitokd [command]       " + _("Send command to bitokd running with -server or -daemon\n") +
            "  bitokd [command] -?    " + _("Get help for a command\n") +
            "  bitokd help            " + _("List commands\n") +
          _("Options:\n") +
            "  -gen              " + _("Generate coins\n") +
            "  -gen=0            " + _("Don't generate coins\n") +
            "  -datadir=<dir>    " + _("Specify data directory\n") +
            "  -proxy=<ip:port>  " + _("Connect through socks4 proxy\n") +
            "  -addnode=<ip>     " + _("Add a node to connect to\n") +
            "  -connect=<ip>     " + _("Connect only to the specified node\n") +
            "  -server           " + _("Accept command line and JSON-RPC commands\n") +
            "  -daemon           " + _("Run in the background as a daemon and accept commands\n") +
            "  -?                " + _("This help message\n");
        fprintf(stderr, "%s", strUsage.c_str());
        return false;
    }

    if (mapArgs.count("-datadir"))
        strlcpy(pszSetDataDir, mapArgs["-datadir"].c_str(), sizeof(pszSetDataDir));

    if (mapArgs.count("-debug"))
    {
        fDebug = true;
        fPrintToConsole = true;
    }

    if (mapArgs.count("-printtodebugger"))
        fPrintToDebugger = true;

    if (!fDebug && !pszSetDataDir[0])
        ShrinkDebugFile();

    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("Bitok version %d.%d.%d%s\n", VERSION/10000, (VERSION/100)%100, VERSION%100, pszSubVer);

    if (mapArgs.count("-loadblockindextest"))
    {
        CTxDB txdb("r");
        txdb.LoadBlockIndex();
        PrintBlockTree();
        return false;
    }

    string strErrors;
    if (!BindListenPort(strErrors))
    {
        fprintf(stderr, "%s\n", strErrors.c_str());
        return false;
    }

    printf("Loading addresses...\n");
    if (!LoadAddresses())
        fprintf(stderr, "Warning: Error loading addresses\n");

    printf("Loading block index...\n");
    if (!LoadBlockIndex())
    {
        fprintf(stderr, "Error loading block index\n");
        return false;
    }

    bool fFirstRun;
    if (!LoadWallet(fFirstRun))
    {
        fprintf(stderr, "Error loading wallet\n");
        return false;
    }

    // Initialize nLimitProcessors to CPU count - 1 if not set (leave 1 core for system)
    if (nLimitProcessors == 1 && fFirstRun)
    {
#if wxUSE_GUI
        int nProcessors = wxThread::GetCPUCount();
#else
        int nProcessors = sysconf(_SC_NPROCESSORS_ONLN);
#endif
        if (nProcessors > 1)
        {
            nLimitProcessors = nProcessors - 1;
            CWalletDB().WriteSetting("nLimitProcessors", nLimitProcessors);
            printf("Initialized nLimitProcessors to %d (leaving 1 core for system)\n", nLimitProcessors);
        }
    }

    printf("Done loading\n");

    if (mapArgs.count("-printblockindex") || mapArgs.count("-printblocktree"))
    {
        PrintBlockTree();
        return false;
    }

    if (mapArgs.count("-printblock"))
    {
        string strMatch = mapArgs["-printblock"];
        int nFound = 0;
        for (map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.begin(); mi != mapBlockIndex.end(); ++mi)
        {
            uint256 hash = (*mi).first;
            if (strncmp(hash.ToString().c_str(), strMatch.c_str(), strMatch.size()) == 0)
            {
                CBlockIndex* pindex = (*mi).second;
                CBlock block;
                block.ReadFromDisk(pindex);
                block.BuildMerkleTree();
                block.print();
                printf("\n");
                nFound++;
            }
        }
        if (nFound == 0)
            printf("No blocks matching %s were found\n", strMatch.c_str());
        return false;
    }

    if (mapArgs.count("-gen"))
    {
        if (mapArgs["-gen"].empty())
            fGenerateBitcoins = true;
        else
            fGenerateBitcoins = (atoi(mapArgs["-gen"].c_str()) != 0);
    }

    if (mapArgs.count("-solo"))
    {
        fSoloMining = true;
        printf("Solo mining mode enabled - mining without peers\n");
    }

    if (mapArgs.count("-proxy"))
    {
        fUseProxy = true;
        addrProxy = CAddress(mapArgs["-proxy"]);
        if (!addrProxy.IsValid())
        {
            fprintf(stderr, "Invalid -proxy address\n");
            return false;
        }
    }

    if (mapArgs.count("-paytxfee"))
    {
        if (!ParseMoney(mapArgs["-paytxfee"], nTransactionFee))
        {
            fprintf(stderr, "Invalid -paytxfee amount\n");
            return false;
        }
        if (nTransactionFee > 0.25 * COIN)
            fprintf(stderr, "Warning: -paytxfee is set very high\n");
    }

    if (fDaemon && !fDebug)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "Error: fork() returned %d errno %d\n", pid, errno);
            return false;
        }
        if (pid > 0)
            exit(0);

        setsid();
    }

    if (!CheckDiskSpace())
        return false;

    RandAddSeedPerfmon();

    if (!CreateThread(StartNode, NULL))
    {
        fprintf(stderr, "Error: CreateThread(StartNode) failed\n");
        return false;
    }

    if (mapArgs.count("-server") || fDaemon)
        CreateThread(ThreadRPCServer, NULL);

    return true;
}

int main(int argc, char* argv[])
{
    bool fRet = false;
    try
    {
        fRet = AppInit(argc, argv);
    }
    catch (std::exception& e)
    {
        PrintException(&e, "AppInit()");
    }
    catch (...)
    {
        PrintException(NULL, "AppInit()");
    }

    if (fRet)
    {
        while (!fShutdown)
            Sleep(5000);
    }

    Shutdown(NULL);
    return 0;
}

#endif
