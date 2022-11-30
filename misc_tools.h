#ifndef MISC_TOOLS_H
#define MISC_TOOLS_H

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <vector>
#include <set>
#include <stack>
#include <map>
#include <exception>

namespace opt {

typedef std::vector<char> ByteArray;
typedef std::vector<char>::iterator BaIterator;
typedef std::vector<ByteArray> BaList;
typedef std::vector<std::string> StringList;

inline void initba(ByteArray &ba, char* d, size_t sz  )
{
    ba.assign(d,d+sz);
}

inline void initba(ByteArray &ba, std::string s )
{
    size_t sz=s.size();
    ba.assign(s.c_str(),s.c_str()+sz);
}

inline void extendByteArray(ByteArray &ba, ByteArray &ba2, int off  )
{
    ba.insert(ba.end(),ba2.begin()+off,ba2.end());
}

inline void extendByteArray(ByteArray &ba, ByteArray &ba2  )
{
    ba.insert(ba.end(),ba2.begin(),ba2.end());
}

inline void prependByteArray(ByteArray &ba, ByteArray &ba2  )
{
    ba.insert(ba.begin(),ba2.begin(),ba2.end());
}

inline void extendByteArray(ByteArray &ba, char *d, size_t t  )
{
    ba.insert(ba.end(),d,d+t);
}

template <typename T> inline void extendByteArray(ByteArray &ba, T d)
{
    ba.insert(ba.end(),(char*)&d,(char*)&d+sizeof(T));
}

inline ByteArray byteArrayLeft(ByteArray ba, unsigned int n)
{
    if(ba.size() > n)
    {
        return ByteArray(ba.begin(), ba.begin()+n);
    }
    else
        return ba;
}

inline ByteArray byteArrayRight(ByteArray ba, unsigned int index)
{
    if(ba.size() > index)
    {
        return ByteArray(ba.begin()+index, ba.end());
    }
    else
        return ba;
}

inline ByteArray byteArrayMid(ByteArray ba, unsigned int i, unsigned int n)
{
    if( ba.size() > (i+n) )
    {
        return ByteArray(ba.begin()+i, ba.begin()+i+n);
    }
    else
        return ba;
}

template <typename T> inline ByteArray numToByteArray(T d)
{
    ByteArray ba;
    ba.assign((char*)&d, (char*)&d+sizeof(T));
    return ba;
}

template <typename T> inline void byteArrayToNum(ByteArray &ba, T &k)
{
    k=*((T*)ba.data());
}

inline std::string byteArrayToString(ByteArray &ba, uint offs, uint sz)
{
    if(ba.size() >= (offs+sz))
    {
        return std::string(ba.data()+offs,sz);
    }
    else
        return "";
}

inline std::string byteArrayToString(ByteArray &ba)
{
    if(ba.size() > 0)
        return std::string(ba.data(),ba.size());
    else
        return "";
}


inline void stringToBayteArray(std::string s, ByteArray &ba)
{
    std::copy(s.begin(),s.end(),std::back_inserter(ba));
}

inline std::string right(std::string s, int pos=0)
{
    return s.substr(pos);
}


inline std::string right(std::string s, std::string ref)
{
    size_t p=s.find(ref);
    if(p==std::string::npos)
        return "";
    else
        return s.substr(p+ref.size());
}


inline void printhex(ByteArray ba)
{
    for(uint i=0; i<ba.size(); i++)
    {
        printf("%02x", (unsigned char)ba[i]);
    }
    printf("\n");
    fflush(stdout);
}

inline void printhex(const char *d, uint sz)
{
    for(uint i=0; i<sz; i++)
    {
        printf("%02x", d[i]);
    }
    printf("\n");
    fflush(stdout);
}

inline void split(std::string is, char del, std::vector<std::string> &slist)
{
    std::istringstream iss(is);
    std::string s;
    while (std::getline(iss, s, del))
    {
        if(s.size()!=0)
        {
            if(s.back()=='\r')
            {
                s.pop_back();
            }
            slist.push_back(s);
        }
    }
}

inline void split(std::string s, std::string d, std::vector<std::string> &slist, bool noDelimiter=true)
{
    uint pos=0;
    while(pos <= s.length() )
    {
        size_t fpos=s.find(d,pos);
        if(fpos==std::string::npos)
            fpos=s.length();
        std::string ts;
        if(noDelimiter || (!noDelimiter && pos < d.size()))
        {
            ts=s.substr(pos,fpos-pos);
        }
        else
        {
            ts=s.substr(pos-d.length(),fpos-pos);
        }
        pos=fpos+d.length();
        if(!ts.empty())
            slist.push_back(ts);
    }
}

inline std::string getRight(std::string s, int pos=0)
{
    return s.substr(pos);
}

inline std::string getRight(std::string s, std::string ref)
{
    size_t p=s.find(ref);
    if(p==std::string::npos)
        return "";
    else
        return s.substr(p+ref.size());
}

inline bool startsWith(std::string s, std::string ref)
{
    bool result = s.substr(0,ref.size()) == ref;
    return result;
}

class Thread
{
public:

    virtual ~Thread() = default;

    void start(bool _loop=false)
    {
        if(_loop)
            thread=std::thread(&Thread::run_loop,this);
        else
            thread=std::thread(&Thread::run_no_loop,this);
        running.store(true);
    }
    void stop()
    {
        running.store(false);

    }
    void wait()
    {
        thread.join();
    }
    bool isrunning()
    {
        return running;
    }

protected:
    std::atomic<bool> running;
    std::mutex mutex;

    virtual void run()=0;
    virtual void onstart(){}
    virtual void onstop(){}


private:
    std::thread thread;
    void run_no_loop()
    {
        run();
        running.store(false);
    }
    void run_loop()
    {
        onstart();
        while (running.load())
        {
            run();
        }
        onstop();
    }
};


}


#endif // MISC_TOOLS_H
