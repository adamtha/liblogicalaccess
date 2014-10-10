/**
 * \file pcscreaderunit.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC Reader unit.
 */

#ifndef LOGICALACCESS_PCSCREADERUNIT_HPP
#define LOGICALACCESS_PCSCREADERUNIT_HPP

#include "logicalaccess/readerproviders/readerunit.hpp"
#include "pcscreaderunitconfiguration.hpp"
#include "../iso7816/iso7816readerunit.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"

namespace logicalaccess
{
    class Chip;
    class PCSCReaderCardAdapter;
    class PCSCReaderProvider;
    class SAMChip;

    /**
     * \brief The PC/SC reader unit class.
     */
    class LIBLOGICALACCESS_API PCSCReaderUnit : public ISO7816ReaderUnit
    {
    public:

        /**
         * \brief Constructor.
         */
        PCSCReaderUnit(const std::string& name);

        /**
         * \brief Destructor.
         */
        virtual ~PCSCReaderUnit();

        /**
         * \brief Get the reader unit name.
         * \return The reader unit name.
         */
        virtual std::string getName() const;

        /**
         * \brief Set the reader unit name.
         * \param name The reader unit name.
         */
        void setName(const std::string& name);

        /**
         * \brief Get the connected reader unit name.
         * \return The connected reader unit name.
         */
        virtual std::string getConnectedName();

        /**
         * \brief Get the PC/SC reader unit type.
         * \return The PC/SC reader unit type.
         */
        virtual PCSCReaderUnitType getPCSCType() const;

        /**
         * \brief Set the card type.
         * \param cardType The card type.
         */
        virtual void setCardType(std::string cardType) { d_card_type = cardType; };

        /**
         * \brief Wait for a card insertion.
         * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero, then the call never times out.
         * \return True if a card was inserted, false otherwise. If a card was inserted, the name of the reader on which the insertion was detected is accessible with getReader().
         * \warning If the card is already connected, then the method always fail.
         */
        virtual bool waitInsertion(unsigned int maxwait);

        /**
         * \brief Wait for a card removal.
         * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero, then the call never times out.
         * \return True if a card was removed, false otherwise. If a card was removed, the name of the reader on which the removal was detected is accessible with getReader().
         */
        virtual bool waitRemoval(unsigned int maxwait);

        /**
         * \brief Connect to the card.
         * \return True if the card was connected without error, false otherwise.
         *
         * If the card handle was already connected, connect() first call disconnect(). If you intend to do a reconnection, call reconnect() instead.
         */
        virtual bool connect();

        /**
         * \brief Connect to the card.
         * \param reader The reader name. If an empty or invalid reader name is specified, connect will fail.
         * \param share_mode The share mode.
         * \return True if the card was connected without error, false otherwise.
         *
         * If the card handle was already connected, connect() first call disconnect(). If you intend to do a reconnection, call reconnect() instead.
         */
        bool connect(PCSCShareMode share_mode);

        /**
         * \brief Reconnect to the card with the currently active share mode on the same reader.
         * \return True if the card was reconnected without error, false otherwise.
         */
        bool reconnect();

        /**
         * \brief Disconnect from the card.
         */
        virtual void disconnect();

        /**
         * \brief Disconnect from the card.
         * \param action Card action on disconnect.
         */
        virtual void disconnect(unsigned int action);

        /**
         * \brief Check if the handle is associated to a card (aka. "connected").
         * \return True if the handle is associated to a card, false otherwise.
         */
        virtual bool isConnected();

        /**
         * \brief Connect to the reader. Implicit connection on first command sent.
         * \return True if the connection successed.
         */
        virtual bool connectToReader();

        /**
         * \brief Disconnect from reader.
         */
        virtual void disconnectFromReader();

        /**
         * \brief Get the low-level SCARDHANDLE.
         * \return The SCARDHANDLE.
         */
        const SCARDHANDLE& getHandle() const;

        /**
         * \brief Get the active protocol for the connected card.
         * \return The active protocol.
         */
        inline unsigned long getActiveProtocol() const { return d_ap; };

        /**
         * \brief Get the currently active share mode.
         * \return The currently active share mode.
         *
         * If the card is not connected, requesting the share mode value is meaningless.
         */
        inline PCSCShareMode getShareMode() const { return d_share_mode; };

        /**
         * \brief Create the chip object from card type.
         * \param type The card type.
         * \return The chip.
         */
        virtual boost::shared_ptr<Chip> createChip(std::string type);

        /**
         * \brief Get the first and/or most accurate chip found.
         * \return The single chip.
         */
        virtual boost::shared_ptr<Chip> getSingleChip();

        /**
         * \brief Get chip available in the RFID rang.
         * \return The chip list.
         */
        virtual std::vector<boost::shared_ptr<Chip> > getChipList();

        /**
         * \brief Get the card serial number.
         * \return The card serial number.
         *
         * If the card handle is not connected to the card, the call fails.
         */
        virtual std::vector<unsigned char> getCardSerialNumber();

        /**
         * \brief Get a string hexadecimal representation of the reader serial number
         * \return The reader serial number or an empty string on error.
         */
        virtual std::string getReaderSerialNumber();

        /**
         * \brief Get the card ATR.
         * \param atr The array that will contains the ATR data.
         * \param atrLength The ATR array length.
         * \return The ATR length.
         */
        size_t getATR(void* atr, size_t atrLength);

        /**
         * \brief Get the default PC/SC reader/card adapter.
         * \return The default PC/SC reader/card adapter.
         */
        virtual boost::shared_ptr<PCSCReaderCardAdapter> getDefaultPCSCReaderCardAdapter();

        /**
         * \brief Get the PC/SC reader/card adapter for a card type.
         * \param type The card type.
         * \return The PC/SC reader/card adapter.
         */
        virtual boost::shared_ptr<ReaderCardAdapter> getReaderCardAdapter(std::string type);

        /**
         * \brief Serialize the current object to XML.
         * \param parentNode The parent node.
         */
        virtual void serialize(boost::property_tree::ptree& parentNode);

        /**
         * \brief UnSerialize a XML node to the current object.
         * \param node The XML node.
         */
        virtual void unSerialize(boost::property_tree::ptree& node);

        /**
         * \brief Get the reader unit configuration.
         * \return The reader unit configuration.
         */
        virtual boost::shared_ptr<ReaderUnitConfiguration> getConfiguration();

        /**
         * \brief Set the reader unit configuration.
         * \param config The reader unit configuration.
         */
        virtual void setConfiguration(boost::shared_ptr<ReaderUnitConfiguration> config);

        /**
         * \brief Change a key in reader memory.
         * \param keystorage The key storage information.
         * \param key The key value.
         */
        virtual void changeReaderKey(boost::shared_ptr<ReaderMemoryKeyStorage> keystorage, const std::vector<unsigned char>& key);

        /**
         * \brief Get the PC/SC reader unit configuration.
         * \return The PC/SC reader unit configuration.
         */
        boost::shared_ptr<PCSCReaderUnitConfiguration> getPCSCConfiguration() { return boost::dynamic_pointer_cast<PCSCReaderUnitConfiguration>(getConfiguration()); };

        boost::shared_ptr<PCSCReaderProvider> getPCSCReaderProvider() const;

        static boost::shared_ptr<PCSCReaderUnit> createPCSCReaderUnit(std::string& readerName);

        /**
         * \brief Make the reader unit as a proxy to another. Use when listening on all PC/SC reader unit.
         * \param readerUnit The referenced reader unit.
         * \param readerUnitConfig The referenced reader unit configuration.
         */
        void makeProxy(boost::shared_ptr<PCSCReaderUnit> readerUnit, boost::shared_ptr<PCSCReaderUnitConfiguration> readerUnitConfig);

        /**
         * \brief Get if the inserted card is from ISO 14443-4 A or B.
         * \param isTypeA Will be set to true if the card is from ISO 14443-4 A, false if not or unknown.
         * \param isTypeB Will be set to true if the card is from ISO 14443-4 B, false if not or unknown.
         */
        virtual void getT_CL_ISOType(bool& isTypeA, bool& isTypeB);

        /**
         * \brief Get The SAM Chip
         */
        boost::shared_ptr<SAMChip> getSAMChip();

        /**
         * \brief Set the SAM Chip
         */
        void setSAMChip(boost::shared_ptr<SAMChip> t);

        /**
         * \brief Get The SAM ReaderUnit
         */
        boost::shared_ptr<PCSCReaderUnit> getSAMReaderUnit();

        /**
         * \brief Set the SAM ReaderUnit
         */
        void setSAMReaderUnit(boost::shared_ptr<PCSCReaderUnit> t);

    protected:

        /**
         * \brief The SAM reader unit.
         */
        boost::shared_ptr<SAMChip> d_sam_chip;

        /**
         * \brief The reader unit name.
         */
        std::string d_name;

        /**
         * \brief The reader unit connected name.
         */
        std::string d_connectedName;

        /**
         * \brief Get the card type from a specified ATR.
         * \param atr The atr string (without terminal NULL character).
         * \param atrlen The length of the atr string.
         * \return The card type.
         */
        std::string getCardTypeFromATR(const unsigned char* atr, size_t atrlen);

        /**
         * \brief Get the generic card type from ATR.
         * \param atr The atr string (without terminal NULL character).
         * \param atrlen The length of the atr string.
         * \return The generic card type.
         */
        std::string getGenericCardTypeFromATR(const unsigned char* atr, size_t atrlen);

        /**
         * \brief Set the first and/or most accurate chip found.
         * \param chip The single chip.
         */
        void setSingleChip(boost::shared_ptr<Chip> chip);

        /**
         * \brief Get the card type from atr code
         * \param code The atr code
         * \return The card type
         */
        std::string atrXToCardType(int code) const;

        /**
         * \brief The handle.
         */
        SCARDHANDLE d_sch;

        /**
         * \brief The share mode.
         */
        PCSCShareMode d_share_mode;

        /**
         * \brief The activated protocol.
         */
        DWORD d_ap;

        /**
         * \brief The current card ATR.
         */
        unsigned char d_atr[64];

        /**
         * \brief The ATR length.
         */
        size_t d_atrLength;

        /**
         * \brief The proxy reader unit.
         */
        boost::shared_ptr<PCSCReaderUnit> d_proxyReaderUnit;

        /**
         * \brief The SAM ReaderUnit used SAM Authentication
         */
        boost::shared_ptr<PCSCReaderUnit> d_sam_readerunit;
    };
}

#endif