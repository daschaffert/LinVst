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

#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>

#include "rdwrops.h"
#include "paths.h"

#include <dlfcn.h> 

#include <iostream>
#include <string>
#include <fstream>

const char *selfname()
{
int i = 5;
}

RemoteVSTClient::RemoteVSTClient(audioMasterCallback theMaster) :
    RemotePluginClient(theMaster)
{
     pid_t child;

    Dl_info info;

char newname [2048];

size_t len;

    if(!dladdr((const char*)selfname, &info))
     {
     m_runok = 1;
     return;
     }

    if(!info.dli_fname)
     {
     m_runok = 1;
     return;
     }
  
strcpy(newname, info.dli_fname);

len = strlen(info.dli_fname);

newname[len - 4] = '.';

newname[len - 3] = 's';

newname[len - 2] = 'o';

newname[len - 1] = '\0';


printf("%s\n", newname);


std::string dllName = newname;

    std::string arg = dllName + "," + getFileIdentifiers();

    const char *argStr = arg.c_str();

				if ((child = vfork()) < 0) {
                                        m_runok = 1;
                                        return;
				} else if (child == 0) { 
					

       if (execlp("/usr/bin/lin-vst-server", "/usr/bin/lin-vst-server", argStr, NULL)) {
                                        m_runok = 1;
                                         return;
					}  

                                         }
                                         
	                                syncStartup();
    
	
 }

RemoteVSTClient::~RemoteVSTClient()
{
    for(int i=0;i<300;i++)
    {
    if(waitpid(-1, NULL, WNOHANG)) 
    break;
    usleep(10000);
    } 
}


