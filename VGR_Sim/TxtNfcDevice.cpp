#include "TxtNfcDevice.h"

namespace ft {

TxtNfcDevice::TxtNfcDevice() : nfcData(new TxtNfcData()) {}
TxtNfcDevice::~TxtNfcDevice() { delete nfcData; }

std::string TxtNfcDevice::readTagsGetUID() {
    // In sim, return the tag set by main.cpp if not empty
    if(simTag.tag_uid.empty()) return "";
    return simTag.tag_uid;
}

bool TxtNfcDevice::eraseTags() {
    std::cout << "[NFC] Erasing Tags...\n";
    simTag = TxtWorkpiece();
    return true;
}

std::string TxtNfcDevice::readTags() {
    std::cout << "[NFC] Reading Tags: " << simTag.tag_uid << "\n";
    return simTag.tag_uid;
}

bool TxtNfcDevice::writeTags(TxtWorkpiece wp, std::vector<uTS> vuts, uint8_t mask_ts) {
    std::cout << "[NFC] Writing Tag: " << wp.tag_uid << " Type: " << wp.type << "\n";
    simTag = wp;
    return true;
}

}