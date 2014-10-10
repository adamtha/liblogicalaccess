/**
 * \file mifareultralightcomnikeyxx21commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C - Omnikey xx21.
 */

#include "../commands/mifareultralightcomnikeyxx21commands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"

namespace logicalaccess
{
    MifareUltralightCOmnikeyXX21Commands::MifareUltralightCOmnikeyXX21Commands()
        : MifareUltralightCPCSCCommands()
    {
    }

    MifareUltralightCOmnikeyXX21Commands::~MifareUltralightCOmnikeyXX21Commands()
    {
    }

    std::vector<unsigned char> MifareUltralightCOmnikeyXX21Commands::sendGenericCommand(const std::vector<unsigned char>& data)
    {
        std::vector<unsigned char> wdata;
        wdata.push_back(0x01);
        wdata.push_back(0x00);
        wdata.push_back(0xF3);
        wdata.push_back(0x00);
        wdata.push_back(0x00);
        wdata.push_back(0x64);
        wdata.insert(wdata.end(), data.begin(), data.end());

        return getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xA0, 0x00, 0x05, static_cast<unsigned char>(wdata.size()), wdata);
    }
}