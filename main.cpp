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

DWORD GetTrackCount(MCI_OPEN_PARMS *pMciOpenParms, MCI_STATUS_PARMS *pMciStatusParms) {
    pMciStatusParms->dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
    DWORD dwStatus = mciSendCommand(pMciOpenParms->wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)pMciStatusParms);
    return dwStatus;
}

DWORD GetTrackStart(MCI_OPEN_PARMS *pMciOpenParms, MCI_STATUS_PARMS *pMciStatusParms, DWORD dwTrack) {
    pMciStatusParms->dwItem = MCI_STATUS_POSITION;
    pMciStatusParms->dwTrack = dwTrack;
    return mciSendCommand(pMciOpenParms->wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK, (DWORD_PTR)pMciStatusParms);
}

DWORD Play(MCI_OPEN_PARMS *pMciOpenParms, MCI_PLAY_PARMS *pMciPlayParms, DWORD dwTrackStart) {
    pMciPlayParms->dwFrom = dwTrackStart;
    return mciSendCommand(pMciOpenParms->wDeviceID, MCI_PLAY, MCI_FROM, (DWORD_PTR)pMciPlayParms);
}

DWORD Stop(MCI_OPEN_PARMS *pMciOpenParms, MCI_GENERIC_PARMS *pMciGenericParms) {
    return mciSendCommand(pMciOpenParms->wDeviceID, MCI_STOP, 0, (DWORD_PTR)pMciGenericParms);
}

DWORD CloseCd(MCI_OPEN_PARMS *pMciOpenParms, MCI_GENERIC_PARMS *pMciGenericParms) {
    return mciSendCommand(pMciOpenParms->wDeviceID, MCI_CLOSE, 0, (DWORD_PTR)pMciGenericParms);
}

DWORD GetCurrentTrackNumber(MCI_OPEN_PARMS *pMciOpenParms, MCI_STATUS_PARMS *pMciStatusParms) {
    pMciStatusParms->dwItem = MCI_STATUS_CURRENT_TRACK;
    return mciSendCommand(pMciOpenParms->wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)pMciStatusParms);
}

int main() {
    MCI_OPEN_PARMS mciOpenParms;
    MCI_PLAY_PARMS mciPlayParms;
    MCI_STATUS_PARMS mciStatusParms;
    MCI_GENERIC_PARMS mciGenericParms;

    if (OpenCd(&mciOpenParms)) {
        std::cerr << "Could not open CD drive." << std::endl;
        return -1;
    }

    if (DWORD dwStatus = GetTrackCount(&mciOpenParms, &mciStatusParms)) {
        CloseCd(&mciOpenParms, &mciGenericParms);
        std::cerr << "The number of tracks could not be retrieved. Error code: " << dwStatus << std::endl;
        return -1;
    }
    DWORD numTracks = (DWORD) mciStatusParms.dwReturn;
    std::cout << "Track count: " << numTracks << std::endl;

    
    if (DWORD dwStatus = GetTrackStart(&mciOpenParms, &mciStatusParms, 1)) {
        CloseCd(&mciOpenParms, &mciGenericParms);
        std::cerr << "Could not get the start position of the track. Error code: " << dwStatus << std::endl;
        return -1;
    }
    DWORD dwTrackStart = (DWORD) mciStatusParms.dwReturn;

    if (DWORD dwStatus = Play(&mciOpenParms, &mciPlayParms, dwTrackStart)) {
        CloseCd(&mciOpenParms, &mciGenericParms);
        std::cerr << "The CD could not be played. Error Code: " << dwStatus << std::endl;
        return -1;
    }
    
    while (true) {
        std::string str;
        std::cin >> str;
        if (str == "q") {
            Stop(&mciOpenParms, &mciGenericParms);
            CloseCd(&mciOpenParms, &mciGenericParms);
            break;
        }
        else if (str[0] == 't') {
            int track = std::stoi(str.substr(1));

            Stop(&mciOpenParms, &mciGenericParms);
            if (DWORD dwStatus = GetTrackStart(&mciOpenParms, &mciStatusParms, track)) {
                CloseCd(&mciOpenParms, &mciGenericParms);
                std::cerr << "Could not get the start position of the track. Error code: " << dwStatus << std::endl;
                return -1;
            }
            DWORD dwTrackStart = (DWORD) mciStatusParms.dwReturn;
            if (DWORD dwStatus = Play(&mciOpenParms, &mciPlayParms, dwTrackStart)) {
                CloseCd(&mciOpenParms, &mciGenericParms);
                std::cerr << "#The CD could not be played. Error Code: " << dwStatus << std::endl;
                return -1;
            }
        }
        else if (str == "ct") {
            if (DWORD dwStatus = GetCurrentTrackNumber(&mciOpenParms, &mciStatusParms)) {
                std::cerr << "Could not get the current track number. Error code: " << dwStatus << std::endl;
                return -1;
            }
            DWORD dwCurrentTrack = (DWORD) mciStatusParms.dwReturn;
            std::cout << "Current track: " << dwCurrentTrack << std::endl;
        }
        
    }  
    return 0;
}
