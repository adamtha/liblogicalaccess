#include <logicalaccess/cards/IKSStorage.hpp>
#include <logicalaccess/dynlibrary/idynlibrary.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/readerproviders/readerconfiguration.hpp>
#include <logicalaccess/readerproviders/serialportdatatransport.hpp>
#include <logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand26format.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand37format.hpp>
#include <logicalaccess/services/storage/storagecardservice.hpp>
#include <logicalaccess/plugins/cards/desfire/nxpav2keydiversification.hpp>

#include <logicalaccess/plugins/cards/desfire/desfireev1chip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1location.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1commands.hpp>
#include <logicalaccess/plugins/cards/desfire/nxpav1keydiversification.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/desfireev1iso7816commands.hpp>
#include <logicalaccess/iks/IslogKeyServer.hpp>

#include <logicalaccess/plugins/lla-tests/macros.hpp>
#include <logicalaccess/plugins/lla-tests/utils.hpp>

void introduction() {
    PRINT_TIME("This test target DESFireEV1 cards. It tests that we are "
                       "can read data from a card w/o knowing the session key");

    PRINT_TIME("You will have 20 seconds to insert a card. Test log below");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
}

ByteVector vector_from_string(const std::string &s) {
    ByteVector ret(s.begin(), s.end());
    return ret;
}

int main(int ac, char **av) {
    using namespace logicalaccess;
    prologue(ac, av);
    introduction();
    ReaderProviderPtr provider;
    ReaderUnitPtr readerUnit;
    ChipPtr chip;
    tie(provider, readerUnit, chip) = lla_test_init();

    iks::IslogKeyServer::configureGlobalInstance("localhost",
                                                 6565,
                                                 "/home/xaqq/Documents/iks/crypto/MyClient1.pem",
                                                 "/home/xaqq/Documents/iks/crypto/MyClient1.key",
                                                 "/home/xaqq/Documents/iks/crypto/MyRootCA.pem");

    PRINT_TIME("Chip identifier: "
                       << logicalaccess::BufferHelper::getHex(chip->getChipIdentifier()));

    LLA_ASSERT(chip->getCardType() == "DESFireEV1",
               "Chip is not an DESFireEV1, but is " + chip->getCardType() + " instead.");

    auto storage =
            std::dynamic_pointer_cast<StorageCardService>(chip->getService(CST_STORAGE));

    auto cmd = std::dynamic_pointer_cast<DESFireISO7816Commands>(chip->getCommands());
    std::shared_ptr<DESFireEV1ISO7816Commands> cmdev1 =
            std::dynamic_pointer_cast<DESFireEV1ISO7816Commands>(chip->getCommands());

    std::shared_ptr<DESFireKey> key(new DESFireKey());
    key->setKeyType(DF_KEY_AES);

    //key->setKeyStorage(std::make_shared<IKSStorage>("00000000-0000-0000-0000-000000000000"));
    key->setKeyStorage(std::make_shared<IKSStorage>("ac915136-545a-4599-b2c5-41d136e29cc3"));
    //key->setKeyStorage(std::make_shared<IKSStorage>("44a68205-b486-4867-95d6-26f8c09d5729"));
    //key->fromString("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");

    cmd->selectApplication(0x000521);
    cmdev1->authenticate(0, key);

    auto ret = cmd->readData(1, 0, 4, logicalaccess::EncryptionMode::CM_ENCRYPT);
    std::cout << "Read data: " << ret << std::endl;

    return 0;
}
