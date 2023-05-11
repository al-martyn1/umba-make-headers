#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <cstdio>
#include <sstream>

int qtVersion = 5;


//----------------------------------------------------------------------------
struct WarningFlags
{

    bool     warnMultipleIncludes;
    bool     warnUppercase;


    static WarningFlags wf_all()
    {
        WarningFlags of;

        of.warnMultipleIncludes    = true;
        of.warnUppercase           = true;

        return of;
    }

    static WarningFlags wf_default()
    {
        WarningFlags of;

        of.warnMultipleIncludes    = false;
        of.warnUppercase           = false;

        return of;
    }

    static WarningFlags wf_none()
    {
        WarningFlags of;

        of.warnMultipleIncludes    = false;
        of.warnUppercase           = false;

        return of;
    }

};


struct InputFileOptions
{
    std::string         namespaceName;
    std::string         qtModule;
    std::string         qtModulePath;
};





//----------------------------------------------------------------------------
inline
bool isEmpty( const std::string &str )
{
    std::string::size_type i = 0, sz = str.size();

    for(; i!=sz; ++i)
    {
        if (str[i]==' ' || str[i]=='\t')
           continue;

        return false;
    }

    return true;
}

//----------------------------------------------------------------------------
inline
std::string makeGuard( const std::string &nameSpace
                     , const std::string &typeName
                     , const std::string &incName
                     )
{
    std::ostringstream ossGuard;

    ossGuard << "UMBA_MAKE_HEADERS_GENERATED_";

    if (!nameSpace.empty())
        ossGuard << nameSpace << "_";

    ossGuard << typeName << "_";

    if (!incName.empty())
    {
        ossGuard << "from_" << incName;
    }

    ossGuard << "_INCLUDE_GUARD";

    return ossGuard.str();
}

//----------------------------------------------------------------------------
inline
std::string correctPathSeparators( std::string path )
{
    std::string::size_type i = 0, sz = path.size();

    for(; i!=sz; ++i)
    {
        if (path[i]=='\\')
            path[i] = '/';
    }

    return path;    
}

//----------------------------------------------------------------------------
inline
std::string removeDupChars( const std::string &s )
{
    std::string res; res.reserve(s.size());

    char prevChar = 0;

    std::string::size_type i = 0, sz = s.size();

    for(; i!=sz; ++i)
    {
        char ch = s[i];
        if (ch==prevChar)
            continue;

        res.append(1,ch);

        prevChar = ch;
    }

    // std::cout << "removeDupChars: " << res << std::endl;

    return res;
}

//----------------------------------------------------------------------------
inline
std::string makeLowerString( std::string s )
{
    std::string::size_type i = 0, sz = s.size();

    for(; i!=sz; ++i)
    {
        if (s[i]>='A' && s[i]<='Z')
        {
            s[i] = (char)(s[i]-'A' + 'a');
        }
        else
        {
            continue;
        }
    }

    return s;

}

//----------------------------------------------------------------------------
inline
std::string makeGuardFromNamespace( std::string ns )
{
    ns = removeDupChars(ns);

    //std::string res; res.reserve(ns.size());

    std::string::size_type i = 0, sz = ns.size();

    for(; i!=sz; ++i)
    {
        if (ns[i]=='_' || (ns[i]>='A' && ns[i]<='Z') || (ns[i]>='a' && ns[i]<='z') || (ns[i]>='0' && ns[i]<='9'))
        {
            //res.append(1,ns[i]);
            //ns[i]
            continue;
        }
        else
        {
            ns[i] = '_';
        }
    }

    // std::cout << "makeGuardFromNamespace: " << ns << std::endl;

    return ns;    
}

//----------------------------------------------------------------------------
inline
std::string getPath( std::string p )
{
    p = correctPathSeparators(p);
    std::string::size_type sepPos = p.find_last_of( '/' );
    if (sepPos==std::string::npos)
        return std::string(); 
    return std::string( p, sepPos );
}

//----------------------------------------------------------------------------
inline
std::string appendPathSep( std::string p )
{
    if (!p.empty())
    {
        if (p[p.size()-1]!='/')
            p += "/";
    }

    return p;
}

//----------------------------------------------------------------------------
inline
bool isFullUppercase( const std::string &str )
{
    std::string::size_type i = 0, sz = str.size();

    for(; i!=sz; ++i)
    {
        if (str[i]=='_')
            continue;

        if (str[i]>='A' && str[i]<='Z')
            continue;

        return false;
    }

    return true;

}

//----------------------------------------------------------------------------
std::string toUpper(std::string s)
{
    for( auto &ch : s )
    {
        if (ch>='a' && ch<='z')
            ch = ch - 'a' + 'A';
    }

    return s;
}

//----------------------------------------------------------------------------
inline
std::string appendPath( const std::string &p, const std::string &n )
{
    if (p.empty())
        return n;

    return appendPathSep(p) + n;
}

//----------------------------------------------------------------------------
// https://ideone.com/OpPkiu
inline
std::string unquote( const std::string &s, char qLeft, char qRight = 0 )
{
    if (s.size()<2)
        return s;

    if (qRight==0)
        qRight = qLeft;

    if (s[0]!=qLeft || s[s.size()-1]!=qRight)
        return s;

    return std::string( s, 1, s.size()-2 );
}

//----------------------------------------------------------------------------
inline
bool isIncludeQuoted( const std::string &s )
{
    if (s.size()<2)
        return false;

    if (s[0]=='\"' && s[s.size()-1]=='\"')
        return true;

    if (s[0]=='<' && s[s.size()-1]=='>')
        return true;

    return false;
}

//----------------------------------------------------------------------------
inline
std::string quoteInclude( const std::string &s, bool bUserInclude )
{
    if (!bUserInclude)
        return std::string("<") + s + std::string(">");
    else
        return std::string("\"") + s + std::string("\"");
}

//----------------------------------------------------------------------------
inline
bool readNamelists( const std::string                              &namelistName
                  , std::vector< std::string >                     &namesOrder
                  , std::set<std::string>                          &usedNames
                  , std::map< std::string, std::set<std::string> > &names
                  , InputFileOptions                               &inputFileOptions
                  , std::map< std::string, std::string >           &qtModules
                  , std::map< std::string, std::string >           &qtModulePaths
                  , const std::set<std::string>                    &defines
                  , const WarningFlags                             &warningFlags
                  )
{
    using std::cout;
    using std::cerr;
    using std::endl;

    std::ifstream istrteamNamelist(namelistName.c_str());

    if (!istrteamNamelist)
    {
        cerr << "Failed to open input namelist file '" << namelistName << "'" << endl;
        return false;
    }


    std::set<std::string> lastIncludes;

    std::string typeInfoStr;


    while( std::getline(istrteamNamelist, typeInfoStr ) )
    {

        if (typeInfoStr.empty())
            continue;

        if (isEmpty(typeInfoStr))
            continue;

        if (typeInfoStr[0]=='#')
        {
            std::istringstream iss(typeInfoStr);
            std::string kwd, name;

            iss >> kwd;

            if (kwd=="#break")
                return true;

            iss >> name;


            if (kwd=="#break_ifdef")
            {
                std::set<std::string>::const_iterator it = defines.find(name);
                if (it!=defines.end())
                    return true;
            }

            if (kwd=="#break_ifndef")
            {
                std::set<std::string>::const_iterator it = defines.find(name);
                if (it==defines.end())
                    return true;
            }

            if (kwd=="#namespace")
            {
                inputFileOptions.namespaceName = name;
                continue;
            }

            if (kwd=="#qt_module")
            {
                inputFileOptions.qtModule = name;
                continue;
            }

            if (kwd=="#qt_module_path")
            {
                inputFileOptions.qtModulePath = name; // unquote(name, '\"');
                continue;
            }

            if (kwd!="#include")
                continue;

            if (name.empty())
            {
                cerr << "Empty include statement" << endl;
                return false;
            }

            std::string curPath = getPath( namelistName );

            std::string includedFullName = appendPath( curPath, name );

            if ( !readNamelists( includedFullName, namesOrder, usedNames, names, inputFileOptions, qtModules, qtModulePaths, defines, warningFlags ))
               return false;

            continue;
        }


        if (typeInfoStr[0]=='#' || typeInfoStr[0]==';')
            continue;

        std::istringstream iss(typeInfoStr);

        std::string typeName;
        iss >> typeName;

        if (usedNames.find(typeName)==usedNames.end())
        {
            namesOrder.push_back(typeName);
        }

        usedNames.insert(typeName);

        // !!! namesOrder

        std::string tmpName;

        std::set<std::string> curIncludes;

        while(iss>>tmpName)
        {
            if (tmpName.empty())
                continue;

            if (tmpName[0]=='#' || tmpName[0]==';')
                break;

            curIncludes.insert(tmpName);

            //names[typeName].insert(tmpName);
        }

        if (curIncludes.empty())
            curIncludes  = lastIncludes;
        else
            lastIncludes = curIncludes;

        names[typeName].insert( curIncludes.begin(), curIncludes.end() );

        if (!inputFileOptions.qtModule.empty())
        {
            qtModules[typeName] = inputFileOptions.qtModule;

            if (!inputFileOptions.qtModulePath.empty())
                qtModulePaths[inputFileOptions.qtModule] = inputFileOptions.qtModulePath;
        }

        /*
        if (typeName=="swap")
        {
            cout << "swap includes:" << endl;
            for( std::set<std::string>::const_iterator it=curIncludes.begin(); it!=curIncludes.end(); ++it )
                cout << *it << endl;
        }
        */

    }

    return true;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
int main( int argc, char *argv[])
{
    using std::cout;
    using std::cerr;
    using std::endl;


    InputFileOptions    inputFileOptions;

    //std::string namespaceName;
    std::string namespaceGuardName;
    //std::string incPathPrefix;
    std::string namelistName = "namelist.txt";
    bool        includeModeUser = false;
    bool        cleanMode       = false;
    bool        generateGitAdd  = false;

    std::map< std::string, std::string > qtModules;
    std::map< std::string, std::string > qtModulePaths;

    std::set< std::string>      defines;

    WarningFlags                warningFlags = WarningFlags::wf_default();



    std::vector< std::string > opts;

    for(int argN=1; argN!=argc; ++argN)
    {
        std::string opt = argv[argN];
        //cout << "Option: " << opt << endl;

        std::string::size_type eqPos = opt.find( '=' );
        if (eqPos==std::string::npos)
        {
            opts.push_back(opt);
            continue;
        }

        opts.push_back( std::string( opt, 0, eqPos ) );
        opts.push_back( std::string( opt, eqPos+1 ) );

    }

    std::vector< std::string >::const_iterator optIt = opts.begin();

    while( optIt!=opts.end() )
    {
        //const std::string &opt = *optIt;
        if (*optIt=="-h" || *optIt=="--help")
        {
            cout << "Usage: umba-make-headers [options] [namelist.txt]" << endl
                 << "where options are: " << endl
                 << "    -h, --help                     - print this help and exit" << endl
                 << "    -w, --where                    - print full path to self executable" << endl
                 << "    -g, --git-add                  - turns on 'git-add.bat' file generation" << endl
                 << "    -5, --qt5                      - set Qt version to 5" << endl
                 << "    -6, --qt6                      - set Qt version to 6" << endl
                 /*<< "    -g=val, --guard-prefix=val     - prefix for guard macro, e.g. 'std'" << endl */
                 /*<< "    -i=val, --include-prefix=val   - include prefix (path)" << endl */
                 << "    -c, --clean                    - clean generated files" << endl
                 << "    -u, --user-include[s]          - user quotes instead of <>" << endl
                 << "    -d=VAL, --define=VAL           - define condition (for conditional break)" << endl
                 << "    -W=warn-opt, --warn=warn-      - turn warnings on/off" << endl
                 << "       warn-opt:" << endl
                 << "           all        - turn all warnings on" << endl
                 << "           off        - turn all warnings off" << endl
                 << "           default    - turn all warnings to its default state" << endl
                 << "           multi      - turn 'multiple includes' warning on" << endl
                 << "           no-multi   - turn 'multiple includes' warning off" << endl
                 << "           macro      - turn 'macro detected' warning on" << endl
                 << "           no-macro   - turn 'macro detected' warning off" << endl

                 << "'=' sign can be ommited" << endl
                 << "" << endl
                 << "If namelist.txt file name is ommited, default file name 'namelist.txt' without path is used" << endl
                 ;

            return 0;
        }
        else if (*optIt=="-w" || *optIt=="--where")
        {
            cout << argv[0] << endl;
            return 1;
        }
        else if (*optIt=="-g" || *optIt=="--git-add")
        {
            ++optIt; // move to next option
            generateGitAdd = true;
        }
        else if (*optIt=="-5" || *optIt=="--qt5")
        {
            ++optIt; // move to next option
            qtVersion = 5;
        }
        else if (*optIt=="-6" || *optIt=="--qt6")
        {
            ++optIt; // move to next option
            qtVersion = 6;
        }
        /*
        else if (*optIt=="-g" || *optIt=="--guard-prefix")
        {
            ++optIt; // move to arg
            if ( optIt==opts.end() )
            {
                cout << "-g/--guard-prefix - missing argument" << endl;
                return 1;
            }

            nameSpace = *optIt;
            ++optIt; // move to next option
        }
        */
        /*
        else if (*optIt=="-i" || *optIt=="--include-prefix")
        {
            ++optIt; // move to arg
            if ( optIt==opts.end() )
            {
                cout << "-i/--include-prefix - missing argument" << endl;
                return 1;
            }

            incPathPrefix = *optIt;
            ++optIt; // move to next option
        }
        */
        else if (*optIt=="-d" || *optIt=="--define")
        {
            ++optIt; // move to arg
            if ( optIt==opts.end() )
            {
                cout << "-d/--define - missing argument" << endl;
                return 1;
            }

            defines.insert(*optIt);
            ++optIt; // move to next option
        }
        else if (*optIt=="-W" || *optIt=="--warn")
        {
            ++optIt; // move to arg
            if ( optIt==opts.end() )
            {
                cout << "-W/--warn - missing argument" << endl;
                return 1;
            }

            if (*optIt=="all")
               warningFlags = WarningFlags::wf_all();
            else if (*optIt=="off")
               warningFlags = WarningFlags::wf_none();
            else if (*optIt=="default")
               warningFlags = WarningFlags::wf_default();
            else if (*optIt=="multi")
               warningFlags.warnMultipleIncludes = true;
            else if (*optIt=="no-multi")
               warningFlags.warnMultipleIncludes = false;
            else if (*optIt=="macro")
               warningFlags.warnUppercase        = true;
            else if (*optIt=="no-macro")
               warningFlags.warnUppercase        = false;
            else
            {
                cout << "Unknown -W/--warn option value: " << *optIt << endl;
                return 1;
            }

            ++optIt; // move to next option

        }
        else if (*optIt=="-u" || *optIt=="--user-include" || *optIt=="--user-includes")
        {
            ++optIt; // move to next option
            includeModeUser = true;
        }
        else if (*optIt=="-c" || *optIt=="--clean")
        {
            ++optIt; // move to next option
            cleanMode = true;
        }
        /*
        else if (opt=="" || opt=="")
        {
            ++optIt;
            if ( optIt==opts.end() )
            {
                cout << "Missing argument" << endl;
                return 1;
            }
        }
        */
        else 
        {
            namelistName = correctPathSeparators(*optIt);
            ++optIt; // move to next option
        }
    }

    //incPathPrefix = appendPathSep(incPathPrefix);

    std::vector< std::string >                     namesOrder;
    std::set<std::string>                          usedNames;
    std::map< std::string, std::set<std::string> > names; // map names to its includes



    if (!readNamelists( namelistName, namesOrder, usedNames, names, inputFileOptions, qtModules, qtModulePaths, defines, warningFlags ))
        return 2;


    namespaceGuardName = makeGuardFromNamespace(inputFileOptions.namespaceName);


    unsigned totalFilesGenerated = 0;


    std::ostringstream gitAdd;


    std::vector< std::string >::const_iterator nameIt = namesOrder.begin();

    for(; nameIt!=namesOrder.end(); ++nameIt)
    {
        std::string typeName = *nameIt;

        if (typeName.empty())
            continue;


        std::set<std::string> includesSet;

        std::map< std::string, std::set<std::string> >::const_iterator nit = names.find(typeName);
        if (nit==names.end() || nit->second.empty())
        {
            cout << "Name '" << typeName << "' found in order list, but not found in set" << endl;
        }
        else
        {
            includesSet  = nit->second;
        }


        if (warningFlags.warnUppercase && isFullUppercase(typeName))
        {
            cout<<"Warning: possible macro: '" << typeName << "'" << endl;
            std::set<std::string>::const_iterator itInc = includesSet.begin();
            for(; itInc!=includesSet.end(); ++itInc)
            {
                cout << "    in: " << *itInc << endl;
            }
        }
        


        if (includesSet.size()>1 && !cleanMode && warningFlags.warnMultipleIncludes)
        {
            cout << "Warning: too many includes for: " << typeName << endl;
            for( std::set<std::string>::const_iterator it=includesSet.begin(); it!=includesSet.end(); ++it )
                cout << "    " << *it << endl;
        }


        std::string incName;

        if (!includesSet.empty())
            incName = *includesSet.begin();

        std::map< std::string, std::string >::const_iterator qtModIt = qtModules.find( typeName );

        std::string guardIncName = incName;

        if ( qtModIt!=qtModules.end() )
            guardIncName = makeGuardFromNamespace( appendPath( qtModIt->second, incName ) );


        std::string guardString = makeGuard( namespaceGuardName, typeName, guardIncName );


        if (cleanMode)
        {
            std::remove(typeName.c_str());
            ++totalFilesGenerated;
            continue;
        }


        gitAdd << "git add " << typeName << endl;

        std::ofstream ofs( typeName.c_str() ); //  ( nit->first.c_str());

        if (!ofs)
        {
            cerr << "Failed to open output file '" << typeName << "'" << endl;
            return 3;
        }


        ofs << "#pragma once" << endl << endl;

        ofs << "#if !defined(" << guardString << ")" << endl << endl;
        ofs << "    #define " << guardString << endl << endl;

        // https://en.cppreference.com/w/cpp/string/basic_string
        // https://en.cppreference.com/w/cpp/header/any


        if (inputFileOptions.namespaceName=="std")
        {
            if (!incName.empty())
            {
                //ofs << endl;
                ofs << "    " << "// " << "https://en.cppreference.com/w/cpp/" << incName << "/" << typeName << endl;
                ofs << endl;
            }
        }

        else if ( qtModIt!=qtModules.end() )
        {
            ofs << "    " << "// " << "https://doc.qt.io/qt-5/" << makeLowerString(typeName) << ".html" << endl;
            ofs << endl;
        }


        //ofs << endl;
        //cout << endl;

        // std::map< std::string, std::string >::const_iterator qtModIt = qtModules.find( typeName );

        if ( qtModIt!=qtModules.end() )
        {

            // Qt

            const std::string &qtModule = qtModIt->second;

            std::set<std::string>::const_iterator itInc = includesSet.begin();
            for(; itInc!=includesSet.end(); ++itInc)
            {

                std::string qtModulePath = qtModule;

                std::map< std::string, std::string >::const_iterator qmpIt = qtModulePaths.find(qtModule);

                if (qmpIt!=qtModulePaths.end())
                {
                    qtModulePath = unquote(qmpIt->second, '\"');
                }

                std::string incName = appendPath( qtModulePath, *itInc ); // qtModulePath.empty() ? *itInc : appendPath( qtModulePath, *itInc );

                incName = quoteInclude( incName, includeModeUser );

                ofs << "    " << "#include " << incName << endl;
                ofs << endl;
            }
           
            //ofs << endl;
        }
        else
        {

            // std

            std::set<std::string>::const_iterator itInc = includesSet.begin();
            for(; itInc!=includesSet.end(); ++itInc)
            {

                if (inputFileOptions.namespaceName=="std")
                {
                    ofs << "    " << "// " << "https://en.cppreference.com/w/cpp/header/" << *itInc << endl;
                }
           
                std::string incName = *itInc;
                if (!isIncludeQuoted(*itInc))
                    incName = quoteInclude( *itInc, includeModeUser );

                ofs << "    " << "#include " << incName << endl;
                ofs << endl;
           
            }
           
            ofs << endl;
        
        }

        if ( qtModIt!=qtModules.end() )
        {
            std::string qtModule = qtModIt->second;
            std::string qtModuleUpperCase = toUpper(qtModule);

            if (qtModule.size()>=3)
            {
                if ( qtModuleUpperCase[0]=='Q' && qtModuleUpperCase[1]=='T' && (qtModuleUpperCase[2]!='5' && qtModuleUpperCase[2]!='6') )
                {
                    qtModule.insert(2,1,'0' + qtVersion); // ( size_type index, size_type count, CharT ch );
                }
            }

            ofs << "    #if defined(_MSC_VER)" << endl << endl;

            ofs << "        #if defined(DEBUG) || defined(_DEBUG)" << endl << endl
                << "            #pragma comment(lib, \"" << qtModule << "d\")" << endl << endl
                << "        #else" << endl << endl
                << "            #pragma comment(lib, \"" << qtModule << "\")" << endl << endl
                << "        #endif" << endl << endl
                ;

            ofs << "    #endif /* _MSC_VER */" << endl;
            ofs << endl;
        }



        ofs << "#endif /* " << guardString << " */" << endl << endl;

        ++totalFilesGenerated;

    }

    if (cleanMode)
    {
        cout << "Total files removed: " << totalFilesGenerated << endl;
    }
    else
    {
        cout << "Total files generated: " << totalFilesGenerated << endl;

        if (generateGitAdd)
        {
            std::ofstream gitAddFile( "git-add.bat" );

            gitAddFile << gitAdd.str() << endl;
        }
    }


    return 0;
}



