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

#include <unistd.h>
#include <iostream>
#include <typeinfo>
#include <assert.h>
#include <zlib.h>

#include "ziplib.h"


ZipFile::ZipFile(const char *name) 
	: _input(*new ifstream(name, ios::in | ios::binary)), 
	_name(name), 
	_zlist(*new list<ZipFileEntry *>)
{
	parse();
}

ZipFile::ZipFile(ifstream& is)
	: _input(is), 
	_name(string("<input stream>")),
	_zlist(*new list<ZipFileEntry *>)
{
	parse();
}

void
ZipFile::findSig(ifstream &is, int sig, int dir, int limit)
{
	int i;
	for (i = 0; i < limit; i++) {
		int s;
		is.read((char*)&s, sizeof(s));

#ifdef DEEPDEBUG
		cerr << "seekg: " << is.tellg() << " sig: " << s << endl;
#endif

		if (s == sig)
			break;

		is.seekg(dir, ios::cur);
	}

	if (i == limit)
		throw InvalidSignature();
}

void
ZipFile::parse()
{
	// First parse the end of central directory header
	_input.seekg(ECDS_LEN, ios::end);

	findSig(_input, ECDS_SIGNATURE, -1, SIG_LIMIT);

	// read the ECDS
	short nentries;
	int  cd_offset, cd_size;

	_input.seekg(6, ios::cur);
	_input.read((char *)&nentries, sizeof(nentries));
	_input.read((char *)&cd_size, sizeof(cd_size));

#ifdef DEEPDEBUG
	cerr << "cd off seekg: " << _input.tellg() << endl;
#endif

	_input.read((char *)&cd_offset, sizeof(cd_offset));

#ifdef DEEPDEBUG
	cerr << "CD offset: " << cd_offset << endl;
#endif

	_input.seekg(cd_offset, ios::beg);

	// Read the central directory
	for (int i=0; i < nentries; i++) {
		CentralDirRec rec;
		_input.read((char *)&rec, sizeof(rec));

		int sig = rec.sig;

#ifdef DEEPDEBUG
		cerr << "CD sig: " << sig << endl;
#endif

		if (sig != CD_SIGNATURE)
			throw InvalidSignature();


		// Read filename
		char *fname = new char[rec.fnlen];
		_input.read(fname, rec.fnlen);
		char *ef = new char[rec.eflen];
		_input.read(ef, rec.eflen);
		char *fc = new char[rec.fclen];
		_input.read(fc, rec.fclen);
		string name = string(fname, 0, rec.fnlen);

		delete fname;
		delete ef;
		delete fc;

#ifdef DEEPDEBUG
		cerr << "name: " << name << endl;
		cerr << "len: " << rec.fnlen<< endl;
		cerr << "meth: " << rec.compression_meth << endl;
#endif
		ZipFileEntry *ze = new ZipFileEntry(name, _input, rec.lhoff,
											rec.csize);
  		_zlist.push_back(ze);
	}
}

list <ZipFileEntry *> 
ZipFile::entries() 
{
  	// Not implemented
  	return _zlist;
}

list <ZipFileEntry *>::const_iterator
ZipFile::iterator() 
{
  	return _zlist.begin();
}

void
ZipFile::close() 
{
  	ifstream *ifs = dynamic_cast<ifstream *> (&_input);
  	if (ifs != NULL)
  		ifs->close();
}

string&
ZipFile::getName() 
{
  	return _name;
}

ZipFile::~ZipFile()
{
	list<ZipFileEntry *>::iterator it;

	for (it = _zlist.begin(); it != _zlist.end(); it++) {
		ZipFileEntry *ze = *it;
		delete ze;
	}

	delete &_input;
	delete &_zlist;
}

// Entry class
ZipFileEntry::ZipFileEntry(string& name, ifstream& is, int lhoff, int csize)
	: _is(is), _name(name), _lhoff(lhoff), _csize(csize)
{
	_initialized = false;
}

void
ZipFileEntry::parse()
{
  	// Step1: local file header
  	int sig;

	_is.seekg(_lhoff, ios::beg);

#ifdef DEEPDEBUG
  	cerr << "pos: " << _is.tellg() << endl;
#endif

  	_is.read((char *)&sig, 4);
  	if (sig != ZipFile::LFH_SIGNATURE)
  		throw InvalidSignature();
  	__int16_t version;
  	_is.read((char *)&version, 2);
#ifdef DEEPDEBUG
  	cerr << "version: " << version << endl;
#endif
  	__int16_t flag, lastDate, lastTime;
  	_is.read((char *)&flag, 2);
  	_is.read((char *)&_method, 2);
  	_is.read((char *)&lastDate, 2);
  	_is.read((char *)&lastTime, 2);

#ifdef DEEPDEBUG
  	cerr << "flag: " << flag << endl;
  	cerr << "compression: " << _method << endl;
#endif

  	__int32_t crc, size, csize;
  	_is.read((char *)&crc, 4);
  	_is.read((char *)&size, 4);
  	_is.read((char *)&csize, 4);


#ifdef DEEPDEBUG
  	cerr << "crc: " << crc << endl;
  	cerr << "size: " << size << endl;
  	cerr << "csize: " << csize << endl;
#endif

  	__int16_t flen, elen;
  	_is.read((char *)&flen, 2);
  	_is.read((char *)&elen, 2);

#ifdef DEEPDEBUG
  	cerr << "flen: " << flen << endl;
  	cerr << "elen: " << elen << endl;
#endif

  	char *fnamebuf = new char[flen];
  	char *ebuf = new char[elen];

  	_is.read((char *)fnamebuf, flen);
  	_is.read((char *)ebuf, elen);

  	_name = string(fnamebuf, 0, flen);

	delete fnamebuf;
	delete ebuf;

#ifdef DEEPDEBUG
  	cerr << "name: " << _name << endl;
#endif

  	// Step 2: read data
	if (csize == 0)
		csize = _csize; // from central dir
  	char *fbuf = new char[csize];
  	_is.read((char *)fbuf, csize);
	_cdata = fbuf;

  	// Step 3: read data descriptor if any
  	if (flag & HAS_DATA_DESCRIPTOR) {

#ifdef DEEPDEBUG
  		cerr << "has dd" << endl;
#endif
  		_is.read((char *)&sig, 4);
		if (sig != ZipFile::ELFH_SIGNATURE)
			throw InvalidSignature();

		_is.read((char *)&crc, 4);
		_is.read((char *)&csize, 4);
		_is.read((char *)&size, 4);

#ifdef DEEPDEBUG
		cerr << "crc: " << crc << endl;
		cerr << "size: " << size << endl;
		cerr << "csize: " << csize << endl;
#endif

	}

	// beginning of data
	_off = _is.tellg();
	_csize = csize;
	_size = size;

	_initialized = true;
}

int
ZipFileEntry::doInflate(char *data)
{
	int err;
	z_stream d_stream; /* decompression stream */

	memset(&d_stream, 0, sizeof(d_stream));
	d_stream.next_in  = (Bytef*) _cdata;
	d_stream.avail_in = _csize;
	d_stream.next_out = (Bytef*) data;
	d_stream.avail_out = _size;

	err = inflateInit2(&d_stream, -MAX_WBITS);
	assert(err == Z_OK);

	while (d_stream.total_in < _csize) {
		err = inflate(&d_stream, Z_SYNC_FLUSH);
		if (err == Z_STREAM_END) 
			break;
		assert(err == Z_OK);
	}

	err = inflateEnd(&d_stream);
	assert(err == Z_OK);
	return err;
}

char *
ZipFileEntry::getData()
{
	if (!_initialized)
		parse();

	// Commonly used method
	if (_method == METHOD_UNCOMPRESSED)
		return _cdata;

	if (_method == METHOD_DEFLATED) {
		_data = new char[_size];
	
		int ret = doInflate(_data);

#ifdef DEEPDEBUG
		cerr << "uncompress returned: " << ret << endl;
		cerr << "size: " << _size << endl;
#endif
		assert(ret == Z_OK);
		delete _cdata;

		return _data;
	}
		
	throw UnknownCompressionMethod();
}

ZipFileEntry::~ZipFileEntry()
{
	if (_initialized)
		delete _data;
}

ZipFileEntry *
JarFile::getManifest()
{
    return NULL;
}
