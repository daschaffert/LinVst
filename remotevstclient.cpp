/*
  dssi-vst: a DSSI plugin wrapper for VST effects and instruments
  Copyright 2004-2007 Chris Cannam

     This file is part of linvst.

    linvst is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "remotevstclient.h"

#include "aeffectx.h"

#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <iostream>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>

#include "rdwrops.h"
#include "paths.h"

#include <iostream>
#include <string>
#include <fstream>

#include <CoreFoundation/CoreFoundation.h>


RemoteVSTClient::RemoteVSTClient(audioMasterCallback theMaster) :
    RemotePluginClient(theMaster)
{
    pid_t child;
	SInt32 errorCode;
	CFDataRef data;
	CFDictionaryRef props;
	CFArrayRef bundledllfiles;
	std::string dllName;
	char dllpath[PATH_MAX];
	
	CFStringRef bundref = CFStringCreateWithCString(0, "com.linvst", 0);
	CFBundleRef bundle = CFBundleGetBundleWithIdentifier(bundref);
	
	CFURLRef configUrl = CFBundleCopyResourceURL(bundle, CFSTR("config"), CFSTR("plist"), NULL);

	if (configUrl == 0) {		
		m_runok = 1;
		return;
	}
		
	CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault, configUrl, &data, NULL, NULL, &errorCode);

	props = (CFDictionaryRef) CFPropertyListCreateFromXMLData( kCFAllocatorDefault, data, kCFPropertyListImmutable, NULL);
	
	// Vst dll is in Resources vst folder, CFSTR("Vst") gets replaced with NULL for vst dll in Resource folder
	
	bundledllfiles = NULL;
		
	bundledllfiles = CFBundleCopyResourceURLsOfType(bundle, CFSTR("dll"), NULL );
	
	if(bundledllfiles == NULL) {
	bundledllfiles = CFBundleCopyResourceURLsOfType(bundle, CFSTR("DLL"), NULL );
	}
	
	if(bundledllfiles == NULL) {
	bundledllfiles = CFBundleCopyResourceURLsOfType(bundle, CFSTR("Dll"), NULL );
	}
			
	int numOfFiles = CFArrayGetCount(bundledllfiles);
	
    if (numOfFiles == 0) {
		m_runok = 1;
		return;
	}
	
	CFURLRef dllurl = NULL;
	
	dllurl = (CFURLRef)CFArrayGetValueAtIndex(bundledllfiles, 0);
	
	if (dllurl == NULL) 
	{
		m_runok = 1;
		return;
	}
		
	if (CFURLGetFileSystemRepresentation(dllurl, true, (UInt8 *)dllpath, sizeof(dllpath)))
    {
    dllName = dllpath;
    }
    else
	{
	if(bundledllfiles != NULL)
	CFRelease(bundledllfiles);
	m_runok = 1;
	return;
	}
				
	if(bundledllfiles != NULL)
	CFRelease(bundledllfiles);
		 
    std::string arg = dllName + "," + getFileIdentifiers();

    const char *argStr = arg.c_str();
	
	CFURLRef server_url = CFBundleCopyResourceURL(bundle, CFSTR("lin-vst-server.exe"), CFSTR("so"), NULL);

	if (server_url == NULL) {
	  m_runok = 1;
      return;
      }


		CFStringRef server_path = CFURLCopyFileSystemPath(server_url, kCFURLPOSIXPathStyle);

		if (server_path == NULL) {
		 m_runok = 1;
         return;
         }
        
			const char *c_server_path = CFStringGetCStringPtr(server_path, NULL);

			CFStringRef wine_path =  (CFStringRef) CFDictionaryGetValue(props, CFSTR("wine-path"));
	
			if (wine_path == NULL) {
			  m_runok = 1;
              return;
              }

				const char *c_wine_path = CFStringGetCStringPtr(wine_path, NULL);	

				std::cerr << "RemoteVSTClient: executing " << c_server_path << "\n";
				
				if ((child = fork()) < 0) {
	                     m_runok = 1;
	                     return;
				} else if (child == 0) { // child process
					if (execlp(c_wine_path, "wine", c_server_path, argStr, NULL)) {
	                      m_runok = 1;
	                      return;
					}
				}
				
				syncStartup();
}

RemoteVSTClient::~RemoteVSTClient()
{
    for(int i = 0; i < 3; ++i) {
	if (waitpid(-1, NULL, WNOHANG)) break;
	sleep(1);
    }
}

