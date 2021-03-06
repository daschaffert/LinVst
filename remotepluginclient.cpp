/*  dssi-vst: a DSSI plugin wrapper for VST effects and instruments
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

#include "remotepluginclient.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <time.h>
#include <iostream>
#include <errno.h>

#include "paths.h"

#ifdef AMT

void* RemotePluginClient::AMThread()
{
    int         opcode;
    int         val;
    int         ok = 1;

   int timeout = 50;

    VstTimeInfo *timeInfo;

    int         els;
    int         *ptr2;
    int         sizeidx = 0;
    int         size;
    VstEvents   *evptr;

    struct amessage
    {
        int flags;
        int pcount;
        int parcount;
        int incount;
        int outcount;
        int delay;
    } am;

    while (!m_threadbreak)
    {
        if (m_shm)
        {
    timespec ts_timeout;

    if(m_386run == 0)
    {

    clock_gettime(CLOCK_REALTIME, &ts_timeout);
    time_t seconds = timeout / 1000;
    ts_timeout.tv_sec += seconds;
    ts_timeout.tv_nsec += (timeout - seconds * 1000) * 1000000;
    if (ts_timeout.tv_nsec >= 1000000000) {
        ts_timeout.tv_nsec -= 1000000000;
        ts_timeout.tv_sec++;
    }   

    if (sem_timedwait(&m_shmControl3->runServer, &ts_timeout)) {
        if (errno == ETIMEDOUT) {
          continue;
        } else {
        if(m_inexcept == 0)
            RemotePluginClosedException();
        }
    }
}
else
{
    if (fwait(&m_shmControl3->runServer386, timeout)) {
        if (errno == ETIMEDOUT) {
            continue;
        } else {
            RemotePluginClosedException();
        }
    }
}

    while (dataAvailable(&m_shmControl3->ringBuffer)) {

    if(m_threadbreak)
    break;
   
                opcode = -1;

   tryReadring(&m_shmControl3->ringBuffer, &opcode, sizeof(int));

                switch(opcode)
                {
                    case audioMasterGetTime:       
                    val = readIntring(&m_shmControl3->ringBuffer);
                    timeInfo = (VstTimeInfo *) m_audioMaster(theEffect, audioMasterGetTime, 0, val, 0, 0);
                    memcpy((VstTimeInfo*)&m_shm3[61440], timeInfo, sizeof(VstTimeInfo));
                    break;

                case audioMasterIOChanged:
                    memcpy(&am, &m_shm3[65536], sizeof(am));
                    theEffect->flags = am.flags;
                    theEffect->numPrograms = am.pcount;
                    theEffect->numParams = am.parcount;
                    theEffect->numInputs = am.incount;
                    theEffect->numOutputs = am.outcount;
                    theEffect->initialDelay = am.delay;
                    // m_updateio = 1;
                    m_audioMaster(theEffect, audioMasterIOChanged, 0, 0, 0, 0);

                    break;

                case audioMasterProcessEvents:
                    val = readIntring(&m_shmControl3->ringBuffer);

                    ptr2 = (int *)m_shm3;
                    els = *ptr2;
                    sizeidx = sizeof(int);

                    if (els > VSTSIZE)
                        els = VSTSIZE;

                    evptr = &vstev[0];
                    evptr->numEvents = els;
                    evptr->reserved = 0;

                    for (int i = 0; i < els; i++)
                    {
                        VstEvent* bsize = (VstEvent*) &m_shm3[sizeidx];
                        size = bsize->byteSize + (2*sizeof(VstInt32));
                        evptr->events[i] = bsize;
                        sizeidx += size;
                    }

                    m_audioMaster(theEffect, audioMasterProcessEvents, 0, val, evptr, 0);
               
                    break;

                default:
                    break;
                }
            }
     if(m_386run == 0)
     {
     if (sem_post(&m_shmControl3->runClient)) {
        std::cerr << "Could not post to semaphore\n";
     }
     }
     else
     { 
     if (fpost(&m_shmControl3->runClient386)) {
        std::cerr << "Could not post to semaphore\n";
     }
     }
}
}
     // m_threadbreakexit = 1;
    // pthread_exit(0);
    return 0;
}

#endif

RemotePluginClient::RemotePluginClient(audioMasterCallback theMaster) :
#ifdef AMT
    m_audioMaster(theMaster),
#endif
    m_controlRequestFd(-1),
    m_controlResponseFd(-1),
    m_parRequestFd(-1),
    m_parResponseFd(-1),
#ifndef RINGB
    m_processFd(-1),
    m_processResponseFd(-1),
#endif
    m_shmFd(-1),
    m_shmFd2(-1),
    m_shmFd3(-1),
    m_controlRequestFileName(0),
    m_controlResponseFileName(0),
    m_parRequestFileName(0),
    m_parResponseFileName(0),
#ifndef RINGB
    m_processFileName(0),
    m_processResponseFileName(0),
#endif
#ifdef AMT
    m_AMThread(0),
    m_threadbreak(0),
    m_threadbreakexit(0),
    m_updateio(0),
#endif
    m_shmFileName(0),
    m_shm(0),
    m_shmSize(0),
    m_shmFileName2(0),
    m_shm2(0),
    m_shmSize2(0),
    m_shmFileName3(0),
    m_shm3(0),
    m_shmSize3(0),
#ifdef RINGB
    m_shmControlFd(-1),
    m_shmControl(0),
    m_shmControlFileName(0),
    m_shmControl2Fd(-1),
    m_shmControl2(0),
    m_shmControl2FileName(0),
    m_shmControl3Fd(-1),
    m_shmControl3(0),
    m_shmControl3FileName(0),
#endif
    m_bufferSize(-1),
    m_numInputs(-1),
    m_numOutputs(-1),
    m_finishaudio(0),
    m_runok(0),
    m_inexcept(0),
    m_386run(0),
    theEffect(0)
{
    char tmpFileBase[60];

    srand(time(NULL));

    sprintf(tmpFileBase, "/tmp/rplugin_crq_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_controlRequestFileName = strdup(tmpFileBase);

    unlink(m_controlRequestFileName);
    if (mkfifo(m_controlRequestFileName, 0666)) //FIXME what permission is correct here?
    {
        perror(m_controlRequestFileName);
        cleanup();
        throw((std::string)"Failed to create FIFO");
    }

    sprintf(tmpFileBase, "/tmp/rplugin_crs_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_controlResponseFileName = strdup(tmpFileBase);

    unlink(m_controlResponseFileName);
    if (mkfifo(m_controlResponseFileName, 0666)) //FIXME what permission is correct here?
    {
        perror(m_controlResponseFileName);
        cleanup();
        throw((std::string)"Failed to create FIFO");
    }

    sprintf(tmpFileBase, "/tmp/rplugin_gpa_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_parRequestFileName = strdup(tmpFileBase);

    unlink(m_parRequestFileName);
    if (mkfifo(m_parRequestFileName, 0666)) //FIXME what permission is correct here?
    {
        perror(m_parRequestFileName);
        cleanup();
        throw((std::string)"Failed to create FIFO");
    }

    sprintf(tmpFileBase, "/tmp/rplugin_spa_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_parResponseFileName = strdup(tmpFileBase);

    unlink(m_parResponseFileName);
    if (mkfifo(m_parResponseFileName, 0666)) //FIXME what permission is correct here?
    {
        perror(m_parResponseFileName);
        cleanup();
        throw((std::string)"Failed to create FIFO");
    }

#ifndef RINGB

    sprintf(tmpFileBase, "/tmp/rplugin_prc_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_processFileName = strdup(tmpFileBase);

    unlink(m_processFileName);
    if (mkfifo(m_processFileName, 0666)) //FIXME what permission is correct here?
    {
        perror(m_processFileName);
        cleanup();
        throw((std::string)"Failed to create FIFO");
    }

    sprintf(tmpFileBase, "/tmp/rplugin_prr_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_processResponseFileName = strdup(tmpFileBase);

    unlink(m_processResponseFileName);
    if (mkfifo(m_processResponseFileName, 0666)) //FIXME what permission is correct here?
    {
        perror(m_processResponseFileName);
        cleanup();
        throw((std::string)"Failed to create FIFO");
    }

#endif

    sprintf(tmpFileBase, "/vstrplugin_shm_XXXXXX");
    m_shmFd = shm_mkstemp(tmpFileBase);
    if (m_shmFd < 0)
    {
        cleanup();
        throw((std::string)"Failed to open or create shared memory file");
    }
    m_shmFileName = strdup(tmpFileBase);

    sprintf(tmpFileBase, "/vstrplugin_shn_XXXXXX");
    m_shmFd2 = shm_mkstemp(tmpFileBase);
    if (m_shmFd2 < 0)
    {
        cleanup();
        throw((std::string)"Failed to open or create shared memory file");
    }
    m_shmFileName2 = strdup(tmpFileBase);

    sprintf(tmpFileBase, "/vstrplugin_sho_XXXXXX");
    m_shmFd3 = shm_mkstemp(tmpFileBase);
    if (m_shmFd3 < 0)
    {
        cleanup();
        throw((std::string)"Failed to open or create shared memory file");
    }
    m_shmFileName3 = strdup(tmpFileBase);

#ifdef RINGB

    sprintf(tmpFileBase, "/vstrplugin_shc_XXXXXX");
    m_shmControlFd = shm_mkstemp(tmpFileBase);
    if (m_shmControlFd < 0) {
	cleanup();
	throw((std::string)"Failed to open or create shared memory file");
    }

    m_shmControlFileName = strdup(tmpFileBase);

    sprintf(tmpFileBase, "/vstrplugin_shz_XXXXXX");
    m_shmControl2Fd = shm_mkstemp(tmpFileBase);
    if (m_shmControl2Fd < 0) {
	cleanup();
	throw((std::string)"Failed to open or create shared memory file");
    }

    m_shmControl2FileName = strdup(tmpFileBase);

    sprintf(tmpFileBase, "/vstrplugin_shy_XXXXXX");
    m_shmControl3Fd = shm_mkstemp(tmpFileBase);
    if (m_shmControl3Fd < 0) {
	cleanup();
	throw((std::string)"Failed to open or create shared memory file");
    }

    m_shmControl3FileName = strdup(tmpFileBase);

#endif

}

RemotePluginClient::~RemotePluginClient()
{

#ifdef AMT
    m_threadbreak = 1;
    m_threadbreakexit = 1;
#endif
 
    if (theEffect)
    delete theEffect;
    cleanup();
}

void RemotePluginClient::syncStartup()
{
    // The first (write) fd we open in a nonblocking call, with a
    // short retry loop so we can easily give up if the other end
    // doesn't appear to be responding.  We want a nonblocking FIFO
    // for this and the process fd anyway.
    bool    connected = false;
    int     timeout = 60;

    for (int attempt = 0; attempt < timeout; ++attempt)
    {
        if ((m_controlRequestFd = open(m_controlRequestFileName, O_WRONLY | O_NONBLOCK)) >= 0)
        {
            connected = true;
            break;
        }
        else if (errno != ENXIO)
            break; // an actual error occurred
        usleep(250000);
    }

    if (!connected)
    {
        cleanup();
        throw((std::string)"Plugin server timed out on startup");
    }

    if (connected)
    {
        if ((m_controlResponseFd = open(m_controlResponseFileName, O_RDONLY)) < 0)
        {
            cleanup();
            throw((std::string)"Failed to open control FIFO");
        }

        connected = false;

        for (int attempt = 0; attempt < timeout; ++attempt)
        {
            if ((m_parRequestFd = open(m_parRequestFileName, O_WRONLY | O_NONBLOCK)) >= 0)
            {
                connected = true;
                break;
            }
            else if (errno != ENXIO)
                break; // an actual error occurred
            usleep(250000);
        }

        if (!connected)
        {
            cleanup();
            throw((std::string)"Plugin server timed out on startup");
        }

        if ((m_parResponseFd = open(m_parResponseFileName, O_RDONLY)) < 0)
        {
            cleanup();
            throw((std::string)"Failed to open control FIFO");
        }

#ifndef RINGB

        connected = false;

        for (int attempt = 0; attempt < timeout; ++attempt)
        {
            if ((m_processFd = open(m_processFileName, O_WRONLY | O_NONBLOCK)) >= 0)
            {
                connected = true;
                break;
            }
            else if (errno != ENXIO)
                break; // an actual error occurred
            usleep(250000);
        }

        if (!connected)
        {
            cleanup();
            throw((std::string)"Failed to open process FIFO");
        }

        if ((m_processResponseFd = open(m_processResponseFileName, O_RDONLY)) < 0)
        {
            cleanup();
            throw((std::string)"Failed to open process FIFO");
        }
#endif
    }

    bool b = false;
    tryRead(m_controlResponseFd, &b, sizeof(bool));
    if (!b)
    {
        cleanup();
        throw((std::string)"Remote plugin did not start correctly");
    }

    tryWrite(m_controlRequestFd, &m_386run, sizeof(int));

    theEffect = new AEffect;
}

void RemotePluginClient::cleanup()
{

#ifdef AMT
/*
    if (m_shm)
        for (int i=0;i<1000;i++)
        {
            usleep(10000);
            if (m_threadbreakexit)
            break;
        }

*/
    if (m_AMThread)
        pthread_join(m_AMThread, NULL);
#endif

    if (m_controlRequestFd >= 0)
    {
        close(m_controlRequestFd);
        m_controlRequestFd = -1;
    }
    if (m_controlResponseFd >= 0)
    {
        close(m_controlResponseFd);
        m_controlResponseFd = -1;
    }
    if (m_parRequestFd >= 0)
    {
        close(m_parRequestFd);
        m_parRequestFd = -1;
    }
    if (m_parResponseFd >= 0)
    {
        close(m_parResponseFd);
        m_parResponseFd = -1;
    }
#ifndef RINGB
    if (m_processFd >= 0)
    {
        close(m_processFd);
        m_processFd = -1;
    }
    if (m_processResponseFd >= 0)
    {
        close(m_processResponseFd);
        m_processResponseFd = -1;
    }
#endif

    if (m_controlRequestFileName)
    {
        unlink(m_controlRequestFileName);
        free(m_controlRequestFileName);
        m_controlRequestFileName = 0;
    }
    if (m_controlResponseFileName)
    {
        unlink(m_controlResponseFileName);
        free(m_controlResponseFileName);
        m_controlResponseFileName = 0;
    }
    if (m_parRequestFileName)
    {
        unlink(m_parRequestFileName);
        free(m_parRequestFileName);
        m_parRequestFileName = 0;
    }
    if (m_parResponseFileName)
    {
        unlink(m_parResponseFileName);
        free(m_parResponseFileName);
        m_parResponseFileName = 0;
    }
#ifndef RINGB
    if (m_processFileName)
    {
        unlink(m_processFileName);
        free(m_processFileName);
        m_processFileName = 0;
    }
    if (m_processResponseFileName)
    {
        unlink(m_processResponseFileName);
        free(m_processResponseFileName);
        m_processResponseFileName = 0;
    }
#endif

    if (m_shm)
    {
        munlock(m_shm, m_shmSize);
        munmap(m_shm, m_shmSize);
        m_shm = 0;
    }
    if (m_shm2)
    {
        munlock(m_shm2, m_shmSize2);
        munmap(m_shm2, m_shmSize2);
        m_shm2 = 0;
    }
    if (m_shm3)
    {
        munlock(m_shm3, m_shmSize3);
        munmap(m_shm3, m_shmSize3);
        m_shm3 = 0;
    }

    if (m_shmFd >= 0)
    {
        close(m_shmFd);
        m_shmFd = -1;
    }
    if (m_shmFd2 >= 0)
    {
        close(m_shmFd2);
        m_shmFd2 = -1;
    }
    if (m_shmFd3 >= 0)
    {
        close(m_shmFd3);
        m_shmFd3 = -1;
    }

    if (m_shmFileName)
    {
	shm_unlink(m_shmFileName);
        free(m_shmFileName);
        m_shmFileName = 0;
    }
    if (m_shmFileName2)
    {
 	shm_unlink(m_shmFileName2);
        free(m_shmFileName2);
        m_shmFileName2 = 0;
    }
    if (m_shmFileName3)
    {
	shm_unlink(m_shmFileName3);
        free(m_shmFileName3);
        m_shmFileName3 = 0;
    }

#ifdef RINGB

    if (m_shmControl) {
        munlock(m_shmControl, sizeof(ShmControl));
        munmap(m_shmControl, sizeof(ShmControl));
        m_shmControl = 0;
    }
    if (m_shmControlFd >= 0) {
        close(m_shmControlFd);
        m_shmControlFd = -1;
    }
    if (m_shmControlFileName) {
        shm_unlink(m_shmControlFileName);
        free(m_shmControlFileName);
        m_shmControlFileName = 0;
    }

    if (m_shmControl2) {
        munlock(m_shmControl2, sizeof(ShmControl));
        munmap(m_shmControl2, sizeof(ShmControl));
        m_shmControl2 = 0;
    }
    if (m_shmControl2Fd >= 0) {
        close(m_shmControl2Fd);
        m_shmControl2Fd = -1;
    }
    if (m_shmControl2FileName) {
        shm_unlink(m_shmControl2FileName);
        free(m_shmControl2FileName);
        m_shmControl2FileName = 0;
    }

    if (m_shmControl3) {
        munlock(m_shmControl3, sizeof(ShmControl));
        munmap(m_shmControl3, sizeof(ShmControl));
        m_shmControl3 = 0;
    }
    if (m_shmControl3Fd >= 0) {
        close(m_shmControl3Fd);
        m_shmControl3Fd = -1;
    }
    if (m_shmControl3FileName) {
        shm_unlink(m_shmControl3FileName);
        free(m_shmControl3FileName);
        m_shmControl3FileName = 0;
    }

#endif

}

std::string RemotePluginClient::getFileIdentifiers()
{
    std::string id;
    id += m_controlRequestFileName + strlen(m_controlRequestFileName) - 6;
    id += m_controlResponseFileName + strlen(m_controlResponseFileName) - 6;
    id += m_parRequestFileName + strlen(m_parRequestFileName) - 6;
    id += m_parResponseFileName + strlen(m_parResponseFileName) - 6;
#ifndef RINGB
    id += m_processFileName + strlen(m_processFileName) - 6;
    id += m_processResponseFileName + strlen(m_processResponseFileName) - 6;
#endif
    id += m_shmFileName + strlen(m_shmFileName) - 6;
    id += m_shmFileName2 + strlen(m_shmFileName2) - 6;
    id += m_shmFileName3 + strlen(m_shmFileName3) - 6;
#ifdef RINGB
    id += m_shmControlFileName + strlen(m_shmControlFileName) - 6;
    id += m_shmControl2FileName + strlen(m_shmControl2FileName) - 6;
    id += m_shmControl3FileName + strlen(m_shmControl3FileName) - 6;
#endif

    //  std::cerr << "Returning file identifiers: " << id << std::endl;
    return id;
}

int RemotePluginClient::sizeShm()
{
    if (m_shm)
    return 0;

    size_t sz = FIXED_SHM_SIZE;
    size_t sz2 = 66048;
#ifdef AMT
    size_t sz3 = 66048;
#else
    size_t sz3 = 512;
#endif

    ftruncate(m_shmFd, sz);
    m_shm = (char *)mmap(0, sz, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd, 0);
    if (!m_shm)
    {
        std::cerr << "RemotePluginClient::sizeShm: ERROR: mmap or mremap failed for " << sz
                    << " bytes from fd " << m_shmFd << "!" << std::endl;
        m_shmSize = 0;
        return 1;     
    }
    else
    {
        memset(m_shm, 0, sz);
        m_shmSize = sz;
        
        if(mlock(m_shm, sz) != 0)
        perror("mlock fail1");
    }

    ftruncate(m_shmFd2, sz2);
    m_shm2 = (char *)mmap(0, sz2, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd2, 0);
    if (!m_shm2)
    {
        std::cerr << "RemotePluginClient::sizeShm: ERROR: mmap or mremap failed for " << sz2
                    << " bytes from fd " << m_shmFd2 << "!" << std::endl;
        m_shmSize2 = 0;
        return 1;     
    }
    else
    {
        memset(m_shm2, 0, sz2);
        m_shmSize2 = sz2;

        if(mlock(m_shm2, sz2) != 0)
        perror("mlock fail2");

    }

    ftruncate(m_shmFd3, sz3);
    m_shm3 = (char *)mmap(0, sz3, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd3, 0);
    if (!m_shm3)
    {
        std::cerr << "RemotePluginClient::sizeShm: ERROR: mmap or mremap failed for " << sz3
                    << " bytes from fd " << m_shmFd3 << "!" << std::endl;
        m_shmSize3 = 0;
        return 1;     
    }
    else
    {
        memset(m_shm3, 0, sz3);
        m_shmSize3 = sz3;

        if(mlock(m_shm3, sz3) != 0)
        perror("mlock fail3");

    }

#ifdef RINGB

    ftruncate(m_shmControlFd, sizeof(ShmControl));

    m_shmControl = static_cast<ShmControl *>(mmap(0, sizeof(ShmControl), PROT_READ | PROT_WRITE, MAP_SHARED, m_shmControlFd, 0));

    if (!m_shmControl) {
        return 1;     
    }

    memset(m_shmControl, 0, sizeof(ShmControl));

    if(mlock(m_shmControl, sizeof(ShmControl)) != 0)
    perror("mlock fail4");

    if(m_386run == 0)
    {

    if (sem_init(&m_shmControl->runServer, 1, 0)) {
        return 1;     
    }
    if (sem_init(&m_shmControl->runClient, 1, 0)) {
        return 1;     
    }

    }

    ftruncate(m_shmControl2Fd, sizeof(ShmControl));

    m_shmControl2 = static_cast<ShmControl *>(mmap(0, sizeof(ShmControl), PROT_READ | PROT_WRITE, MAP_SHARED, m_shmControl2Fd, 0));

    if (!m_shmControl2) {
        return 1;     
    }

    memset(m_shmControl2, 0, sizeof(ShmControl));

    if(mlock(m_shmControl2, sizeof(ShmControl)) != 0)
    perror("mlock fail5");

    if(m_386run == 0)
    {

    if (sem_init(&m_shmControl2->runServer, 1, 0)) {
        return 1;     
    }
    if (sem_init(&m_shmControl2->runClient, 1, 0)) {
        return 1;     
    }

    }

    ftruncate(m_shmControl3Fd, sizeof(ShmControl));

    m_shmControl3 = static_cast<ShmControl *>(mmap(0, sizeof(ShmControl), PROT_READ | PROT_WRITE, MAP_SHARED, m_shmControl3Fd, 0));

    if (!m_shmControl3) {
        return 1;     
    }

    memset(m_shmControl3, 0, sizeof(ShmControl));

    if(mlock(m_shmControl3, sizeof(ShmControl)) != 0)
    perror("mlock fail4");

    if(m_386run == 0)
    {

    if (sem_init(&m_shmControl3->runServer, 1, 0)) {
        return 1;     
    }
    if (sem_init(&m_shmControl3->runClient, 1, 0)) {
        return 1;     
    }

    }

#endif

#ifdef AMT
    m_threadbreak = 0;
    // m_threadbreakexit = 0;

    if(pthread_create(&m_AMThread, NULL, RemotePluginClient::callAMThread, this) != 0)
    return 1; 
 #endif

return 0;

}

float RemotePluginClient::getVersion()
{
    //FIXME!!! client code needs to be testing this
    writeOpcode(m_parRequestFd, RemotePluginGetVersion);
    return readFloat(m_parResponseFd);
}

int RemotePluginClient::getUID()
{
    writeOpcode(m_parRequestFd, RemotePluginUniqueID);
    return readInt(m_parResponseFd);
}

std::string RemotePluginClient::getName()
{
    writeOpcode(m_parRequestFd, RemotePluginGetName);
    return readString(m_parResponseFd);
}

std::string RemotePluginClient::getMaker()
{
    writeOpcode(m_parRequestFd, RemotePluginGetMaker);
    return readString(m_parResponseFd);
}

void RemotePluginClient::setBufferSize(int s)
{
    if (s <= 0)
        return;

    if (s == m_bufferSize)
        return;

    if (!m_shm)
    {  
    if(sizeShm() == 1)
    effVoidOp(effClose);
    }

    m_bufferSize = s;
    writeOpcode(m_parRequestFd, RemotePluginSetBufferSize);
    writeInt(m_parRequestFd, s);
    int ok = readInt(m_parResponseFd);
}

void RemotePluginClient::setSampleRate(int s)
{
    writeOpcode(m_parRequestFd, RemotePluginSetSampleRate);
    writeInt(m_parRequestFd, s);
    int ok = readInt(m_parResponseFd);
}

void RemotePluginClient::reset()
{
    writeOpcode(m_parRequestFd, RemotePluginReset);
    if (m_shmSize > 0)
    {
        memset(m_shm, 0, m_shmSize);
        memset(m_shm2, 0, m_shmSize2);
        memset(m_shm3, 0, m_shmSize3);
    }
    int ok = readInt(m_parResponseFd);
}

void RemotePluginClient::terminate()
{
    writeOpcode(m_parRequestFd, RemotePluginTerminate);
}

int RemotePluginClient::getEffInt(int opcode)
{
    writeOpcode(m_parRequestFd, RemotePluginGetEffInt);
    writeInt(m_parRequestFd, opcode);
    return readInt(m_parResponseFd);
}

void RemotePluginClient::getEffString(int opcode, int index, char *ptr, int len)
{
    writeOpcode(m_parRequestFd, RemotePluginGetEffString);
    writeInt(m_parRequestFd, opcode);
    writeInt(m_parRequestFd, index);
    strncpy(ptr, readString(m_parResponseFd).c_str(), len);
    ptr[len-1] = 0;
}

int RemotePluginClient::getFlags()
{
    writeOpcode(m_parRequestFd, RemotePluginGetFlags);
    return readInt(m_parResponseFd);
}

int RemotePluginClient::getinitialDelay()
{
    writeOpcode(m_parRequestFd, RemotePluginGetinitialDelay);
    return readInt(m_parResponseFd);
}

int RemotePluginClient::getInputCount()
{
    // writeOpcode(m_processFd, RemotePluginGetInputCount);
    // m_numInputs = readInt(m_processResponseFd);

    writeOpcode(m_parRequestFd, RemotePluginGetInputCount);
    m_numInputs = readInt(m_parResponseFd);
    return m_numInputs;
}

int RemotePluginClient::getOutputCount()
{
    // writeOpcode(m_processFd, RemotePluginGetOutputCount);
    // m_numOutputs = readInt(m_processResponseFd);

    writeOpcode(m_parRequestFd, RemotePluginGetOutputCount);
    m_numOutputs = readInt(m_parResponseFd);
    return m_numOutputs;
}

int RemotePluginClient::getParameterCount()
{
    writeOpcode(m_parRequestFd, RemotePluginGetParameterCount);
    return readInt(m_parResponseFd);
}

std::string RemotePluginClient::getParameterName(int p)
{
    writeOpcode(m_parRequestFd, RemotePluginGetParameterName);
    writeInt(m_parRequestFd, p);
    return readString(m_parResponseFd);
}

#ifdef RINGB

void
RemotePluginClient::setParameter(int p, float v)
{
    if(m_finishaudio == 1)
    return;

    if(m_shm)
    {
    writeOpcodering(&m_shmControl->ringBuffer, RemotePluginSetParameter);
    writeIntring(&m_shmControl->ringBuffer, p);
    writeFloatring(&m_shmControl->ringBuffer, v);
    commitWrite(&m_shmControl->ringBuffer);
    waitForServer();
    }
}

float
RemotePluginClient::getParameter(int p)
{
    float *ptr2;

    if(m_finishaudio == 1)
    return 0;

    if(m_shm)
    {
    ptr2 = (float *)&m_shm[FIXED_SHM_SIZE - 512];
    writeOpcodering(&m_shmControl->ringBuffer, RemotePluginGetParameter);
    writeIntring(&m_shmControl->ringBuffer, p);
    commitWrite(&m_shmControl->ringBuffer);
    waitForServer();
    return *ptr2;
    }
}

#else

void RemotePluginClient::setParameter(int p, float v)
{
    writeOpcode(m_parRequestFd, RemotePluginSetParameter);
    writeInt(m_parRequestFd, p);
    writeFloat(m_parRequestFd, v);
    // wait for a response
    int ok = readInt(m_parResponseFd);
}

float RemotePluginClient::getParameter(int p)
{
    writeOpcode(m_parRequestFd, RemotePluginGetParameter);
    writeInt(m_parRequestFd, p);
    return readFloat(m_parResponseFd);
}

#endif

float RemotePluginClient::getParameterDefault(int p)
{
    writeOpcode(m_parRequestFd, RemotePluginGetParameterDefault);
    writeInt(m_parRequestFd, p);
    return readFloat(m_parResponseFd);
}

void RemotePluginClient::getParameters(int p0, int pn, float *v)
{
    writeOpcode(m_parRequestFd, RemotePluginGetParameters);
    writeInt(m_parRequestFd, p0);
    writeInt(m_parRequestFd, pn);
    tryRead(m_parResponseFd, v, (pn - p0 + 1) * sizeof(float));
}

int RemotePluginClient::getProgramCount()
{
    writeOpcode(m_parRequestFd, RemotePluginGetProgramCount);
    return readInt(m_parResponseFd);
}

std::string RemotePluginClient::getProgramNameIndexed(int n)
{
    writeOpcode(m_parRequestFd, RemotePluginGetProgramNameIndexed);
    writeInt(m_parRequestFd, n);
    return readString(m_parResponseFd);
}

std::string RemotePluginClient::getProgramName()
{
    writeOpcode(m_parRequestFd, RemotePluginGetProgramName);
    return readString(m_parResponseFd);
}

void RemotePluginClient::setCurrentProgram(int n)
{
    writeOpcode(m_parRequestFd, RemotePluginSetCurrentProgram);
    writeInt(m_parRequestFd, n);
    int ok = readInt(m_parResponseFd);
}

#ifdef RINGB
void
RemotePluginClient::waitForServer()
{

    if(m_386run == 0)
    {
    sem_post(&m_shmControl->runServer);

    timespec ts_timeout;
    clock_gettime(CLOCK_REALTIME, &ts_timeout);
    ts_timeout.tv_sec += 5;
    if (sem_timedwait(&m_shmControl->runClient, &ts_timeout) != 0) {
        if(m_inexcept == 0)
	RemotePluginClosedException();
    }
    }
    else
    {
    fpost(&m_shmControl->runServer386);

    if (fwait(&m_shmControl->runClient386, 5000)) {
         if(m_inexcept == 0)
	 RemotePluginClosedException();
    }
    }
}

void
RemotePluginClient::waitForServer2()
{

    if(m_386run == 0)
    {
    sem_post(&m_shmControl2->runServer);

    timespec ts_timeout;
    clock_gettime(CLOCK_REALTIME, &ts_timeout);
    ts_timeout.tv_sec += 5;
    if (sem_timedwait(&m_shmControl2->runClient, &ts_timeout) != 0) {
        if(m_inexcept == 0)
	RemotePluginClosedException();
    }
    }
    else
    {
    fpost(&m_shmControl2->runServer386);

    if (fwait(&m_shmControl2->runClient386, 5000)) {
         if(m_inexcept == 0)
	 RemotePluginClosedException();
    }
    }
}

#endif

void RemotePluginClient::process(float **inputs, float **outputs, int sampleFrames)
{
    if ((m_bufferSize <= 0) || (sampleFrames <= 0) || (m_numInputs < 0) || (m_numOutputs < 0) || (m_finishaudio == 1) || (!m_shm))
        return;

    if ((m_numInputs + m_numOutputs) * m_bufferSize * sizeof(float) > FIXED_SHM_SIZE)
        return;

#ifdef AMT
    if (m_updateio)
    {
        getInputCount();
        getOutputCount();
        m_updateio = 0;
    }
#endif

    size_t blocksz = sampleFrames * sizeof(float);

    if(m_numInputs > 0)
    {
    for (int i = 0; i < m_numInputs; ++i)
    memcpy(m_shm + i * blocksz, inputs[i], blocksz);
    }

#ifdef RINGB

    writeOpcodering(&m_shmControl2->ringBuffer, RemotePluginProcess);
    writeIntring(&m_shmControl2->ringBuffer, sampleFrames);

    commitWrite(&m_shmControl2->ringBuffer);

    waitForServer2();  
#else

    writeOpcode(m_processFd, RemotePluginProcess);
    writeInt(m_processFd, sampleFrames);

    int resp;

    resp = readInt(m_processResponseFd);

#endif

    if(m_numOutputs > 0)
    {
    for (int i = 0; i < m_numOutputs; ++i)
    memcpy(outputs[i], m_shm + i * blocksz, blocksz);
    }
    return;
}

int RemotePluginClient::processVstEvents(VstEvents *evnts)
{
    int ret;
    int eventnum;
    int *ptr;
    int sizeidx = 0;

    if ((evnts->numEvents <= 0) || (!evnts) || (m_finishaudio == 1) || (!m_shm))
        return 0;    

    ptr = (int *)m_shm2;
    eventnum = evnts->numEvents;
    sizeidx = sizeof(int);

    if (eventnum > VSTSIZE)
    eventnum = VSTSIZE;            

    for (int i = 0; i < evnts->numEvents; i++)
    {
        VstEvent* pEvent = evnts->events[i];

        if (pEvent->type == kVstSysExType)
        {
            eventnum--;
        }
        else
        {
            unsigned int size = (2*sizeof(VstInt32)) + evnts->events[i]->byteSize;
            memcpy(&m_shm2[sizeidx], evnts->events[i] , size);
            sizeidx += size;
        }
    }

    *ptr = eventnum;
    ret = evnts->numEvents;

#ifdef RINGB
    writeOpcodering(&m_shmControl2->ringBuffer, RemotePluginProcessEvents);
    commitWrite(&m_shmControl2->ringBuffer);

    waitForServer2();  

#else

    writeOpcode(m_processFd, RemotePluginProcessEvents);
    int ok = readInt(m_processResponseFd);

#endif

    return ret;
}

void RemotePluginClient::setDebugLevel(RemotePluginDebugLevel level)
{
    writeOpcode(m_parRequestFd, RemotePluginSetDebugLevel);
    tryWrite(m_parRequestFd, &level, sizeof(RemotePluginDebugLevel));
}

bool RemotePluginClient::warn(std::string str)
{
    writeOpcode(m_parRequestFd, RemotePluginWarn);
    writeString(m_parRequestFd, str);
    bool b;
    tryRead(m_parResponseFd, &b, sizeof(bool));
    return b;
}

void RemotePluginClient::showGUI()
{
    writeOpcode(m_controlRequestFd, RemotePluginShowGUI);

#ifdef EMBED
    tryRead(m_controlResponseFd, &winm, sizeof(winm));
#else
    int ok = readInt(m_controlResponseFd);
#endif
}

void RemotePluginClient::hideGUI()
{
    writeOpcode(m_controlRequestFd, RemotePluginHideGUI);
    int ok = readInt(m_controlResponseFd);
}

#ifdef EMBED
void RemotePluginClient::openGUI()
{
    writeOpcode(m_controlRequestFd, RemotePluginOpenGUI);
}
#endif

void RemotePluginClient::effVoidOp(int opcode)
{
    if (opcode == effClose)
    {
#ifdef AMT
        m_threadbreak = 1;
#endif
        m_finishaudio = 1;
        writeOpcode(m_controlRequestFd, RemotePluginDoVoid);
        writeInt(m_controlRequestFd, opcode);
        int ok = readInt(m_controlResponseFd);
        m_runok = 1;
    }
    else
    {
        writeOpcode(m_controlRequestFd, RemotePluginDoVoid);
        writeInt(m_controlRequestFd, opcode);
        int ok2 = readInt(m_controlResponseFd);
    }
}

int RemotePluginClient::getChunk(void **ptr, int bank_prg)
{
    static void *chunk_ptr = 0;
    writeOpcode(m_parRequestFd, RemotePluginGetChunk);
    writeInt(m_parRequestFd, bank_prg);
    int sz = readInt(m_parResponseFd);

    if (chunk_ptr != 0)
        free(chunk_ptr);
    chunk_ptr = malloc(sz);

    tryRead(m_parResponseFd, chunk_ptr, sz);
    *ptr = chunk_ptr;
    return sz;
}

int RemotePluginClient::setChunk(void *ptr, int sz, int bank_prg)
{
    writeOpcode(m_parRequestFd, RemotePluginSetChunk);
    writeInt(m_parRequestFd, sz);
    writeInt(m_parRequestFd, bank_prg);
    tryWrite2(m_parRequestFd, ptr, sz);
    return readInt(m_parResponseFd);
}

/*
int RemotePluginClient::canBeAutomated(int param)
{
    writeOpcode(m_parRequestFd, RemotePluginCanBeAutomated);
    writeInt(m_parRequestFd, param);
    return readInt(m_parResponseFd);
}
*/

int RemotePluginClient::getProgram()
{
    writeOpcode(m_parRequestFd, RemotePluginGetProgram);
    return readInt(m_parResponseFd);
}

int RemotePluginClient::EffectOpen()
{
    writeOpcode(m_controlRequestFd, RemotePluginEffectOpen);
    return readInt(m_controlResponseFd);
}

#ifdef RINGB

void RemotePluginClient::rdwr_tryWrite2(int fd, const void *buf, size_t count, const char *file, int line)
{
     ssize_t w = 0;

     if(m_runok == 1)
     return;
    
     while ((w = write(fd, buf, count)) < (ssize_t)count) {

     if (w < 0) {

        if (errno == EPIPE) 
        {
        if(m_finishaudio == 1)
        return;
        }
    
    	if (errno != EAGAIN) {
		char message[100];
		sprintf(message, "Write failed on fd %d at %s:%d", fd, file, line);
		perror(message);
                if(m_inexcept == 0)
		RemotePluginClosedException(); 
                break;
	    }
	    w = 0;
    }
    
    buf = (void *)(((char *)buf) + w);
	count -= w;
    }
}

void RemotePluginClient::rdwr_tryRead(int fd, void *buf, size_t count, const char *file, int line)
{
    ssize_t r = 0;

    if(m_runok == 1)
    return;

    while ((r = read(fd, buf, count)) < (ssize_t)count) {

	if (r == 0) {
	    // end of file

            if(m_finishaudio == 1)
            return;
            else 
            {
            if(m_inexcept == 0)
	    RemotePluginClosedException();
            break;
            }
	} else if (r < 0) {
	    if (errno != EAGAIN) {
		char message[100];
		sprintf(message, "Read failed on fd %d at %s:%d", fd, file, line);
		perror(message);
                if(m_inexcept == 0)
		RemotePluginClosedException();
                break;
	    }
	    r = 0;
	}

	buf = (void *)(((char *)buf) + r);
	count -= r;
    }
}

void RemotePluginClient::rdwr_tryWrite(int fd, const void *buf, size_t count, const char *file, int line)
{
       if(m_runok == 1)
       return;

        ssize_t w = write(fd, buf, count);

        if (w < 0) {
	char message[100];

        if (errno == EPIPE) 
        {
        if(m_finishaudio == 1)
        return;
        }
       
	sprintf(message, "Write failed on fd %d at %s:%d", fd, file, line);
	perror(message);
        if(m_inexcept == 0)
	RemotePluginClosedException();
        }

        if (w < (ssize_t)count) {
	fprintf(stderr, "Failed to complete write on fd %d (have %d, put %d) at %s:%d\n",
		fd, count, w, file, line);
        if(m_inexcept == 0)
	RemotePluginClosedException();
        }
}

void RemotePluginClient::rdwr_tryReadring(RingBuffer *ringbuf, void *buf, size_t count, const char *file, int line)
{
    char *charbuf = static_cast<char *>(buf);
    size_t tail = ringbuf->tail;
    size_t head = ringbuf->head;
    size_t wrap = 0;

    if(m_runok == 1)
    return;

    if (head <= tail) {
        wrap = SHM_RING_BUFFER_SIZE;
    }
    if (head - tail + wrap < count) {
        if(m_inexcept == 0)
        RemotePluginClosedException();
    }
    size_t readto = tail + count;
    if (readto >= SHM_RING_BUFFER_SIZE) {
        readto -= SHM_RING_BUFFER_SIZE;
        size_t firstpart = SHM_RING_BUFFER_SIZE - tail;
        memcpy(charbuf, ringbuf->buf + tail, firstpart);
        memcpy(charbuf + firstpart, ringbuf->buf, readto);
    } else {
        memcpy(charbuf, ringbuf->buf + tail, count);
    }
    ringbuf->tail = readto;
}

void RemotePluginClient::rdwr_tryWritering(RingBuffer *ringbuf, const void *buf, size_t count, const char *file, int line)
{
    const char *charbuf = static_cast<const char *>(buf);
    size_t written = ringbuf->written;
    size_t tail = ringbuf->tail;
    size_t wrap = 0;

    if(m_runok == 1)
    return;

    if (tail <= written) {
        wrap = SHM_RING_BUFFER_SIZE;
    }
    if (tail - written + wrap < count) {
        std::cerr << "Operation ring buffer full! Dropping events." << std::endl;
        ringbuf->invalidateCommit = true;
        return;
    }

    size_t writeto = written + count;
    if (writeto >= SHM_RING_BUFFER_SIZE) {
        writeto -= SHM_RING_BUFFER_SIZE;
        size_t firstpart = SHM_RING_BUFFER_SIZE - written;
        memcpy(ringbuf->buf + written, charbuf, firstpart);
        memcpy(ringbuf->buf, charbuf + firstpart, writeto);
    } else {
        memcpy(ringbuf->buf + written, charbuf, count);
    }
    ringbuf->written = writeto;
   }

void RemotePluginClient::rdwr_writeOpcodering(RingBuffer *ringbuf, RemotePluginOpcode opcode, const char *file, int line)
{
rdwr_tryWritering(ringbuf, &opcode, sizeof(RemotePluginOpcode), file, line);
}

int RemotePluginClient::rdwr_readIntring(RingBuffer *ringbuf, const char *file, int line)
{
    int i = 0;
    rdwr_tryReadring(ringbuf, &i, sizeof(int), file, line);
    return i;
}

void RemotePluginClient::rdwr_writeIntring(RingBuffer *ringbuf, int i, const char *file, int line)
{
   rdwr_tryWritering(ringbuf, &i, sizeof(int), file, line);
}

void RemotePluginClient::rdwr_writeFloatring(RingBuffer *ringbuf, float f, const char *file, int line)
{
   rdwr_tryWritering(ringbuf, &f, sizeof(float), file, line);
}

float RemotePluginClient::rdwr_readFloatring(RingBuffer *ringbuf, const char *file, int line)
{
    float f = 0;
    rdwr_tryReadring(ringbuf, &f, sizeof(float), file, line);
    return f;
}

void RemotePluginClient::rdwr_commitWrite(RingBuffer *ringbuf, const char *file, int line)
{
    if (ringbuf->invalidateCommit) {
        ringbuf->written = ringbuf->head;
        ringbuf->invalidateCommit = false;
    } else {
        ringbuf->head = ringbuf->written;
    }
}

bool RemotePluginClient::dataAvailable(RingBuffer *ringbuf)
{
    return ringbuf->tail != ringbuf->head;
}

void RemotePluginClient::rdwr_writeOpcode(int fd, RemotePluginOpcode opcode, const char *file, int line)
{
    rdwr_tryWrite(fd, &opcode, sizeof(RemotePluginOpcode), file, line);
}    

void RemotePluginClient::rdwr_writeString(int fd, const std::string &str, const char *file, int line)
{
    int len = str.length();
    rdwr_tryWrite(fd, &len, sizeof(int), file, line);
    rdwr_tryWrite(fd, str.c_str(), len, file, line);
}

std::string RemotePluginClient::rdwr_readString(int fd, const char *file, int line)
{
    int len;
    static char *buf = 0;
    static int bufLen = 0;
    rdwr_tryRead(fd, &len, sizeof(int), file, line);
    if (len + 1 > bufLen) {
	delete buf;
	buf = new char[len + 1];
	bufLen = len + 1;
    }
    rdwr_tryRead(fd, buf, len, file, line);
    buf[len] = '\0';
    return std::string(buf);
}

void RemotePluginClient::rdwr_writeInt(int fd, int i, const char *file, int line)
{
    rdwr_tryWrite(fd, &i, sizeof(int), file, line);
}

int RemotePluginClient::rdwr_readInt(int fd, const char *file, int line)
{
    int i = 0;
    rdwr_tryRead(fd, &i, sizeof(int), file, line);
    return i;
}

void RemotePluginClient::rdwr_writeFloat(int fd, float f, const char *file, int line)
{
    rdwr_tryWrite(fd, &f, sizeof(float), file, line);
}

float RemotePluginClient::rdwr_readFloat(int fd, const char *file, int line)
{
    float f = 0;
    rdwr_tryRead(fd, &f, sizeof(float), file, line);
    return f;
}

#else

void RemotePluginClient::rdwr_tryWrite2(int fd, const void *buf, size_t count, const char *file, int line)
{
     ssize_t w = 0;

     if(m_runok == 1)
     return;
    
     while ((w = write(fd, buf, count)) < (ssize_t)count) {

     if (w < 0) {

        if (errno == EPIPE) 
        {
        if(m_finishaudio == 1)
        return;
        }
    
    	if (errno != EAGAIN) {
		char message[100];
		sprintf(message, "Write failed on fd %d at %s:%d", fd, file, line);
		perror(message);
                if(m_inexcept == 0)
		RemotePluginClosedException(); 
                break;
	    }
	    w = 0;
    }
    
    buf = (void *)(((char *)buf) + w);
	count -= w;
    }
}

void RemotePluginClient::rdwr_tryRead(int fd, void *buf, size_t count, const char *file, int line)
{
    ssize_t r = 0;

    if(m_runok == 1)
    return;

    while ((r = read(fd, buf, count)) < (ssize_t)count) {

	if (r == 0) {
	    // end of file

            if(m_finishaudio == 1)
            return;
            else 
            {
            if(m_inexcept == 0)
	    RemotePluginClosedException();
            break;
            }
	} else if (r < 0) {
	    if (errno != EAGAIN) {
		char message[100];
		sprintf(message, "Read failed on fd %d at %s:%d", fd, file, line);
		perror(message);
                if(m_inexcept == 0)
		RemotePluginClosedException();
                break;
	    }
	    r = 0;
	}

	buf = (void *)(((char *)buf) + r);
	count -= r;
    }
}

void RemotePluginClient::rdwr_tryWrite(int fd, const void *buf, size_t count, const char *file, int line)
{
        if(m_runok == 1)
        return;

        ssize_t w = write(fd, buf, count);

        if (w < 0) {
	char message[100];

        if (errno == EPIPE) 
        {
        if(m_finishaudio == 1)
        return;
        }
       
	sprintf(message, "Write failed on fd %d at %s:%d", fd, file, line);
	perror(message);
        if(m_inexcept == 0)
	RemotePluginClosedException();
        }

        if (w < (ssize_t)count) {
	fprintf(stderr, "Failed to complete write on fd %d (have %d, put %d) at %s:%d\n",
		fd, count, w, file, line);
        if(m_inexcept == 0)
	RemotePluginClosedException();
        }
}

void RemotePluginClient::rdwr_writeOpcode(int fd, RemotePluginOpcode opcode, const char *file, int line)
{
    rdwr_tryWrite(fd, &opcode, sizeof(RemotePluginOpcode), file, line);
}    

void RemotePluginClient::rdwr_writeString(int fd, const std::string &str, const char *file, int line)
{
    int len = str.length();
    rdwr_tryWrite(fd, &len, sizeof(int), file, line);
    rdwr_tryWrite(fd, str.c_str(), len, file, line);
}

std::string RemotePluginClient::rdwr_readString(int fd, const char *file, int line)
{
    int len;
    static char *buf = 0;
    static int bufLen = 0;
    rdwr_tryRead(fd, &len, sizeof(int), file, line);
    if (len + 1 > bufLen) {
	delete buf;
	buf = new char[len + 1];
	bufLen = len + 1;
    }
    rdwr_tryRead(fd, buf, len, file, line);
    buf[len] = '\0';
    return std::string(buf);
}

void RemotePluginClient::rdwr_writeInt(int fd, int i, const char *file, int line)
{
    rdwr_tryWrite(fd, &i, sizeof(int), file, line);
}

int RemotePluginClient::rdwr_readInt(int fd, const char *file, int line)
{
    int i = 0;
    rdwr_tryRead(fd, &i, sizeof(int), file, line);
    return i;
}

void RemotePluginClient::rdwr_writeFloat(int fd, float f, const char *file, int line)
{
    rdwr_tryWrite(fd, &f, sizeof(float), file, line);
}

float RemotePluginClient::rdwr_readFloat(int fd, const char *file, int line)
{
    float f = 0;
    rdwr_tryRead(fd, &f, sizeof(float), file, line);
    return f;
}

#endif

void RemotePluginClient::RemotePluginClosedException()
{

m_inexcept = 1;

 //   m_runok = 1;
#ifdef AMT
    m_threadbreak = 1;
 //   m_threadbreakexit = 1;
#endif
    m_finishaudio = 1;

    effVoidOp(effClose);
}

#ifdef RINGB

bool RemotePluginClient::fwait(int *futexp, int ms)
       {
       timespec timeval;
       int retval;

       if(ms > 0) {
		timeval.tv_sec  = ms / 1000;
		timeval.tv_nsec = (ms %= 1000) * 1000000;
	}

       for (;;) {                  
          retval = syscall(SYS_futex, futexp, FUTEX_WAIT, 0, &timeval, NULL, 0);
          if (retval == -1 && errno != EAGAIN)
          return true;

          if((*futexp != 0) && (__sync_val_compare_and_swap(futexp, *futexp, *futexp - 1) > 0))
          break;
          }
                               
          return false;          
       }

bool RemotePluginClient::fpost(int *futexp)
       {
       int retval;

	__sync_fetch_and_add(futexp, 1);

        retval = syscall(SYS_futex, futexp, FUTEX_WAKE, 1, NULL, NULL, 0);
/*
        if (retval  == -1)
        return true;
*/  
        return false;          
       }

#endif