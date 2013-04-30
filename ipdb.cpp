// **********************************************************************************
//
// BSD License.
// This file is part of locations.
//
// Copyright (c) 2013, Bruno Keymolen, email: bruno.keymolen@gmail.com
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or other
// materials provided with the distribution.
// Neither the name of "Bruno Keymolen" nor the names of its contributors may be
// used to endorse or promote products derived from this software without specific
// prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// **********************************************************************************

#include "ipdb.h"
#include <stdlib.h>

#include <unistd.h>
#include <stdio.h>
#include <fstream>

#include <cstring>

namespace keymolen {

	IPDB::IPDB() {

	}

	IPDB::~IPDB() {
		for(std::map<unsigned int, IPDBRecord*>::iterator it=_db.begin();it!=_db.end();it++)
		{
			delete(it->second);
		}
	}

	//"16777216","16777471","AU","AUSTRALIA","QUEENSLAND","SOUTH BRISBANE","-27.483330","153.016670","-","+10:00"
	void IPDB::load(std::string path)
	{
		std::ifstream db_file;
		db_file.open(path.c_str(), std::ios::in);

		std::string line;
		while (!db_file.eof())
		{
			std::getline(db_file, line);

			//Process the line
			IPDBRecord* r = new IPDBRecord();
			r->lon = atol(parseString(line.c_str(), 7).c_str());
			r->lat = atol(parseString(line.c_str(), 8).c_str());
			_db[atoi(parseString(line.c_str(), 1).c_str())] = r;
		}
		db_file.close();
	}

	IPDB::IPDBRecord* IPDB::search(std::string ipstring)
	{
		unsigned int  ip = ipstring2int(ipstring);

		std::map<unsigned int, IPDBRecord*>::iterator it = _db.lower_bound(ip);
		if(it == _db.end())
		{
			return 0;
		}
		if(it->first != ip && it != _db.begin())
			it--;
		return it->second;
	}


	std::string IPDB::parseString(const char* buf, int nr)
	{
		char tmp[256];

		if(nr > 9)
			return "";

		char* b = (char*)buf;
		char* e = 0;

		nr--;
		while(nr-- > 0)
		{
			b = strchr(b+1, ',');
			if(b==0)
				return "";
		}

		b = strchr(b, '"');
		if(b==0)return "";
		e = strchr(b+1, '"');
		if(b==0)return "";


		memcpy(tmp, b+1, e-b-1);
		tmp[e-b-1] = 0;

		return tmp;
	}

	unsigned int IPDB::ipstring2int(std::string ip)
	{
		unsigned int r = 0;
		char *c = (char*)&r;

		char p1[50];
		strncpy(p1, ip.c_str(), 49);
		if(p1==0)return 0;
		char *p2 = strchr(p1+1, '.');
		if(p2==0)return 0;
		*p2 = 0;
		p2++;
		char *p3 = strchr(p2+1, '.');
		if(p3==0)return 0;
		*p3 = 0;
		p3++;
		char *p4 = strchr(p3+1, '.');
		if(p4==0)return 0;
		*p4 = 0;
		p4++;
		//allow a .port format like on mac
		char *p5 = strchr(p4+1, '.');
		if(p5!=0)
			*p5=0;

		//std::cout << p1 << " " << p2 << " " << p3 << " " << p4 << std::endl;

		c[3] = atoi(p1);
		c[2] = atoi(p2);
		c[1] = atoi(p3);
		c[0] = atoi(p4);

		return r;
	}

}
