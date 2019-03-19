/*
 *      Copyright (C) 2018 Gonzalo Vega
 *      https://github.com/gonzalo-hvega/xbmc-pvr-iptvsimple/
 *
 *      Copyright (C) 2013-2015 Anton Fedchin
 *      http://github.com/afedchin/xbmc-addon-iptvsimple/
 *
 *      Copyright (C) 2011 Pulse-Eight
 *      http://www.pulse-eight.com/
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "client.h"
#include "PVRIptvData.h"
#include "PVRDvrData.h"
#include "PVRUtils.h"
#include "xbmc_pvr_dll.h"
#include "p8-platform/util/util.h"

using namespace ADDON;

#ifdef TARGET_WINDOWS
#define snprintf _snprintf
#ifdef CreateDirectory
#undef CreateDirectory
#endif
#ifdef DeleteFile
#undef DeleteFile
#endif
#endif

bool m_bCreated = false;
ADDON_STATUS m_CurStatus = ADDON_STATUS_UNKNOWN;
PVRDvrData *m_dvr = NULL;
bool m_bIsPlaying = false;
PVRIptvData *m_data = NULL;
PVRIptvChannel m_currentChannel;

/* User adjustable settings are saved here.
 * Default values are defined inside client.h
 * and exported to the other source files.
 */
std::string g_strUserPath = "";
std::string g_strClientPath = "";

CHelper_libXBMC_addon *XBMC = NULL;
CHelper_libXBMC_pvr *PVR = NULL;
CHelper_libKODI_guilib *GUI = NULL;

std::string g_strTvgPath = "";
std::string g_strM3UPath = "";
std::string g_strLogoPath = "";
int g_iEPGTimeShift = 0;
int g_iStartNumber = 1;
bool g_bTSOverride = true;
bool g_bCacheM3U = false;
bool g_bCacheEPG = false;
int g_iEPGLogos = 0;
std::string g_strRecPath = "";
std::string g_strFFMPEG = "";
std::string g_strAVParams = "";
std::string g_strFFPROBE = "";
std::string g_strFileExt = "";
int g_iStrmTimeout = 60;
int g_iStrmQuality = 1;
std::string g_strSmbPath = "";
std::string g_strSmbMount = "";
std::string g_strSmbUnmount = "";

extern std::string PathCombine(const std::string &strPath, const std::string &strFileName) {
    std::string strResult = strPath;
    if (strResult.at(strResult.size() - 1) == '\\' ||
        strResult.at(strResult.size() - 1) == '/') {
        strResult.append(strFileName);
    } else {
        strResult.append("/");
        strResult.append(strFileName);
    }

    return strResult;
}

extern std::string GetClientFilePath(const std::string &strFileName) {
    return PathCombine(g_strClientPath, strFileName);
}

extern std::string GetUserFilePath(const std::string &strFileName) {
    return PathCombine(g_strUserPath, strFileName);
}

extern "C" {

void ADDON_ReadSettings(void) {
    char buffer[1024];
    int iPathType = 0;
    if (!XBMC->GetSetting("m3uPathType", &iPathType)) {
        iPathType = 1;
    }
    if (iPathType) {
        if (XBMC->GetSetting("m3uUrl", &buffer)) {
            g_strM3UPath = buffer;
        }
        if (!XBMC->GetSetting("m3uCache", &g_bCacheM3U)) {
            g_bCacheM3U = true;
        }
    } else {
        if (XBMC->GetSetting("m3uPath", &buffer)) {
            g_strM3UPath = buffer;
        }
        g_bCacheM3U = false;
    }
    if (!XBMC->GetSetting("startNum", &g_iStartNumber)) {
        g_iStartNumber = 1;
    }
    if (!XBMC->GetSetting("epgPathType", &iPathType)) {
        iPathType = 1;
    }
    if (iPathType) {
        if (XBMC->GetSetting("epgUrl", &buffer)) {
            g_strTvgPath = buffer;
        }
        if (!XBMC->GetSetting("epgCache", &g_bCacheEPG)) {
            g_bCacheEPG = true;
        }
    } else {
        if (XBMC->GetSetting("epgPath", &buffer)) {
            g_strTvgPath = buffer;
        }
        g_bCacheEPG = false;
    }
    float fShift;
    if (XBMC->GetSetting("epgTimeShift", &fShift)) {
        g_iEPGTimeShift = (int) (fShift * 3600.0); // hours to seconds
    }
    if (!XBMC->GetSetting("epgTSOverride", &g_bTSOverride)) {
        g_bTSOverride = true;
    }
    if (!XBMC->GetSetting("logoPathType", &iPathType)) {
        iPathType = 1;
    }
    if (XBMC->GetSetting(iPathType ? "logoBaseUrl" : "logoPath", &buffer)) {
        g_strLogoPath = buffer;
    }

    // Logos from EPG
    if (!XBMC->GetSetting("logoFromEpg", &g_iEPGLogos))
        g_iEPGLogos = 0;

    // Recording settings
    if (XBMC->GetSetting("recordingsPath", &buffer)) {
        g_strRecPath = buffer;
    }

    if (XBMC->GetSetting("ffmpegPath", &buffer)) {
        g_strFFMPEG = buffer;
    }

    if (XBMC->GetSetting("ffmpegParams", &buffer)) {
        g_strAVParams = buffer;
    }

    if (XBMC->GetSetting("ffprobePath", &buffer)) {
        g_strFFPROBE = buffer;
    }

    if (XBMC->GetSetting("fileExtension", &buffer)) {
        g_strFileExt = buffer;
    }

    int streamTimeout;
    if (XBMC->GetSetting("streamTimeout", &streamTimeout)) {
        g_iStrmTimeout = streamTimeout;
    }

    int streamQuality;
    if (XBMC->GetSetting("streamQuality", &streamQuality)) {
        g_iStrmQuality = streamQuality;
    }

    if (XBMC->GetSetting("sambaPath", &buffer)) {
        g_strSmbPath = buffer;
    }

    if (XBMC->GetSetting("sambaMount", &buffer)) {
        g_strSmbMount = buffer;
    }

    if (XBMC->GetSetting("sambaUnmount", &buffer)) {
        g_strSmbUnmount = buffer;
    }
}

ADDON_STATUS ADDON_Create(void *hdl, void *props) {
    if (!hdl || !props) {
        return ADDON_STATUS_UNKNOWN;
    }

    PVR_PROPERTIES *pvrprops = (PVR_PROPERTIES *) props;

    XBMC = new CHelper_libXBMC_addon;
    if (!XBMC->RegisterMe(hdl)) {
        SAFE_DELETE(XBMC);
        return ADDON_STATUS_PERMANENT_FAILURE;
    }

    PVR = new CHelper_libXBMC_pvr;
    if (!PVR->RegisterMe(hdl)) {
        SAFE_DELETE(PVR);
        SAFE_DELETE(XBMC);
        return ADDON_STATUS_PERMANENT_FAILURE;
    }

    GUI = new CHelper_libKODI_guilib;
    if (!GUI->RegisterMe(hdl)) {
        SAFE_DELETE(GUI);
        SAFE_DELETE(PVR);
        SAFE_DELETE(XBMC);
        return ADDON_STATUS_PERMANENT_FAILURE;
    }

    XBMC->Log(LOG_DEBUG, "%s - Creating the PVR IPTV Simple add-on", __FUNCTION__);

    m_CurStatus = ADDON_STATUS_UNKNOWN;
    g_strUserPath = pvrprops->strUserPath;
    g_strClientPath = pvrprops->strClientPath;

    if (!XBMC->DirectoryExists(g_strUserPath.c_str())) {
        XBMC->CreateDirectory(g_strUserPath.c_str());
    }

    ADDON_ReadSettings();

    m_data = new PVRIptvData;
    m_dvr = new PVRDvrData;
    m_CurStatus = ADDON_STATUS_OK;
    m_bCreated = true;

    return m_CurStatus;
}

ADDON_STATUS ADDON_GetStatus() {
    return m_CurStatus;
}

void ADDON_Destroy() {
    CloseThreads();

    delete m_dvr;
    delete m_data;

    m_bCreated = false;
    m_CurStatus = ADDON_STATUS_UNKNOWN;
}

ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue) {
    // reset cache and restart addon

    std::string strFile = GetUserFilePath(M3U_FILE_NAME);
    if (XBMC->FileExists(strFile.c_str(), false)) {
        XBMC->DeleteFile(strFile.c_str());
    }

    strFile = GetUserFilePath(TVG_FILE_NAME);
    if (XBMC->FileExists(strFile.c_str(), false)) {
        XBMC->DeleteFile(strFile.c_str());
    }

    return ADDON_STATUS_NEED_RESTART;
}

/***********************************************************
 * PVR Client AddOn specific public library functions
 ***********************************************************/

void OnSystemSleep() {
}

void OnSystemWake() {
}

void OnPowerSavingActivated() {
}

void OnPowerSavingDeactivated() {
}

PVR_ERROR GetAddonCapabilities(PVR_ADDON_CAPABILITIES *pCapabilities) {
    pCapabilities->bSupportsEPG = true;
    pCapabilities->bSupportsTV = true;
    pCapabilities->bSupportsRadio = true;
    pCapabilities->bSupportsChannelGroups = true;
    pCapabilities->bSupportsRecordings = true;
    pCapabilities->bSupportsTimers = true;
    pCapabilities->bSupportsRecordingPlayCount = true;
    pCapabilities->bSupportsLastPlayedPosition = true;
    pCapabilities->bSupportsRecordingsRename = false;
    pCapabilities->bSupportsRecordingsLifetimeChange = false;
    pCapabilities->bSupportsDescrambleInfo = false;

    return PVR_ERROR_NO_ERROR;
}

const char *GetBackendName(void) {
    static const char *strBackendName = "IPTV Simple PVR Add-on";
    return strBackendName;
}

const char *GetBackendVersion(void) {
    static std::string strBackendVersion = STR(IPTV_VERSION);
    return strBackendVersion.c_str();
}

const char *GetConnectionString(void) {
    static std::string strConnectionString = "Connected";
    return strConnectionString.c_str();
}

const char *GetBackendHostname(void) {
    return "";
}

PVR_ERROR GetDriveSpace(long long *iTotal, long long *iUsed) {
    *iTotal = 0;
    *iUsed = 0;
    return PVR_ERROR_NO_ERROR;
}

PVR_ERROR GetEPGForChannel(ADDON_HANDLE handle, const PVR_CHANNEL &channel, time_t iStart, time_t iEnd) {
    if (m_data)
        return m_data->GetEPGForChannel(handle, channel, iStart, iEnd);

    return PVR_ERROR_SERVER_ERROR;
}

int GetChannelsAmount(void) {
    if (m_data)
        return m_data->GetChannelsAmount();

    return -1;
}

PVR_ERROR GetChannels(ADDON_HANDLE handle, bool bRadio) {
    if (m_data)
        return m_data->GetChannels(handle, bRadio);

    return PVR_ERROR_SERVER_ERROR;
}

PVR_ERROR
GetChannelStreamProperties(const PVR_CHANNEL *channel, PVR_NAMED_VALUE *properties, unsigned int *iPropertiesCount) {
    if (!channel || !properties || !iPropertiesCount)
        return PVR_ERROR_SERVER_ERROR;

    if (*iPropertiesCount < 1)
        return PVR_ERROR_INVALID_PARAMETERS;

    if (m_data && m_data->GetChannel(*channel, m_currentChannel)) {
        strncpy(properties[0].strName, PVR_STREAM_PROPERTY_STREAMURL, sizeof(properties[0].strName) - 1);
        strncpy(properties[0].strValue, m_currentChannel.strStreamURL.c_str(), sizeof(properties[0].strValue) - 1);
        *iPropertiesCount = 1;
        if (!m_currentChannel.properties.empty()) {
            for (auto &prop : m_currentChannel.properties) {
                strncpy(properties[*iPropertiesCount].strName, prop.first.c_str(),
                        sizeof(properties[*iPropertiesCount].strName) - 1);
                strncpy(properties[*iPropertiesCount].strValue, prop.second.c_str(),
                        sizeof(properties[*iPropertiesCount].strName) - 1);
                (*iPropertiesCount)++;
            }
        }
        return PVR_ERROR_NO_ERROR;
    }

    return PVR_ERROR_SERVER_ERROR;
}

int GetChannelGroupsAmount(void) {
    if (m_data)
        return m_data->GetChannelGroupsAmount();

    return -1;
}

PVR_ERROR GetChannelGroups(ADDON_HANDLE handle, bool bRadio) {
    if (m_data)
        return m_data->GetChannelGroups(handle, bRadio);

    return PVR_ERROR_SERVER_ERROR;
}

PVR_ERROR GetChannelGroupMembers(ADDON_HANDLE handle, const PVR_CHANNEL_GROUP &group) {
    if (m_data)
        return m_data->GetChannelGroupMembers(handle, group);

    return PVR_ERROR_SERVER_ERROR;
}

PVR_ERROR SignalStatus(PVR_SIGNAL_STATUS &signalStatus) {
    snprintf(signalStatus.strAdapterName, sizeof(signalStatus.strAdapterName), "IPTV Simple Adapter 1");
    snprintf(signalStatus.strAdapterStatus, sizeof(signalStatus.strAdapterStatus), "OK");

    return PVR_ERROR_NO_ERROR;
}

PVR_ERROR AddTimer(const PVR_TIMER &timer) {
    if (m_dvr)
        return m_dvr->AddTimer(timer);

    return PVR_ERROR_SERVER_ERROR;
}

PVR_ERROR DeleteTimer(const PVR_TIMER &timer, bool bForceDelete) {
    if (m_dvr)
        return m_dvr->DeleteTimer(timer, bForceDelete);

    return PVR_ERROR_SERVER_ERROR;
}

PVR_ERROR UpdateTimer(const PVR_TIMER &timer) {
    if (m_dvr)
        return m_dvr->UpdateTimer(timer);

    return PVR_ERROR_SERVER_ERROR;
}

PVR_ERROR GetTimers(ADDON_HANDLE handle) {
    if (m_dvr)
        return m_dvr->GetTimers(handle);

    return PVR_ERROR_SERVER_ERROR;
}

int GetTimersAmount(void) {
    if (m_dvr)
        return m_dvr->GetTimersAmount();

    return -1;
}

PVR_ERROR GetRecordings(ADDON_HANDLE handle, bool deleted) {
    if (m_dvr)
        return m_dvr->GetRecordings(handle);

    return PVR_ERROR_SERVER_ERROR;
}

int GetRecordingsAmount(bool deleted) {
    if (m_dvr)
        return m_dvr->GetRecordingsAmount();

    return -1;
}

PVR_ERROR DeleteRecording(const PVR_RECORDING &recording) {
    if (m_dvr)
        return m_dvr->DeleteRecording(recording);

    return PVR_ERROR_SERVER_ERROR;
}

bool OpenRecordedStream(const PVR_RECORDING &recording) {
    if (m_dvr)
        return m_dvr->OpenRecordedStream(recording);

    return false;
}

void CloseRecordedStream(void) {
    if (m_dvr)
        m_dvr->CloseRecordedStream();
}

int ReadRecordedStream(unsigned char *pBuffer, unsigned int iBufferSize) {
    if (m_dvr)
        return m_dvr->ReadRecordedStream(pBuffer, iBufferSize);

    return 0;
}

long long SeekRecordedStream(long long iPosition, int iWhence /* = SEEK_SET */) {
    if (m_dvr)
        return m_dvr->SeekRecordedStream(iPosition, iWhence);

    return 0;
}

long long PositionRecordedStream(void) {
    if (m_dvr)
        return m_dvr->GetReaderPosition();

    return -1;
}

long long LengthRecordedStream(void) {
    if (m_dvr)
        return m_dvr->GetReaderLength();

    return 0;
}

PVR_ERROR SetRecordingLastPlayedPosition(const PVR_RECORDING &recording, int lastplayedposition) {
    if (m_dvr)
        return m_dvr->SetRecordingLastPlayedPosition(recording, lastplayedposition);

    return PVR_ERROR_SERVER_ERROR;
}

int GetRecordingLastPlayedPosition(const PVR_RECORDING &recording) {
    if (m_dvr)
        return m_dvr->GetRecordingLastPlayedPosition(recording);

    return -1;
}

PVR_ERROR SetRecordingPlayCount(const PVR_RECORDING &recording, int count) {
    if (m_dvr)
        return m_dvr->SetRecordingPlayCount(recording, count);

    return PVR_ERROR_SERVER_ERROR;
}

/** UNUSED API FUNCTIONS */
bool CanPauseStream(void) { return false; }
PVR_ERROR GetRecordingStreamProperties(const PVR_RECORDING *, PVR_NAMED_VALUE *,
                                       unsigned int *) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR OpenDialogChannelScan(void) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR
CallMenuHook(const PVR_MENUHOOK &menuhook, const PVR_MENUHOOK_DATA &item) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR DeleteChannel(const PVR_CHANNEL &channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR RenameChannel(const PVR_CHANNEL &channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR OpenDialogChannelSettings(const PVR_CHANNEL &channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR OpenDialogChannelAdd(const PVR_CHANNEL &channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
void CloseLiveStream(void) {}
bool OpenLiveStream(const PVR_CHANNEL &channel) { return false; }
void DemuxReset(void) {}
void DemuxFlush(void) {}
int ReadLiveStream(unsigned char *pBuffer, unsigned int iBufferSize) { return 0; }
long long SeekLiveStream(long long iPosition, int iWhence /* = SEEK_SET */) { return -1; }
long long LengthLiveStream(void) { return -1; }
PVR_ERROR RenameRecording(const PVR_RECORDING &recording) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR GetRecordingEdl(const PVR_RECORDING &, PVR_EDL_ENTRY[], int *) { return PVR_ERROR_NOT_IMPLEMENTED; } ;
PVR_ERROR GetTimerTypes(PVR_TIMER_TYPE types[], int *size) { return PVR_ERROR_NOT_IMPLEMENTED; }
void DemuxAbort(void) {}
DemuxPacket *DemuxRead(void) { return NULL; }
bool IsTimeshifting(void) { return false; }
bool IsRealTimeStream(void) { return true; }
void PauseStream(bool bPaused) {}
bool CanSeekStream(void) { return false; }
bool SeekTime(double, bool, double *) { return false; }
void SetSpeed(int) {} ;
PVR_ERROR UndeleteRecording(const PVR_RECORDING &recording) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR DeleteAllRecordingsFromTrash() { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR SetEPGTimeFrame(int) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR GetDescrambleInfo(PVR_DESCRAMBLE_INFO *) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR SetRecordingLifetime(const PVR_RECORDING *) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR GetStreamTimes(PVR_STREAM_TIMES *) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR GetStreamProperties(PVR_STREAM_PROPERTIES *) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR IsEPGTagRecordable(const EPG_TAG *, bool *) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR IsEPGTagPlayable(const EPG_TAG *, bool *) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR
GetEPGTagStreamProperties(const EPG_TAG *, PVR_NAMED_VALUE *, unsigned int *) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR GetEPGTagEdl(const EPG_TAG *epgTag, PVR_EDL_ENTRY edl[], int *size) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR GetStreamReadChunkSize(int *chunksize) { return PVR_ERROR_NOT_IMPLEMENTED; }

} // extern "C"
