/*
 * Copyright 2001 Arun Sharma <arun@sharma-home.net>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    1.Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    2.Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY ARUN SHARMA ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ARUN SHARMA OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _ZIPLIB_H_
#define _ZIPLIB_H_

#include <sys/types.h>
#include <fstream>
#include <list>
#include <string.h>

using namespace std ;

class ZipFileEntry;
/* Format documented at:
 * http://www.goice.co.jp/member/mo/formats/zip.html
 */
   
class ZipFile {
 public:
    static const __int32_t LFH_SIGNATURE = 0x04034b50;
    static const __int32_t ELFH_SIGNATURE = 0x08074b50;
    static const __int32_t CD_SIGNATURE = 0x02014b50;
    static const __int32_t ECDS_SIGNATURE = 0x06054b50;

    static const __int32_t ECDS_LEN = -22;
    static const __int32_t SIG_LIMIT= 64 * 1024;

 protected:
    string _name;
    ifstream& _input;
    list <ZipFileEntry *>& _zlist;

 private:
    void parse();
    void findSig(ifstream &_input, int sig, int dir, int limit);

 public:
    ZipFile(const char *);
    ZipFile(ifstream& is);
    virtual ~ZipFile();
    string& getName();
    // The iterator method is preferred for performance reasons
    list <ZipFileEntry *> entries();
    list <ZipFileEntry *>::const_iterator iterator();
    void close();
};

// Inner class
class ZipFileEntry {
 private:
    static const unsigned short HAS_DATA_DESCRIPTOR = 0x8;
    static const unsigned int METHOD_UNCOMPRESSED = 0;
    static const unsigned int METHOD_DEFLATED = 8;

    void parse();
    int doInflate(char *dest);
    
 protected:
    string _name;
    string _crc;
    string _extra;
    short _method;
    string _comment;
    size_t _size;
    size_t _csize; 		// compressed size
    long _time;
    int _lhoff;			// local header off
    ifstream &_is;
    bool _initialized;
    off_t _off;
    char *_cdata;
    char *_data;

 public:
    ZipFileEntry(string& name, ifstream& input, int lhoff, int csize);
    ~ZipFileEntry();

    const string& getComment() const { return _comment;};
    const string& getCrc() const { return _crc; };
    const string& getExtra() const { return _extra; };
    const short getMethod() const { return _method;};
    const string& getName() const { return _name;};
    size_t getSize() const { return _size; };
    long getTime() const { return _time; };
    bool isDirectory() const { return _name.find_last_of("/") == _name.length()-1;};

    // Get the uncompressed data for this file
    char *getData();
};

class JarFile : public ZipFile {
 public:
    JarFile(const char *c) : ZipFile(c) {};
    JarFile(ifstream& is) : ZipFile(is) {};
    ZipFileEntry *getManifest(); // unimplemented
};

class ZipFileException {
};

class InvalidSignature : public ZipFileException {
};

class UnknownCompressionMethod : public ZipFileException {
};

// Private internal structure
struct CentralDirRec {
    int 		sig;
    short 		version_created;
    short		version_req;
    short		flag;
    short		compression_meth;
    short		mtime;
    short		mdate;
    int			crc;
    int			csize;
    int			ucsize;
    short		fnlen;
    short		eflen;
    short		fclen;
    short		disknum;
    short		ifattr;
    int			efattr;
    int			lhoff;
} __attribute__ ((packed));

#endif /* _ZIPLIB_H_ */
