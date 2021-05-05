#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <cstdio>

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
std::string appendPath( const std::string &p, const std::string &n )
{
    return appendPathSep(p) + n;
}

//----------------------------------------------------------------------------
inline
bool readNamelists( const std::string                              &namelistName
                  , std::vector< std::string >                     &namesOrder
                  , std::set<std::string>                          &usedNames
                  , std::map< std::string, std::set<std::string> > &names
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
            iss >> name;

            if (kwd=="#break")
                return true;

            if (kwd!="#include")
                continue;

            if (name.empty())
            {
                cerr << "Empty include statement" << endl;
                return false;
            }

            std::string curPath = getPath( namelistName );

            std::string includedFullName = appendPath( curPath, name );

            if ( !readNamelists( includedFullName, namesOrder, usedNames, names ))
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


int main( int argc, char *argv[])
{
    using std::cout;
    using std::cerr;
    using std::endl;

    std::string nameSpace;
    std::string incPathPrefix;
    std::string namelistName = "namelist.txt";
    bool        includeModeUser = false;
    bool        cleanMode       = false;

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
        const std::string &opt = *optIt;
        if (opt=="-h" || opt=="--help")
        {
            cout << "Usage: umba-make-headers [options] [namelist.txt]" << endl
                 << "where options are: " << endl
                 << "    -h, --help                     - print this help and exit" << endl
                 << "    -w, --where                    - print full path to self executable" << endl
                 << "    -g=val, --guard-prefix=val     - prefix for guard macro, e.g. 'std'" << endl
                 << "    -i=val, --include-prefix=val   - include prefix (path)" << endl
                 << "    -c, --clean                    - clean generated files" << endl
                 << "    -u, --user-include[s]          - user quotes instead of <>" << endl
                 << "'=' sign can be ommited" << endl
                 << "" << endl
                 << "If namelist.txt file name is ommited, default file name 'namelist.txt' without path is used" << endl
                 ;

            return 1;
        }
        else if (opt=="-w" || opt=="--where")
        {
            cout << argv[0] << endl;
            return 1;
        }
        else if (opt=="-g" || opt=="--guard-prefix")
        {
            ++optIt;
            if ( optIt==opts.end() )
            {
                cout << "-g/--guard-prefix - missing argument" << endl;
                return 1;
            }

            nameSpace = *optIt;
            ++optIt;
        }
        else if (opt=="-i" || opt=="--include-prefix")
        {
            ++optIt;
            if ( optIt==opts.end() )
            {
                cout << "-i/--include-prefix - missing argument" << endl;
                return 1;
            }

            incPathPrefix = *optIt;
            ++optIt;
        }
        else if (opt=="-u" || opt=="--user-include" || opt=="--user-includes")
        {
            ++optIt; // -u, --user-include[s]
            includeModeUser = true;
        }
        else if (opt=="-c" || opt=="--clean")
        {
            ++optIt; // -u, --user-include[s]
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
            namelistName = correctPathSeparators(opt);
            ++optIt;
        }
    }

    incPathPrefix = appendPathSep(incPathPrefix);

    std::vector< std::string >                     namesOrder;
    std::set<std::string>                          usedNames;
    std::map< std::string, std::set<std::string> > names; // map names to its includes

    // cout << "Namespace: " << nameSpace << endl << endl;

    //<<<
    #if 0
    std::ifstream istrteamNamelist(namelistName.c_str());

    if (!istrteamNamelist)
    {
        cerr << "Failed to open input namelist file '" << namelistName << "'" << endl;
        return 2;
    }

    std::string typeInfoStr;


    while( std::getline(istrteamNamelist, typeInfoStr ) )
    {

        if (typeInfoStr.empty())
            continue;

        if (isEmpty(typeInfoStr))
            continue;

        if (typeInfoStr[0]=='#' || typeInfoStr[0]==';')
            continue;

        std::istringstream iss(typeInfoStr);

        std::string typeName;
        iss >> typeName;

        if (usedNames.find(typeName)!=usedNames.end())
            continue;

        usedNames.insert(typeName);
        namesOrder.push_back(typeName);

        // !!! namesOrder

        std::string tmpName;

        while(iss>>tmpName)
        {
            if (tmpName.empty())
                continue;

            if (tmpName[0]=='#' || tmpName[0]==';')
                break;

            names[typeName].insert(tmpName);
        }
    }

    #endif
    //>>>

    if (!readNamelists( namelistName, namesOrder, usedNames, names ))
        return 2;


    unsigned totalFilesGenerated = 0;

    //std::set<std::string> lastIncludes;

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
            //includesSet = lastIncludes;
            cout << "Name '" << typeName << "' found in order list, but not found in set" << endl;
        }
        else
        {
            includesSet  = nit->second;
            //lastIncludes = includesSet;
        }

        // !!!

        if (includesSet.size()>1 && !cleanMode)
        {
            cout << "Too many includes for: " << typeName << endl;
            for( std::set<std::string>::const_iterator it=includesSet.begin(); it!=includesSet.end(); ++it )
                cout << "    " << *it << endl;
        }


        std::string incName;

        if (!includesSet.empty())
            incName = *includesSet.begin();

        std::string guardString = makeGuard( nameSpace, typeName, incName );

        char openQuot  = '<';
        char closeQuot = '>';

        if (includeModeUser)
        {
            openQuot  = '\"';
            closeQuot = '\"';
        }

        if (cleanMode)
        {
            std::remove(typeName.c_str());
            ++totalFilesGenerated;
            continue;
        }


        std::ofstream ofs( typeName.c_str() ); //  ( nit->first.c_str());

        if (!ofs)
        {
            cerr << "Failed to open output file '" << typeName << "'" << endl;
            return 3;
        }


        ofs << "#pragma once" << endl << endl;

        ofs << "#if !defined(" << guardString << ")" << endl << endl;
        ofs << "    #define " << guardString << endl << endl;

        if (nameSpace=="std" && !incName.empty())
        {
            ofs << endl;
            ofs << "    " << "// " << "https://en.cppreference.com/w/cpp/" << incName << "/" << typeName << endl << endl;
            ofs << endl;
        }

        // https://en.cppreference.com/w/cpp/string/basic_string
        // https://en.cppreference.com/w/cpp/header/any

        std::set<std::string>::const_iterator itInc = includesSet.begin();
        for(; itInc!=includesSet.end(); ++itInc)
        {
            if (nameSpace=="std")
            {
                ofs << "    " << "// " << "https://en.cppreference.com/w/cpp/header/" << *itInc << endl;
            }

            ofs << "    " << "#include " << openQuot << incPathPrefix << *itInc << closeQuot << endl;
            ofs << endl;
        }

        ofs << endl;
        //cout << endl;
         
        ofs << "#endif /* " << guardString << " */" << endl << endl;

        ++totalFilesGenerated;

    }

    if (cleanMode)
        cout << "Total files removed: " << totalFilesGenerated << endl;
    else
        cout << "Total files generated: " << totalFilesGenerated << endl;


    #if 0

    std::map< std::string, std::set<std::string> >::const_iterator nit = names.begin();
    for(; nit != names.end(); ++nit)
    {

        std::string typeName = nit->first; // *nit->second.begin();

        const std::set<std::string> &incSet = nit->second;

        std::string incName; // = *nit->second.begin();
        std::ostringstream ossGuard;

        ossGuard << "GENERATED_";

        if (!nameSpace.empty())
            ossGuard << nameSpace << "_";

        ossGuard << typeName;

        if (!nit->second.empty())
        {
            incName = *nit->second.begin();
            ossGuard << "_from_" << incName;
        }

        ossGuard << "_INCLUDE_GUARD";

        std::string guard = ossGuard.str();

        char openQuot  = '<';
        char closeQuot = '>';

        if (includeModeUser)
        {
            openQuot  = '\"';
            closeQuot = '\"';
        }


        std::ofstream ofs( typeName.c_str() ); //  ( nit->first.c_str());

        ofs << "#pragma once" << endl << endl;

        ofs << "#if !defined(" << guard << ")" << endl << endl;
        ofs << "    #define " << guard << endl << endl;

        if (nameSpace=="std" && !incName.empty())
        {
            ofs << "    " << "// " << "https://en.cppreference.com/w/cpp/" << incName << "/" << typeName << endl << endl;
        }

        // https://en.cppreference.com/w/cpp/string/basic_string
        // https://en.cppreference.com/w/cpp/header/any

        std::set<std::string>::const_iterator itInc = incSet.begin();
        for(; itInc!=incSet.end(); ++itInc)
        {
            if (nameSpace=="std")
            {
                ofs << "    " << "// " << "https://en.cppreference.com/w/cpp/header/" << *itInc << endl;
            }

            ofs << "    " << "#include " << openQuot << incPathPrefix << *itInc << closeQuot << endl;
        }

        ofs << endl;
        //cout << endl;
         
        ofs << "#endif /* " << guard << " */" << endl << endl;

    }

    #endif

    return 0;
}



