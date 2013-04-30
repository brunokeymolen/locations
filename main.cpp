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
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#include <iostream>
#include <string.h>
#include <cmath>

#include <fcgiapp.h>
#include "ipdb.h"

std::string path = "/opt/keymolen/IP2LOCATION-LITE-DB11.CSV";
std::string ips = "";

void doLocate(std::vector<std::string>& ips);
int parseIps(std::vector<std::string>& ia, std::string ips);
void doCGI();
std::string makeHTML(keymolen::IPDB &db, std::vector<std::string> &ips);

void usage(char * s)
{

	fprintf( stderr, "\n");
    fprintf( stderr, "%s -d <path> -I <ip addresses>  - show current tcp/udp connection locations. build: %s-%s", s, __DATE__, __TIME__);
	fprintf( stderr, "   d: ip2location database (default: /opt/keymolen/IP2LOCATION-LITE-DB11.CSV)\n");
	fprintf( stderr, "   I: IP addresses to display, if absent the app will run as a CGI\n");
	fprintf( stderr, "\n");
}

int main(int argc, char** argv) {

	int c;
	while ( ((c = getopt( argc, argv, "d:I:?" ) ) ) != -1 )
	{
	    switch (c)
	    {
	    case 'd':
	    	path = optarg;
	    	break;
	    case 'I':
	    	ips = optarg;
	    	break;
	    case '?':
        default:
            	usage(argv[0]);
            	return -1;
	    }
	}

	if(path.empty())
	{
		usage(argv[0]);
		return -1;
	}

	if(ips.empty())
	{
		doCGI();
	}
	else
	{
		std::vector<std::string> vip;
		parseIps(vip, ips);
		doLocate(vip);
	}

	return 0;
}



void doLocate(std::vector<std::string>& ips)
{
	keymolen::IPDB db;
	db.load(path);

	for(std::vector<std::string>::iterator it=ips.begin();it!=ips.end();it++)
	{
		std::string ip = *it;
		keymolen::IPDB::IPDBRecord *f = db.search(ip);
		if(f)
		{
			std::cout << \
				 f->lon << std:: endl << \
				 f->lat << std:: endl << \
				 std:: endl;
		}
		else
		{
			std::cout << std:: endl << \
				 *it << std:: endl << \
				 "NOT FOUND" << std:: endl << \
				 std:: endl;
		}
	}

}


void doCGI() {
	keymolen::IPDB db;
	db.load(path);

	//Start the CGI loop
	FCGX_Init();
	while (true) {
		FCGX_Request* request = new FCGX_Request;
		FCGX_InitRequest(request, 0, 0);
		int rc = FCGX_Accept_r(request);

		if (rc < 0) {
			std::ostringstream os;
			os << "Content-type: text/html\n";
			os << "\n";
			os << "<b>Error! </b>";
			os << rc;
			os << "\n\n\n";
			std::string resp = os.str();
			FCGX_PutS(resp.c_str(), request->out);
			FCGX_FFlush(request->out);
			FCGX_Finish_r(request);
			delete request;
			usleep(100);
			continue;
		}

		const char* method = FCGX_GetParam("REQUEST_METHOD", request->envp);
		const char* path_info = FCGX_GetParam("PATH_INFO", request->envp);
		const char* cl = FCGX_GetParam("CONTENT_LENGTH", request->envp);
		const char* req_full = FCGX_GetParam("QUERY_STRING", request->envp);
		const char* remoteip = FCGX_GetParam("REMOTE_ADDR", request->envp);

		std::vector<std::string> ips;
		ips.push_back(remoteip);

		if (req_full) {
			const char *req = strstr(req_full, "ips=");
			if (req != 0) {
				req += 4;
				parseIps(ips, req);
			}
		}

		std::string resp = makeHTML(db, ips);
		FCGX_PutS(resp.c_str(), request->out);

		FCGX_FFlush(request->out);
		FCGX_Finish_r(request);
		delete request;
	}

}



int parseIps(std::vector<std::string>& ia, std::string ips)
{
	char* pos = (char*)ips.c_str();
	char ip[50];

	char* begin = pos;

	while(*pos != 0)
	{
		if(*pos == ',' || *(pos+1) == 0)
		{
			if(pos-begin-1 < 20 && pos-begin-1 > 0)
			{

				if(*(pos+1) == 0)
				{
					memcpy(ip, begin, (pos-begin)+1);
					ip[(pos-begin)+1] = 0;
				}
				else
				{
					memcpy(ip, begin, pos-begin);
					ip[pos-begin] = 0;
				}

				ia.push_back(ip);
			}
			begin = pos+1;
		}

		pos++;
	}

	return ia.size();
}

std::string makeHTML(keymolen::IPDB &db, std::vector<std::string> &ips)
{

	 std::ostringstream os;
     os << "Content-type: text/html\n";
     os << "\n";
     os << "<!DOCTYPE html>";
     os << "<html>";
     os << "<head>";
     os << "<meta name=\"viewport\" content=\"initial-scale=1.0, user-scalable=no\" />";
     os << "<style type=\"text/css\">";
     os << "html { height: 100% }";
     os << "body { height: 100%; margin: 0; padding: 0 }";
     os << "#map-canvas { height: 90% }";
     os << "</style>";
     os << "<script type=\"text/javascript\"";
     os << "src=\"https://maps.googleapis.com/maps/api/js?key=AIzaSyCg6zaOl9sCDvLFXYopL9LDiJVNiOiNJK8&sensor=true\">";
     os << "</script>";
     os << "<script type=\"text/javascript\">";

     std::vector<std::string>::iterator it=ips.begin();
     std::string f_home_ip = *it;
     keymolen::IPDB::IPDBRecord *f_home = db.search(f_home_ip);
     os << "var myhome;var infowindow;";

     os << "function initialize() {";
     if(f_home)
     {
    	 os << " myhome = new google.maps.LatLng( " << f_home->lon << ", " << f_home->lat << ");";
     }
     os << " var mapOptions = {";
     os << "  center: myhome,";
     os << "  zoom: 2,";
     os << "  mapTypeId: google.maps.MapTypeId.ROADMAP";
     os << " };";
     os << " var map = new google.maps.Map(document.getElementById(\"map-canvas\"),";
     os << " mapOptions);";

     os << "\n";

     int m = 0;
     while(++it!=ips.end())
	 {
		std::string ip = *it;
		keymolen::IPDB::IPDBRecord *f = db.search(ip);
		if(f)
		{

			 os << " current = new google.maps.LatLng( " << f->lon << ", " << f->lat << ");";
		     os << " var marker" << m <<  " = new google.maps.Marker({";
		     os << "  position: current,";
		     os << "  map: map,";
		     os << "  title: '" << ip << "'";
		     os << " });";

		     os << "\n";

		     os << "google.maps.event.addListener(marker" << m <<  ", \"click\", function() {";
		     os << " if (infowindow) infowindow.close();";
    		 os << " infowindow = new google.maps.InfoWindow({content: '" << ip << "'});";
		     os << " infowindow.open(map, marker" << m <<  ");";
		     os << "});";

		     os << "\n";


		     os << "var myTrip=[myhome,current];";
			 os << "var flightPath=new google.maps.Polyline({";
			 os << " path:myTrip,";
			 os << " strokeColor:\"#AA1010\",";
			 os << " strokeOpacity:0.3,";
			 os << " strokeWeight:1";
			 os << " });";
			 os << "flightPath.setMap(map);";

			 os << "\n";

			 m++;
		}
	 }

     if(f_home)
     {
    	 os << " var marker_home = new google.maps.Marker({";
    	 os << "  position: myhome,";
    	 os << "  map: map,";
    	 //os << "  icon: new google.maps.MarkerImage(\"http://maps.google.com/mapfiles/kml/pal3/icon48.png\"),";
    	 os << "  title: '" << f_home_ip << "'";
    	 os << "  });";

	     os << "google.maps.event.addListener(marker_home, \"click\", function() {";
	     os << " if (infowindow) infowindow.close();";
		 os << " infowindow = new google.maps.InfoWindow({content: 'home" << "<br>" << f_home_ip << "'});";
	     os << " infowindow.open(map, marker_home);";
	     os << "});";

     }

     os << "\n";

     os << "}";
     os << "google.maps.event.addDomListener(window, 'load', initialize);";
     os << "</script>";
     os << "</head>";

     os << "<body>";
     os << "<div id=\"map-canvas\"></div>";

     os << "<div id=\"text\"><font size=2><center><i>Author: Bruno Keymolen</i><br><br>";
     os << "\"The IP geolocation data is provided by http://www.ip2location.com\"<br>IP2Location is a registered trademark of Hexasoft Development Sdn Bhd. All other trademarks are the property of their respective owners.";
     os << "</center></font></div>";

     os << "<script type=\"text/javascript\">";
     os << "var gaJsHost = ((\"https:\" == document.location.protocol) ? \"https://ssl.\" : \"http://www.\");";
     os << "document.write(unescape(\"%3Cscript src='\" + gaJsHost + \"google-analytics.com/ga.js' type='text/javascript'%3E%3C/script%3E\"));";
     os << "</script>";
     os << "<script type=\"text/javascript\">";
     os << "try {";
    	 os << "var pageTracker = _gat._getTracker(\"UA-3567484-1\");";
     os << "pageTracker._trackPageview();";
     os << "} catch(err) {}</script>";


     os << "</body>";
     os << "</html>";
     os << "\n\n\n";

     return os.str();
}

