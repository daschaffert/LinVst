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


#ifndef REMOTE_PLUGIN_CLIENT_H
#define REMOTE_PLUGIN_CLIENT_H

#define __cdecl

#include "pluginterfaces/vst2.x/aeffectx.h"

#include "remoteplugin.h"
#include "rdwrops.h"
#include <string>
#include <vector>
#include <sys/shm.h>

#define VSTSIZE 2048


// Any of the methods in this file, including constructors, should be
// considered capable of throwing RemotePluginClosedException.  Do not
// call anything here without catching it.

class RemotePluginClient
{
public:
                        RemotePluginClient(audioMasterCallback theMaster);
    virtual             ~RemotePluginClient();

    std::string         getFileIdentifiers();

    float               getVersion();
    int                 getUID();

    std::string         getName();
    std::string         getMaker();

    void                setBufferSize(int);
    void                setSampleRate(int);

    void                reset();
    void                terminate();

    int                 getInputCount();
    int                 getOutputCount();
    int                 getFlags();
    int                 getinitialDelay();

    int                 getParameterCount();
    std::string         getParameterName(int);
    void                setParameter(int, float);
    float               getParameter(int);
    float               getParameterDefault(int);
    void                getParameters(int, int, float *);

    int                 getProgramCount();
    std::string         getProgramNameIndexed(int);
    std::string         getProgramName();
    void                setCurrentProgram(int);

    int                 processVstEvents(VstEvents *);
    int                 getChunk(void **ptr, int bank_prog);
    int                 setChunk(void *ptr, int sz, int bank_prog);
    // int                 canBeAutomated(int param);
    int                 getProgram();
    int                 EffectOpen();

    // void                effMainsChanged(int s);
    // int                 getUniqueID();

    // Either inputs or outputs may be NULL if (and only if) there are none
    void                process(float **inputs, float **outputs, int sampleFrames);

#ifdef RINGB
    void                waitForServer();
    void                waitForServer2();
#endif

    void                setDebugLevel(RemotePluginDebugLevel);
    bool                warn(std::string);

    void                showGUI();
    void                hideGUI();

#ifdef EMBED
    void                openGUI();
#endif

    int                 getEffInt(int opcode);
    void                getEffString(int opcode, int index, char *ptr, int len);
    void                effVoidOp(int opcode);

    int                 m_bufferSize;
    int                 m_numInputs;
    int                 m_numOutputs;
    int                 m_finishaudio;
    int                 m_runok;
    AEffect             *theEffect;

#ifdef AMT
    int                 m_threadbreak;
    int                 m_threadbreakexit;
    int                 m_updateio;
    VstEvents           vstev[VSTSIZE];
#endif

#ifdef EMBED
    struct winmessage
    {
        int handle;
        int width;
        int height;
    } winm;
#endif

char *m_shm3;

int                     m_386run;

int                     m_inexcept;

protected:
    void                cleanup();
    void                syncStartup();

private:
    int                 m_controlRequestFd;
    int                 m_controlResponseFd;
    int                 m_parRequestFd;
    int                 m_parResponseFd;
#ifndef RINGB
    int                 m_processFd;
    int                 m_processResponseFd;
#endif
    int                 m_shmFd;
    int                 m_shmFd2;
    int                 m_shmFd3;
    char                *m_controlRequestFileName;
    char                *m_controlResponseFileName;
    char                *m_parRequestFileName;
    char                *m_parResponseFileName;
#ifndef RINGB
    char                *m_processFileName;
    char                *m_processResponseFileName;
#endif
#ifdef RINGB
    int                 m_shmControlFd;
    char                *m_shmControlFileName;
    ShmControl          *m_shmControl;
    int                 m_shmControl2Fd;
    char                *m_shmControl2FileName;
    ShmControl          *m_shmControl2;
    int                 m_shmControl3Fd;
    char                *m_shmControl3FileName;
    ShmControl          *m_shmControl3;
#endif

    char                *m_shmFileName;
    char                *m_shm;
    size_t              m_shmSize;

    char                *m_shmFileName2;
    char                *m_shm2;
    size_t              m_shmSize2;

    char                *m_shmFileName3;
    size_t              m_shmSize3;

    int                 sizeShm();

#ifdef AMT
    pthread_t           m_AMThread;
    static void         *callAMThread(void *arg) { return ((RemotePluginClient*)arg)->AMThread(); }
    void                *AMThread();
    audioMasterCallback m_audioMaster;
#endif

void RemotePluginClosedException();

#ifdef RINGB

bool fwait(int *fcount, int ms);
bool fpost(int *fcount);

void rdwr_tryWrite2(int fd, const void *buf, size_t count, const char *file, int line);
void rdwr_tryRead(int fd, void *buf, size_t count, const char *file, int line);
void rdwr_tryWrite(int fd, const void *buf, size_t count, const char *file, int line);
void rdwr_tryReadring(RingBuffer *ringbuf, void *buf, size_t count, const char *file, int line);
void rdwr_tryWritering(RingBuffer *ringbuf, const void *buf, size_t count, const char *file, int line);
void rdwr_writeOpcodering(RingBuffer *ringbuf, RemotePluginOpcode opcode, const char *file, int line);
int rdwr_readIntring(RingBuffer *ringbuf, const char *file, int line);
void rdwr_writeIntring(RingBuffer *ringbuf, int i, const char *file, int line);
void rdwr_writeFloatring(RingBuffer *ringbuf, float f, const char *file, int line);
float rdwr_readFloatring(RingBuffer *ringbuf, const char *file, int line);
void rdwr_writeOpcode(int fd, RemotePluginOpcode opcode, const char *file, int line);
void rdwr_writeString(int fd, const std::string &str, const char *file, int line);
std::string rdwr_readString(int fd, const char *file, int line);
void rdwr_writeInt(int fd, int i, const char *file, int line);
int rdwr_readInt(int fd, const char *file, int line);
void rdwr_writeFloat(int fd, float f, const char *file, int line);
float rdwr_readFloat(int fd, const char *file, int line);

void rdwr_commitWrite(RingBuffer *ringbuf, const char *file, int line);
bool dataAvailable(RingBuffer *ringbuf);

#else
void rdwr_tryWrite2(int fd, const void *buf, size_t count, const char *file, int line);
void rdwr_tryRead(int fd, void *buf, size_t count, const char *file, int line);
void rdwr_tryWrite(int fd, const void *buf, size_t count, const char *file, int line);
void rdwr_writeOpcode(int fd, RemotePluginOpcode opcode, const char *file, int line);
void rdwr_writeString(int fd, const std::string &str, const char *file, int line);
std::string rdwr_readString(int fd, const char *file, int line);
void rdwr_writeInt(int fd, int i, const char *file, int line);
int rdwr_readInt(int fd, const char *file, int line);
void rdwr_writeFloat(int fd, float f, const char *file, int line);
float rdwr_readFloat(int fd, const char *file, int line);
#endif
};
#endif
