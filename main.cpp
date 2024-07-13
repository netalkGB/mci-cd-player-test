#include  <windows.h>
#include <iostream>
#include <string>

#define DRIVE_LETTER L"D:"

#pragma comment(lib, "winmm.lib")


DWORD OpenCd(MCI_OPEN_PARMS *pMciOpenParms) {
    pMciOpenParms->lpstrDeviceType = L"cdaudio";
    pMciOpenParms->lpstrElementName =  DRIVE_LETTER;
    return mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD_PTR)pMciOpenParms);
}

DWORD GetTrackCount(MCIDEVICEID wDeviceID, MCI_STATUS_PARMS *pMciStatusParms) {
    pMciStatusParms->dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
    DWORD dwStatus = mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)pMciStatusParms);
    return dwStatus;
}

DWORD GetTrackStart(MCIDEVICEID wDeviceID, MCI_STATUS_PARMS *pMciStatusParms, DWORD dwTrack) {
    pMciStatusParms->dwItem = MCI_STATUS_POSITION;
    pMciStatusParms->dwTrack = dwTrack;
    return mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK, (DWORD_PTR)pMciStatusParms);
}

DWORD Play(MCIDEVICEID wDeviceID, MCI_PLAY_PARMS *pMciPlayParms, DWORD dwTrackStart) {
    pMciPlayParms->dwFrom = dwTrackStart;
    return mciSendCommand(wDeviceID, MCI_PLAY, MCI_FROM, (DWORD_PTR)pMciPlayParms);
}

DWORD Stop(MCIDEVICEID wDeviceID, MCI_GENERIC_PARMS *pMciGenericParms) {
    return mciSendCommand(wDeviceID, MCI_STOP, 0, (DWORD_PTR)pMciGenericParms);
}

DWORD Pause(MCIDEVICEID wDeviceID) {
    return mciSendCommand(wDeviceID, MCI_PAUSE, 0, 0);
}

DWORD Resume(MCIDEVICEID wDeviceID) {
    return mciSendCommand(wDeviceID, MCI_RESUME, 0, 0);
}

DWORD CloseCd(MCIDEVICEID wDeviceID, MCI_GENERIC_PARMS *pMciGenericParms) {
    return mciSendCommand(wDeviceID, MCI_CLOSE, 0, (DWORD_PTR)pMciGenericParms);
}

DWORD GetCurrentTrackNumber(MCIDEVICEID wDeviceID, MCI_STATUS_PARMS *pMciStatusParms) {
    pMciStatusParms->dwItem = MCI_STATUS_CURRENT_TRACK;
    return mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)pMciStatusParms);
}

DWORD GetTrackLength(MCIDEVICEID wDeviceID, MCI_STATUS_PARMS *pMciStatusParms, DWORD dwTrack) {
    pMciStatusParms->dwItem = MCI_STATUS_LENGTH;
    pMciStatusParms->dwTrack = dwTrack;
    return mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK, (DWORD_PTR)pMciStatusParms);
}

DWORD GetCurrentPosition(MCIDEVICEID wDeviceID, MCI_STATUS_PARMS *pMciStatusParms) {
    pMciStatusParms->dwItem = MCI_STATUS_POSITION;
    return mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)pMciStatusParms);
}

DWORD ConvertToMilliseconds(DWORD time) {
    DWORD minutes = MCI_MSF_MINUTE(time);
    DWORD seconds = MCI_MSF_SECOND(time);
    DWORD frames = MCI_MSF_FRAME(time);
    return (minutes * 60000) + (seconds * 1000) + (frames * (1000 / 75));
}

int main() {
    MCI_STATUS_PARMS mciStatusParms;
    MCI_GENERIC_PARMS mciGenericParms;
    MCI_OPEN_PARMS mciOpenParms;


    if (OpenCd(&mciOpenParms)) {
        std::cerr << "Could not open CD drive." << std::endl;
        return -1;
    }
    MCIDEVICEID wDeviceID = mciOpenParms.wDeviceID;

    if (DWORD dwStatus = GetTrackCount(wDeviceID, &mciStatusParms)) {
        CloseCd(wDeviceID, &mciGenericParms);
        std::cerr << "The number of tracks could not be retrieved. Error code: " << dwStatus << std::endl;
        return -1;
    }
    DWORD numTracks = (DWORD) mciStatusParms.dwReturn;
    std::cout << "Track count: " << numTracks << std::endl;

    
    if (DWORD dwStatus = GetTrackStart(wDeviceID, &mciStatusParms, 1)) {
        CloseCd(wDeviceID, &mciGenericParms);
        std::cerr << "Could not get the start position of the track. Error code: " << dwStatus << std::endl;
        return -1;
    }
    DWORD dwTrackStart = (DWORD) mciStatusParms.dwReturn;

    MCI_PLAY_PARMS mciPlayParms;
    if (DWORD dwStatus = Play(wDeviceID, &mciPlayParms, dwTrackStart)) {
        CloseCd(wDeviceID, &mciGenericParms);
        std::cerr << "The CD could not be played. Error Code: " << dwStatus << std::endl;
        return -1;
    }
    
    while (true) {
        std::string str;
        std::cin >> str;
        if (str == "q") {
            Stop(wDeviceID, &mciGenericParms);
            CloseCd(wDeviceID, &mciGenericParms);
            break;
        } else if (str[0] == 't') {
            int track = std::stoi(str.substr(1));

            Stop(wDeviceID, &mciGenericParms);
            if (DWORD dwStatus = GetTrackStart(wDeviceID, &mciStatusParms, track)) {
                CloseCd(wDeviceID, &mciGenericParms);
                std::cerr << "Could not get the start position of the track. Error code: " << dwStatus << std::endl;
                return -1;
            }
            DWORD dwTrackStart = (DWORD) mciStatusParms.dwReturn;
            MCI_PLAY_PARMS mciPlayParms;
            if (DWORD dwStatus = Play(wDeviceID, &mciPlayParms, dwTrackStart)) {
                CloseCd(wDeviceID, &mciGenericParms);
                std::cerr << "#The CD could not be played. Error Code: " << dwStatus << std::endl;
                return -1;
            }
        } else if (str == "ct") {
            if (DWORD dwStatus = GetCurrentTrackNumber(wDeviceID, &mciStatusParms)) {
                std::cerr << "Could not get the current track number. Error code: " << dwStatus << std::endl;
                return -1;
            }
            DWORD dwCurrentTrack = (DWORD) mciStatusParms.dwReturn;
            if (DWORD dwStatus = GetCurrentPosition(wDeviceID, &mciStatusParms)) {
                std::cerr << "Could not get the current position. Error code: " << dwStatus << std::endl;
                return -1;
            }
            DWORD dwCurrentPosition = (DWORD)mciStatusParms.dwReturn;
            if (DWORD dwStatus = GetTrackLength(wDeviceID, &mciStatusParms, dwCurrentTrack)) {
                std::cerr << "Could not get the track length. Error code: " << dwStatus << std::endl;
                return -1;
            }
            DWORD dwTrackLength = (DWORD)mciStatusParms.dwReturn;
            std::cout << "Track " << dwCurrentTrack << ": " << ConvertToMilliseconds(dwCurrentPosition) << " ms / " << ConvertToMilliseconds(dwTrackLength) << " ms" << std::endl;

        } else if (str == "p") {
            if (DWORD dwStatus = Pause(wDeviceID)) {
				std::cerr << "Could not pause the CD. Error code: " << dwStatus << std::endl;
				return -1;
			}
        } else if (str == "cp") {
            if (DWORD dwStatus = Resume(wDeviceID)) {
               std::cerr << "Could not resume the CD. Error code: " << dwStatus << std::endl;
               return -1;
            }
        }
        
    }  
    return 0;
}
